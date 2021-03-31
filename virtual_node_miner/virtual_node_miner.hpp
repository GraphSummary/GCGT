#ifndef VIRTUAL_NODE_MINER_HPP
#define VIRTUAL_NODE_MINER_HPP

#include <cstdint>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include "utils/timer.h"

#define MAX_HASH 1610612741
#define K 10
// #define CLUSTER_THRESHOLD 30
// #define VIRTUAL_THRESHOLD 20
int CLUSTER_THRESHOLD = 30;
int VIRTUAL_THRESHOLD = 20;

using adjlist = std::vector<int>;
using cluster = std::pair<int, int>;

struct hash_row {
    int u;
    std::vector<int> hashes;
};

class hash_f {
    int a;
    int b;
    int c;

public:
    hash_f() {
        a = rand();
        b = rand();
        c = rand();
    };

    int hash(int x) {
        return unsigned(a * (x + c) + b) % MAX_HASH;
    }
};

class prefix_tree {
    struct pt_node {
        int v;
        int depth;
        pt_node *parent;
        std::vector<int> u_list;
        std::map<int, pt_node> children;

        int save() const {
            return (u_list.size() - 1) * (depth - 1) - 1;
        }

        void generate_pattern(std::unordered_set<int> &pattern) {
            pt_node *cur = this;
            while (cur->depth > 0) {
                pattern.insert(cur->v);
                cur = cur->parent;
            }
        }
    };

    std::unordered_map<int, int> _v_cnt;
    pt_node _root;

public:

    std::vector<pt_node *> potential_nodes;
    int _raw_num_node=0;

    prefix_tree() {
        _root.v = -1;
        _root.depth = 0;
        _root.u_list.clear();
        _root.children.clear();
    }

    void count_v(int v) {
        if (_v_cnt.find(v) == _v_cnt.end()) {
            _v_cnt[v] = 1;
        } else {
            _v_cnt[v]++;
        }
    }

    void update_tree(pt_node &node, int u, adjlist &outlinks) {
        if (node.depth > 0) {
            node.u_list.emplace_back(u);
        }

        if (node.depth < outlinks.size()) {
            int child = outlinks[node.depth]; // 需要放置的点
            if (node.children.find(child) == node.children.end()) {
                pt_node new_node;
                new_node.depth = node.depth + 1;
                new_node.v = child;
                new_node.parent = &node;
                node.children[child] = new_node;
            }
            update_tree(node.children[child], u, outlinks);
        }
    }

    void add_node(int u, adjlist &adj) {
        adjlist outlinks;
        for (auto v : adj) {
            // if (_v_cnt[v] > 1) outlinks.emplace_back(v);
            // 虚拟点不加入,注意： outlinks <= adj
            if (_v_cnt[v] > 1 && v < _raw_num_node) outlinks.emplace_back(v);
        }
        std::sort(outlinks.begin(), outlinks.end(),
                  [this](const int &a, const int &b) -> bool {
                      return _v_cnt[a] > _v_cnt[b];
                  });

        update_tree(_root, u, outlinks);
    }

    void find_potential_nodes(pt_node &node) {
        if (node.depth > 1 && node.children.size() != 1 && node.u_list.size() > 1) {
            potential_nodes.emplace_back(&node);
        }

        for (auto &child : node.children) {
            find_potential_nodes(child.second);
        }
    }

    void generate_vn() {
        find_potential_nodes(_root);
        std::sort(potential_nodes.begin(), potential_nodes.end(),
                  [](const pt_node *a, const pt_node *b) -> bool {
                      return a->save() > b->save();
                  });
    }
};

class virtual_node_miner {

    struct virtual_node {
        int u_id;
        std::vector<int> v_list;
    };

    std::vector<adjlist> _adjlists;
    size_t _num_node;
    size_t _raw_num_node;
    size_t _num_edge;
    size_t _raw_num_edge;

    std::vector<hash_f> _hashes;
    std::vector<hash_row> _hash_mat;
    std::vector<cluster> _clusters;
    std::vector<std::vector<virtual_node>> _cluster_virtual_nodes;
    std::vector<int> _x; // _x[v]: v in V, the number of real nodes that can be reached from v using virtual edges
    std::vector<int> _y; // _y[v]: v's real outadjsum.
    int merge_virtual_node_num=0;
    std::vector<adjlist> _adjlists_in; // 记录入邻居
    std::unordered_set<int> _active_node_r; // 受影响的真实点
    std::unordered_set<int> _active_node_v; // 受影响的虚拟点


public:
    virtual_node_miner(int CLUSTER_THRESHOLD_, int VIRTUAL_THRESHOLD_){
        CLUSTER_THRESHOLD = CLUSTER_THRESHOLD_;
        VIRTUAL_THRESHOLD = VIRTUAL_THRESHOLD_;
    }

    void reset_hash_function() {
        _hashes.clear();
        for (int i = 0; i < K; i++) {
            _hashes.emplace_back(hash_f());
        }
    }

    bool write_graph(const std::string &file_path) {
        FILE *fp = fopen(file_path.c_str(), "w");

        if (fp == nullptr) {
            std::cout << "graph file cannot create!" << file_path << std::endl;
            return false;
        }

        for (int i = 0; i < _num_node; i++) {
            for (int j = 0; j < _adjlists[i].size(); j++) {
                fprintf(fp, "%d %d\n", i, _adjlists[i][j]);
            }
        }
        fclose(fp);

        return true;
    }

    bool load_graph(const std::string &file_path) {
        auto load_start = time(nullptr);

        FILE *f = fopen(file_path.c_str(), "r");

        if (f == 0) {
            std::cout << "file cannot open! " << file_path << std::endl;
            return false;
        }
        std::cout << "load: " << file_path << std::endl;
        std::vector<std::pair<int, int>> edges;

        int u = 0;
        int v = 0;
        _num_node = 0;
        while (fscanf(f, "%d %d", &u, &v) > 0) {
            assert(u >= 0);
            assert(v >= 0);
            _num_node = std::max(_num_node, size_t(u + 1));
            _num_node = std::max(_num_node, size_t(v + 1));
            edges.emplace_back(std::pair<int, int>(u, v));
        }
        fclose(f);

        std::sort(edges.begin(), edges.end());

        _raw_num_node = _num_node;

        _num_edge = edges.size();
        _raw_num_edge = _num_edge;

        _adjlists.clear();
        _adjlists.resize(_num_node); // _num_node max_id+1
        for (auto edge : edges) {
            _adjlists[edge.first].emplace_back(edge.second);
        }

        auto load_end = time(nullptr);

        return true;
    }

    void calc_min_hash(int u) {
        _hash_mat[u].u = u;
        _hash_mat[u].hashes.clear();

        for (int i = 0; i < K; i++) {
            int min_hash = MAX_HASH;
            for (auto v : _adjlists[u]) {
                min_hash = std::min(min_hash, _hashes[i].hash(v));
            }
            _hash_mat[u].hashes.emplace_back(min_hash);
        }
    }

    void generate_clusters(int col, int start, int end) {
        int idx = start;

        while (idx < end) { // line
            int idx2 = idx;
            while (idx2 < end && _hash_mat[idx].hashes[col] == _hash_mat[idx2].hashes[col]) {
                idx2++;
            }

            if (col == K - 1 || idx2 - idx <= CLUSTER_THRESHOLD) {

                int idx3 = idx2;
                while (idx2 < end) {
                    while (idx3 < end && _hash_mat[idx2].hashes[col] == _hash_mat[idx3].hashes[col]) {
                        idx3++;
                    }
                    if (idx3 - idx <= CLUSTER_THRESHOLD) {
                        idx2 = idx3;
                    } else {
                        break;
                    }
                }

                _clusters.emplace_back(cluster(idx, idx2));
            } else {
                generate_clusters(col + 1, idx, idx2);
            }
            idx = idx2;
        }
    }

    void modify_adjlist(int u, int virual_node_id, std::unordered_set<int> &pattern) {
        std::vector<int> new_adjlist;
        for (auto v : _adjlists[u]) {
            if (pattern.count(v) == 0) new_adjlist.emplace_back(v);
        }
        new_adjlist.emplace_back(virual_node_id);
        _adjlists[u] = new_adjlist;
    }

    void handle_cluster(int cluster_id, int start, int end) {
        prefix_tree pt;
        pt._raw_num_node = _raw_num_node;

        for (int i = start; i < end; i++) { // line
            for (auto v : _adjlists[_hash_mat[i].u]) pt.count_v(v);
        }

        for (int i = start; i < end; i++) {
            pt.add_node(_hash_mat[i].u, _adjlists[_hash_mat[i].u]);
        }

        pt.generate_vn();

        std::unordered_set<int> visited_nodes;
        for (auto potential_node : pt.potential_nodes) {
            int unvisited_node_cnt = 0;
            for (auto u : potential_node->u_list) {
                if (visited_nodes.count(u) == 0) unvisited_node_cnt++;
            }

            if ((unvisited_node_cnt - 1) * (potential_node->depth - 1) < VIRTUAL_THRESHOLD) continue;

            std::unordered_set<int> pattern;
            potential_node->generate_pattern(pattern);

            // create new virtual node
            virtual_node vn;
#pragma omp critical
            {
                vn.u_id = _num_node++;
            };

            for (auto v : pattern) vn.v_list.emplace_back(v);
            std::sort(vn.v_list.begin(), vn.v_list.end());

            for (auto u : potential_node->u_list) {
                if (visited_nodes.count(u) == 0) {
                    visited_nodes.insert(u);
                    modify_adjlist(u, vn.u_id, pattern);
                }
            }

            _cluster_virtual_nodes[cluster_id].emplace_back(vn);
        }
    }

    void one_pass() {
        reset_hash_function();

        // generate min-hash matrix
        _hash_mat.clear();
        // _hash_mat.resize(_num_node);
        _hash_mat.resize(_raw_num_node);

#pragma omp parallel for
        // for (int i = 0; i < _num_node; i++) {
        for (int i = 0; i < _raw_num_node; i++) {
            calc_min_hash(i);
        }
#pragma omp barrier

        // _hash_mat：Sort the rows of the matrix lexicographically
        std::sort(_hash_mat.begin(), _hash_mat.end(),
                  [](const hash_row &a, const hash_row &b) -> bool {
                      for (int i = 0; i < a.hashes.size(); i++) {
                          if (a.hashes[i] != b.hashes[i]) return a.hashes[i] < b.hashes[i];
                      }
                      return false;
                  });

        _clusters.clear();
        int end = 0;
        // while (end < _num_node && _hash_mat[end].hashes[0] < MAX_HASH) end++; // 过滤掉不存在/无出度的点
        while (end < _raw_num_node && _hash_mat[end].hashes[0] < MAX_HASH) end++; // 过滤掉不存在/无出度的点
        generate_clusters(0, 0, end);

        _cluster_virtual_nodes.clear();
        _cluster_virtual_nodes.resize(_clusters.size());

        // generate potential virtual node list
#pragma omp parallel for
        for (int i = 0; i < _clusters.size(); i++) {
            handle_cluster(i, _clusters[i].first, _clusters[i].second);
        }
#pragma omp barrier

        for (auto &vns : _cluster_virtual_nodes) {
            for (auto &vn : vns) {
                if (vn.u_id >= _adjlists.size()) _adjlists.resize(vn.u_id + 1);
                _adjlists[vn.u_id] = vn.v_list;
            }
        }
    }

    void refresh_num_edge() {
        size_t new_num_edge = 0;
        for (auto &adjlist : _adjlists) new_num_edge += adjlist.size();
        _num_edge = new_num_edge;
    }

    // edge: u->v, insert v into u
    void merge_samevirtual_node(int u, int v) {
        std::vector<int> &u_adjlist = _adjlists[u];
        std::vector<int>::iterator it = find(u_adjlist.begin(), u_adjlist.end(), v);
        // u_adjlist.erase(it);
        std::swap(*it, u_adjlist.back());
        u_adjlist.pop_back();
        for (auto id : _adjlists[v]) {
            if(id >= _raw_num_node && _adjlists[id].size() > 0){
                printf("这里居然有虚拟点！！！merge_samevirtual_node\n");
                abort();
            }
            u_adjlist.emplace_back(id);
        }
        _adjlists[v].clear();
    }

    void find_samevirtual_node(int u){
        for (auto v : _adjlists[u]) {
            if(v >= _raw_num_node && _adjlists[v].size() > 0){
                // printf("%d -> %d\n", u, v);
                find_samevirtual_node(v);
                merge_virtual_node_num++;
                merge_samevirtual_node(u, v); // u->v, insert v into u
            }
        }
    }

    void deal_samevirtual_node(){
        for (int u = _raw_num_node; u < _num_node; u++) {
            find_samevirtual_node(u);
        }
    }

    void compress(int pass_num = 3) {
        printf("CLUSTER_THRESHOLD=%d, VIRTUAL_THRESHOLD=%d\n", CLUSTER_THRESHOLD, VIRTUAL_THRESHOLD);
        // pass_num = 1; // 图的压缩次数，如果压缩多次会产生压缩点指向压缩点的情况。多次压缩的化，需要最和合并虚拟点
        for (int pass = 0; pass < pass_num; pass++) {
            double start_com = clock();
            timer_next("compress time_" + std::to_string(pass));
            one_pass();
            refresh_num_edge();
            std::cout << "compress time=" << (clock() - start_com)/ CLOCKS_PER_SEC << std::endl;

            printf("pass %d: node: %lu/%lu (%.4lf)\tedge: %lu/%lu (%.4lf)\n", pass,
                   _num_node, _raw_num_node, float(_num_node) / _raw_num_node,
                   _num_edge, _raw_num_edge, float(_num_edge) / _raw_num_edge
            );
            // int t = merge_virtual_node_num;
            // deal_samevirtual_node();
            // printf("--pass %d: node: %lu/%lu (%.4lf)\tedge: %lu/%lu (%.4lf)\n", pass,
            //        _num_node, _raw_num_node, float(_num_node) / _raw_num_node,
            //        _num_edge, _raw_num_edge, float(_num_edge) / _raw_num_edge
            // );
            // printf("%d\n", merge_virtual_node_num-t);
        }
        std::string file_path = "./out/result.txt";
        FILE *fp = fopen(file_path.c_str(), "a+");
        fprintf(fp, "nor_node:%d\n", int(_raw_num_node));
        fprintf(fp, "nor_edge:%d\n", int(_raw_num_edge));
        fprintf(fp, "com_node:%d\n", int(_num_node));
        fprintf(fp, "com_edge:%d\n", int(_num_edge));
        fprintf(fp, "edge compression rate:%f\n", float(int(_num_node)) / int(_raw_num_node));
        fprintf(fp, "node compression rate:%f\n", float(int(_num_edge)) / int(_raw_num_edge));
        fclose(fp);
    }

    void remove_adj(std::vector<int> &u_adjlist, int v){
        std::vector<int>::iterator it = find(u_adjlist.begin(), u_adjlist.end(), v);
        if(it == u_adjlist.end()){
            std::cout << "error: " << v << std::endl;
            exit(0);
        }
        std::swap(*it, u_adjlist.back());
        u_adjlist.pop_back();
    }

    bool load_update(const std::string &file_path) {
        FILE *f = fopen(file_path.c_str(), "r");
        if (f == 0) {
            std::cout << "file cannot open! " << file_path << std::endl;
            return false;
        }
        int u = 0;
        int v = 0;
        char type;
        int line_cnt = 0;
        // 数据边加载边处理：
        while (fscanf(f, "%c %d %d\n", &type, &u, &v) > 0) {
            line_cnt++;
            // std::cout << "load_update: " <<  "f=" << type << ",u=" << u << ",v=" << v << std::endl;
            assert(u >= 0);
            assert(v >= 0);
            if(type == 'd'){
                std::vector<int>::iterator iter=find(_adjlists[u].begin(), _adjlists[u].end(), v);
                if(iter == _adjlists[u].end()){ 
                    // v在虚拟点中
                    // std::cout << "v在虚拟点中 " << std::endl;
                    std::vector<int>::iterator t;
                    for(auto i : _adjlists[u]){
                        if(i < _raw_num_node){ // 必须在虚拟邻居中查找，否则直接跳过
                            continue;
                        }
                        t = find(_adjlists[i].begin(), _adjlists[i].end(), v);
                        if(t != _adjlists[i].end()){
                            // delete： u->i
                            // std::cout << "v在虚拟点中 i=" << i << ",u=" << u << std::endl;
                            remove_adj(_adjlists[u], i);
                            remove_adj(_adjlists_in[i], u);
                            // 将其余边添加到 u_adj
                            for(auto j : _adjlists[i]){
                                if(j == v){
                                    continue;
                                }
                                // std::cout << "v在虚拟点中 " << j << std::endl;
                                _adjlists[u].emplace_back(j);
                                _adjlists_in[j].emplace_back(u);
                            }  
                            _active_node_r.insert(u); 
                            // _active_node_v.add(v); 
                            _active_node_v.insert(i); // i可能会被清理掉，因为save降低了
                            break;
                        }
                    }
                }
                else{
                    // u和v直接相连
                    // std::cout << "u和v直接相连 " << std::endl;
                    std::swap(*iter, _adjlists[u].back());
                    _adjlists[u].pop_back();
                    remove_adj(_adjlists_in[v], u);
                }
            }
            else if (type == 'a'){
                _adjlists[u].emplace_back(v);
                _adjlists_in[v].emplace_back(u);
                _active_node_r.insert(u); 
                // 找和u相似的虚拟点，尝试u和其合并
                // 利用MinHash来找

            }
            else{
                std::cout << "load_update error! " <<  "f=" << type << ",u=" << u << ",v=" << v << std::endl;
                exit(0);
            }
        }
        fclose(f);
        std::cout << "line_cnt=" << line_cnt << std::endl;
        // std::sort(edges.begin(), edges.end());

        return true;
    }

    void increment_compress(const std::string &file_path){
        std::cout << "_raw_num_node=" << _raw_num_node << std::endl;
        // 得到入邻居
        _adjlists_in.clear();
        _adjlists_in.resize(_num_node);
        for(int i = 0; i < _num_node; i++){
            for (int j : _adjlists[i]) {
                _adjlists_in[j].emplace_back(i);
            }
        }
        // 加载数据更新图结构
        load_update(file_path);
        // 清理无用的虚拟节点：_active_node_v

        // 将受影响的点重新聚类: _active_node_r
        


    }

    int fill_x(int vid){
        if(_x[vid] != -1){
            return _x[vid];
        }
        int cnt = 0;
        for(auto id : _adjlists[vid]){
            cnt += fill_x(id);
        }
        return _x[vid] = cnt;
    }

    void computeX(){
        _x.clear();
        _x.resize(_num_node); // _num_node max_id+1
        for (int i = 0; i < _raw_num_node; i++) {  
            _x[i] = 1;  // real node
        }
        for (int i = _raw_num_node; i < _num_node; i++) {
            _x[i] = -1; // virtual node
        }
        for (int i = _raw_num_node; i < _num_node; i++) {
            if(_x[i] == -1){
                fill_x(i);
            }
        }
        // 检测是否存在虚拟点连接虚拟点：
        int cnt = 0;
        for (int i = _raw_num_node; i < _num_node; i++) {
            for(auto id : _adjlists[i]){
                if(_x[id] > 1){
                    printf("%d %d, 虚拟点的出邻居居然也是虚拟点。。。\n", i, id);
                    cnt++;
                }
            }
        }
        printf("虚拟点的出邻居也是虚拟点: cnt=%d\n", cnt);
    }
    
    void computeY(){
        _y.clear();
        _y.resize(_num_node); // _num_node max_id+1
        for (int i = 0; i < _num_node; i++) {
            _y[i] = 0;
            for(int j : _adjlists[i]){
                _y[i] += _x[j];
            }
        }
    }

    bool write_vertex(const std::string &file_path) {
        FILE *fp = fopen(file_path.c_str(), "w");

        if (fp == nullptr) {
            std::cout << "graph file cannot create!" << file_path << std::endl;
            return false;
        }

        // node file first line: number of nodes and number of virtual nodes.
        // update num
        int _raw_num_node_new = 0;
        int _num_node_new = 0;
        for(int i = 0; i < _raw_num_node; i++){
            if(_adjlists[i].size() > 0) _raw_num_node_new++;
        }
        _num_node_new = _raw_num_node_new;
        for(int i = _raw_num_node; i < _num_node; i++){
            if(_adjlists[i].size() > 0) _num_node_new++;
        }
        fprintf(fp, "%d %d\n", int(_num_node_new), int(_raw_num_node_new));
        for (int i = 0; i < _num_node; i++) {
            if(_adjlists[i].size() > 0){
                fprintf(fp, "%d %d %d\n", i, _x[i], _y[i]);
            }
        }
        fclose(fp);

        return true;
    }

    bool write_de_graph(const std::string &file_path) {
        FILE *fp = fopen(file_path.c_str(), "w");

        if (fp == nullptr) {
            std::cout << "graph file cannot create!" << file_path << std::endl;
            return false;
        }

        for (int i = 0; i < _raw_num_node; i++) {
            // Virtual points without in-degree, need to be cleared
            if(i >= _raw_num_node && _adjlists_in.size() > 0 && _adjlists_in[i].size() < 1){
                continue;
            }
            for (int j = 0; j < _adjlists[i].size(); j++) {
                fprintf(fp, "%d %d\n", i, _adjlists[i][j]);
            }
        }
        fclose(fp);

        return true;
    }

    // 解压：将图更改为为压缩时的结构
    void decompress(){
        // 需要保证，_adjlists_in已经初始化赋值了
        std::cout << "_raw_num_node=" << _raw_num_node << std::endl;
        for(int i = _raw_num_node; i < _num_node; i++){ // i is a virtual node
            if(_adjlists[i].size() > 0){
                for(auto j : _adjlists_in[i]){ 
                    std::vector<int> &u_adjlist = _adjlists[j];
                    std::vector<int>::iterator it = find(u_adjlist.begin(), u_adjlist.end(), i);
                    std::swap(*it, u_adjlist.back());
                    u_adjlist.pop_back();
                    // u_adjlist.insert(u_adjlist.end(), _adjlists[i].begin(), _adjlists[i].end());
                    for(auto q : _adjlists[i]){
                        u_adjlist.push_back(q);
                    }
                }

            }
        }
        std::cout << std::endl;
    }

};


#endif /* VIRTUAL_NODE_MINER_HPP */