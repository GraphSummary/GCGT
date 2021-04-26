/*
    查找图中的特殊的结构，即只有一个入度结构
*/
#ifndef TOOLS_FIND_PATTERN_H_
#define TOOLS_FIND_PATTERN_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include "../utils/timer.h"
#include <math.h>
#include <queue>
#include <cassert>
#include <algorithm>
#include <memory.h>
#include "../graph/node.h"
#include "../graph/edge.h"
#include "../app/IterateKernel.cc"
#include "../worker/flags.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::ifstream;
using std::ofstream;

#define MAX_DIST 0xffff

// Functor
template<class vertex_t>
class isEqualALL {
public:
    explicit isEqualALL(vertex_t node) : node(node) {}
    bool operator() (const std::pair<int, int>& element) const {
        return element.first == node;
    }
private:
    const int node;
};

template<class vertex_t, class value_t>
class FindPattern{
public:
    void load(const std::string &efile){
        std::ifstream inFile(efile);
        if(!inFile){
           std::cout << "open file failed. " << efile <<std::endl;
            exit(0);
        }
        std::cout << "finish read file... " << efile <<std::endl;
        vertex_t u, v;
        value_t w;
        std::vector<Edge<vertex_t, value_t>> edges;
        vertex_t max_id;
        while (inFile >> u >> v >> w) {
            assert(u >= 0);
            assert(v >= 0);
            assert(w >= 0);
            max_id = std::max(max_id, u);
            max_id = std::max(max_id, v);
            edges.emplace_back(Edge<vertex_t, value_t>(u, v, w));
        }

        edges_num = edges.size();
        nodes = new Node<vertex_t, value_t>[max_id+1]; 

        for (auto edge : edges) {
            u = edge.source;
            v = edge.destination;
            w = edge.weight;
            if(vertex_map.find(u) == vertex_map.end()){
                vertex_map[u] = nodes_num;
                vertex_reverse_map[nodes_num] = u;
                nodes[nodes_num].id = nodes_num;
                u = nodes_num++;
            }
            else{
                u = vertex_map[u];
            }
            if(vertex_map.find(v) == vertex_map.end()){
                vertex_map[v] = nodes_num;
                vertex_reverse_map[nodes_num] = v;
                nodes[nodes_num].id = nodes_num;
                v = nodes_num++;
            }
            else{
                v = vertex_map[v];
            }
            nodes[u].out_adj.emplace_back(std::pair<vertex_t, value_t>(v, w));
            nodes[v].in_adj.emplace_back(std::pair<vertex_t, value_t>(u, w));
            nodes[v].tag = v;
            nodes[u].tag = u;
            // std::cout << v << ": " << nodes[v].tag << ", " << u << ": " << nodes[u].tag << std::endl;
        }
        std::cout << "load finied, nodes_num=" << nodes_num << " edges_num=" << edges.size() << std::endl;
    }

    void load_update(const std::string &efile){
        std::ifstream inFile(efile);
        if(!inFile){
           std::cout << "open file failed. " << efile <<std::endl;
            exit(0);
        }
        std::cout << "finish read file... " << efile <<std::endl;
        vertex_t u, v;
        value_t w;
        char t;
        std::vector<Edge<vertex_t, value_t>> edges;
        vertex_t update_num = 0;
        while (inFile >> t >> u >> v >> w) {
            assert(t == 'a' || t == 'd');
            assert(u >= 0);
            assert(v >= 0);
            assert(w >= 0);
            v = vertex_map[v];
            u = vertex_map[u];
            update_edges.emplace_back(UpdateEdge<vertex_t, value_t>(t, u, v, w));
            update_num++;
        }
        std::cout << "update_num=" << update_num << std::endl;
    }

    void Dijkstra(ExpandData<vertex_t, value_t> &expand_data){
        // std::cout << "Dijkstra ..." << std::endl;
        // init
        // expand_data.inner_ids.clear();
        // expand_data.bound_ids.clear();
        expand_data.inner_edges.clear();
        expand_data.bound_edges.clear();
        std::vector<vertex_t> &node_set = expand_data.ids; 
        const vertex_t source = expand_data.id;
        unordered_map<vertex_t, value_t> dist_map;
        unordered_map<vertex_t, bool> visited;
        for(auto u : node_set){
            dist_map[u] = MAX_DIST;
            visited[u] = 0;
        }
        dist_map[source] = 0;
        vertex_t max_cnt = node_set.size();
        // dijkstra
        for(vertex_t i = 0; i < max_cnt; i++){
            value_t min_dist = MAX_DIST;
            vertex_t min_j = -1;
            for(auto j : node_set){
                if(visited[j] == 0 && min_dist > dist_map[j]){
                    min_dist = dist_map[j];
                    min_j = j;
                }
            }
            if(min_j == -1){
                break;
            }
            visited[min_j] = 1;
            for(auto edge : nodes[min_j].out_adj){
                vertex_t u = edge.first; // 要保证u属于这个集和内,即u是内部点，不是边界点
                if(Fc[u] == source && dist_map[u] > dist_map[min_j] + edge.second){
                    dist_map[u] = dist_map[min_j] + edge.second;
                }
            }
        }
        // 为结构加入直接指向内部点的最短距离边： source -> 内部id
        for(auto kv : dist_map){
            if(kv.first != source){ // source -> source 不必放入
                // expand_data.edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second));
                // s -> (inner_node + bound_node)
                expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second)); // 可以合到下面的循环里面
            }
            // std::cout << "--" << kv.first << ": " << kv.second << std::endl;
        }
        // 为结构加入直接指向外部点的最短距离边： source -> 外部id
        unordered_map<vertex_t, value_t> bound_map;  // out_node: dist(out_node)
        for(auto kv : dist_map){
            bool is_inner = true;
            for(auto edge : nodes[kv.first].out_adj){
                if(Fc[edge.first] != source){  // 可能会出现发给同一个点
                    // s -> 外部点
                    // expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(edge.first, kv.second + edge.second));
                    if(bound_map.find(edge.first) == bound_map.end()){
                        bound_map[edge.first] = kv.second + edge.second;
                    }
                    else{
                        bound_map[edge.first] = std::min(bound_map[edge.first], kv.second + edge.second);
                    }
                    is_inner = false;
                }
            }
            // if(is_inner){
                // expand_data.inner_ids.emplace_back(kv.first);
            // }
            // else{
            //     expand_data.bound_ids.emplace_back(kv.first);
            // }
        }
        for(auto kv : bound_map){
            expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second));
        }
        // node_set没必要存储，故清空
        // node_set.clear();
        // expand_data.print_edge();
    }

    void init_node(const std::vector<vertex_t> &node_set, const vertex_t& source){
        value_t delta;
        value_t value;
        vector<std::pair<vertex_t, value_t>> data;
        vertex_t old_source = FLAGS_shortestpath_source;
        for(auto i : node_set){
            app_->init_c(i, nodes[i].recvDelta, data, source);
            this->nodes[i].oldDelta = app_->default_v();
            app_->init_v(i, nodes[i].value, data);
        }
    }

    /**
     * 计算超点内部需要的值，以及重新为边界点建立新边
    */
    void init_ExpandData(ExpandData<vertex_t, value_t> &expand_data){
        std::vector<vertex_t> &node_set = expand_data.ids; 
        const vertex_t source = expand_data.id;
        // init supernode
        init_node(node_set, source);
        // 测试：
        // for(auto u : node_set){
        //     std::cout << nodes[u].id << " " << nodes[u].value << " " << nodes[u].recvDelta << " " << nodes[u].oldDelta << std::endl;
        // }
       
        /* iterative calculation */
        bool is_convergence = false;
        int step = 0;
        while(true){
            step++;
            is_convergence = true;
            // send
            for(auto v : node_set){
                Node<vertex_t, value_t>& node = nodes[v];
                if(node.oldDelta != app_->default_v()){
                    for(auto &edge : node.out_adj){ // i -> adj
                        if(Fc[edge.first] != source){ // 只发给内部点
                            continue;
                        }
                        value_t& recvDelta = nodes[edge.first].recvDelta; // adj's recvDelta
                        value_t sendDelta; // i's 
                        app_->g_func(node.oldDelta, edge.second, sendDelta);
                        app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                    }
                    node.oldDelta = app_->default_v(); // delta发完需要清空 
                }
            }
            // receive
            for(auto v : node_set){
                Node<vertex_t, value_t>& node = nodes[v];
                value_t old_value = node.value;
                app_->accumulate(node.value, node.recvDelta); // delat -> value
                if(old_value != node.value){
                    is_convergence = false;
                    app_->accumulate(node.oldDelta, node.recvDelta); // updata delta
                }
                node.recvDelta = app_->default_v();
            }
            if(is_convergence){
                break;
            }
        }
        // std::cout << "step=" << step << std::endl;

        /**
         * create index in supernode 
         **/
        expand_data.inner_edges.clear();
        expand_data.bound_edges.clear();
        // 为结构加入直接指向内部点的最短距离边： source -> 内部id
        // for(auto u : node_set){
        //     Node<vertex_t, value_t>& node = nodes[u];
        //     expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(u, node.value));  // 可以合到下面的循环里面去
        //     // std::cout << "id = " << u << ", v = " << node.value << std::endl; 
        // }
        // 为结构加入直接指向外部点的最短距离边： source -> 外部id
        unordered_map<vertex_t, value_t> bound_map;  // out_node: dist(out_node)
        value_t data;
        for(auto u : node_set){
            Node<vertex_t, value_t>& node = nodes[u];
            expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(u, node.value)); // 为结构加入直接指向内部点的最短距离边： source -> 内部id
            for(auto &edge : node.out_adj){
                if(Fc[edge.first] != source){  // 可能会出现发给同一个点
                    app_->g_func(node.value, edge.second, data);
                    if(bound_map.find(edge.first) == bound_map.end()){
                        // bound_map[edge.first] = node.value + edge.second;
                        bound_map[edge.first] = data;
                    }
                    else{
                        // bound_map[edge.first] = std::min(bound_map[edge.first], node.value + edge.second);
                        app_->accumulate(bound_map[edge.first], data);
                    }
                }
            }
        }
        for(auto kv : bound_map){
            expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second));
        } 
    }

    /**
     * 通过源s传播，找到s可达的点
     */
    bool first_phase(const vertex_t s){
        // std::cout << "start first phase..." << MAX_HOP <<  std::endl;
        std::queue<vertex_t> old_q;
        std::queue<vertex_t> new_q;
        unordered_set<vertex_t> visited_nodes;

        int step = 0;
        nodes[s].tag = s;
        old_q.push(s);
        visited_nodes.insert(s);

        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << node.id << ":" << node.tag << ", ";
        // }
        // std::cout << std::endl;

        while(!old_q.empty() && step < MAX_HOP && visited_nodes.size() <= MAX_NODE_NUM){
            step++;
            while(!old_q.empty()){
                vertex_t q = old_q.front();
                // std::cout << nodes[q].id << ",";
                old_q.pop();
                for(auto edge: nodes[q].out_adj){
                    vertex_t u = edge.first;
                    if(Fc[u] != Fc_default_value) continue;  // 已经位于其他结构中了
                    nodes[u].tag = s;
                    if (visited_nodes.find(u) == visited_nodes.end()){
                        new_q.push(u);
                        visited_nodes.insert(u);
                    }
                } 
            }
            old_q.swap(new_q);
            std::queue<vertex_t> empty;
            new_q.swap(empty);
            // std::cout << "step=" << step << std::endl;
        }
        if(visited_nodes.size() < MIN_NODE_NUM){
            return false;
        }

        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << vertex_reverse_map[node.id] << ":" << vertex_reverse_map[node.tag] << ", ";
        // }
        // std::cout << std::endl;

        return true;
    }

    /**
     * 提取结构
    */
    void second_phase(const vertex_t source){
        // std::cout << "start second phase..." << MAX_HOP <<  std::endl;
        std::queue<vertex_t> old_q;
        std::queue<vertex_t> new_q;
        // bool *visited_nodes_1 = new bool[nodes_num]();  // 加()初始化为0； 考虑换掉，这样申请内存太浪费了
        // bool *in_queue_nodes_1 = new bool[nodes_num]();
        unordered_map<vertex_t, bool> visited_nodes;
        unordered_map<vertex_t, bool> in_queue_nodes;

        // memset(visited_nodes_1, false, sizeof(visited_nodes_1));
        // memset(in_queue_nodes_1, false, sizeof(in_queue_nodes_1));

        for(auto edge : nodes[source].out_adj){
            vertex_t u = edge.first;
            old_q.push(u);
            in_queue_nodes[u] = 1;
            visited_nodes[u] = 1;
            // in_queue_nodes_1[u] = 1;
            // visited_nodes_1[u] = 1;
        }

        // std::cout << "old_id : new_id" << std::endl;
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << vertex_reverse_map[node.id] << ":" << vertex_reverse_map[node.tag] << ", ";
        // }
        // std::cout << std::endl;

        int step = 0;
        while(!old_q.empty()){
            step++;
            while(!old_q.empty()){
                vertex_t q = old_q.front();
                // std::cout << nodes[q].id << ",";
                old_q.pop();
                in_queue_nodes[q] = 0;
                // pull 入邻居的tag确定自己的tag
                value_t tag = source;
                for(auto edge : nodes[q].in_adj){
                    vertex_t u = edge.first;
                    tag = nodes[u].tag;
                    if(tag != source){
                        break;
                    }
                }
                if(tag == source){
                    for(auto edge: nodes[q].out_adj){
                        vertex_t u = edge.first;
                        // nodes[u].tag = source; // 在第一阶段已经做过了，这样才能起到限制跳数
                        if (nodes[u].tag == source && in_queue_nodes[u] == 0 && visited_nodes[u] == 0){
                            new_q.push(u);
                            in_queue_nodes[u] = 1;
                            visited_nodes[u] = 1;
                        }
                    } 
                }
                else{
                    nodes[q].tag = q; // reset
                    for(auto edge : nodes[q].out_adj){
                        vertex_t u = edge.first;
                        Node<vertex_t, value_t> &node = nodes[u];
                        if(node.tag == source){
                            node.tag = node.id; // reset
                            if (in_queue_nodes[u] == 0){ // 这种情况，只需要不再队列中都需要加入队列
                                new_q.push(u);
                                in_queue_nodes[u] = 1;
                                visited_nodes[u] = 1;
                            }
                        }
                    }
                }
            }
            // std::cout << "step=" << step << std::endl;
            old_q.swap(new_q);
            std::queue<vertex_t> empty;
            new_q.swap(empty);

            // for(vertex_t i = 0; i < nodes_num; i++){
            //     Node<vertex_t, value_t> &node = nodes[i];
            //     std::cout << vertex_reverse_map[node.id] << ":" << vertex_reverse_map[node.tag] << ", ";
            // }
            // std::cout << std::endl;
        }
        
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << getOriginId(node.id) << ":" << getOriginId(node.tag) << ", ";
        // }
        // std::cout << std::endl;
    }

    /**
     * 计算结构内最短路径
     */
    void third_phase(const vertex_t source){
        // std::cout << "start third phase..." <<  std::endl;
        std::queue<vertex_t> queue;
        std::vector<vertex_t> visited_nodes;
        queue.push(source);
        visited_nodes.emplace_back(source);

        while(!queue.empty()){
            vertex_t q = queue.front();
            // std::cout << nodes[q].id << ",";
            queue.pop();
            for(auto edge: nodes[q].out_adj){
                vertex_t u = edge.first;
                if(nodes[u].tag == source && std::find(visited_nodes.begin(), visited_nodes.end(), u) == visited_nodes.end()){
                    queue.push(u);
                    visited_nodes.emplace_back(u);
                    // Fc[u] = source;
                }
            } 
        }
        // std::cout << std::endl;
        if(visited_nodes.size() < MIN_NODE_NUM){
            // std::cout << "visited_nodes.size() < MIN_NODE_NUM  size=" << visited_nodes.size() << std::endl;
            return;
        }
        for(auto u : visited_nodes){
            Fc[u] = source;
        }  

        // 查看Fc
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << vertex_reverse_map[node.id] << ":" <<  vertex_reverse_map[Fc[node.tag]] << ", ";
        // }
        // std::cout << std::endl;

        // 查看tag
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     Node<vertex_t, value_t> &node = nodes[i];
        //     std::cout << vertex_reverse_map[node.id] << ":" << vertex_reverse_map[node.tag] << ", ";
        // }
        // std::cout << std::endl;

        // build a supernode
        vertex_t supernoed_id = supernodes_num;
        Fc_map[source] = supernoed_id;
        expand_data[supernoed_id].id = source;
        expand_data[supernoed_id].ids.insert(expand_data[supernoed_id].ids.begin(), visited_nodes.begin(), visited_nodes.end());
        supernodes_num++;
        // // 利用Dijkstra求最短路径
        // Dijkstra(expand_data[supernoed_id]);
    }

    void write_supernode(const std::string &efile){
        ofstream outfile(efile);
        if(!outfile){
           std::cout << "open file failed. " << efile <<std::endl;
            exit(0);
        }
        for(vertex_t i = 0; i < supernodes_num; i++){
            // outfile << getOriginId(expand_data[i].id) << ":inner_ids(" << expand_data[i].inner_ids.size() << "): ";
            outfile << getOriginId(expand_data[i].id) << ":ids(" << expand_data[i].ids.size() << "): ";
            // for(auto id : expand_data[i].inner_ids){
            //     outfile << getOriginId(id) << ", ";
            // }
            // outfile << "\nbound_ids(" << expand_data[i].bound_ids.size() << "): ";
            // for(auto id : expand_data[i].bound_ids){
            //     outfile << getOriginId(id) << ", ";
            // }
            outfile << "\nids(" << expand_data[i].ids.size() << "): ";
            for(auto id : expand_data[i].ids){
                outfile << getOriginId(id) << ", ";
            }
            outfile << "\ninner_edges(" << expand_data[i].inner_edges.size() << "):\n";
            for(auto edge : expand_data[i].inner_edges){
                outfile << getOriginId(edge.first) << ": " << edge.second << std::endl;
            }
            outfile << "bound_edges(" << expand_data[i].bound_edges.size() << "):\n";
            for(auto edge : expand_data[i].bound_edges){
                outfile << getOriginId(edge.first) << ": " << edge.second << std::endl;
            }
        }
    }

    void start_find(const std::string &result_analyse_file=""){
        // init Fc
        Fc = new vertex_t[nodes_num+1];
        for(vertex_t i = 0; i < nodes_num; i++) Fc[i] = Fc_default_value; // 默认值
        // init expand_data
        expand_data = new ExpandData<vertex_t, value_t>[nodes_num/MIN_NODE_NUM];

        for(vertex_t i = 0; i < nodes_num; i++){
            if(Fc[i] != Fc_default_value) continue;
            vertex_t source = i; // 指定一个源点
            bool rt = first_phase(source);
            if(!rt) continue;
            second_phase(source);
            third_phase(source);
            if(i % 10000 == 0){
               std::cout << "----id=" << i << " supernodes_num=" << supernodes_num << std::endl;
            }
        }
        // 为每个结构计算内部边权
#pragma omp parallel for //num_threads(12)
        for(vertex_t i = 0; i < supernodes_num; i++){
            // 利用Dijkstra求最短路径
            // Dijkstra(expand_data[i]);
            init_ExpandData(expand_data[i]);
        }
#pragma omp barrier

        // 统计
        vertex_t inner_edges_num = 0;
        vertex_t bound_edges_num = 0;
        // vertex_t supernodes_comtain_num = supernodes_inner_num + supernodes_bound_num;
        vertex_t supernodes_comtain_num = 0;
        for(vertex_t i = 0; i < supernodes_num; i++){
            inner_edges_num += expand_data[i].inner_edges.size();
            bound_edges_num += expand_data[i].bound_edges.size();
            supernodes_comtain_num += expand_data[i].ids.size();
        }

        // 测试
        // vertex_t ordinary_num = 0;
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     if(Fc[i] == Fc_default_value){
        //         ordinary_num++;
        //     }
        // }

        std::cout << "nodes_num=" << nodes_num << std::endl;
        std::cout << "edges_num=" << edges_num << std::endl;
        std::cout << "supernodes_num=" << supernodes_num << std::endl; 
        std::cout << "supernodes_comtain_num=" << supernodes_comtain_num  << std::endl; 
        std::cout << "inner_edges_num=" << inner_edges_num  << std::endl; 
        std::cout << "bound_edges_num=" << bound_edges_num  << std::endl; 
        std::cout << "supernodes_comtain_num/nodes_num=" << (supernodes_comtain_num*1.0/nodes_num)  << std::endl; 
        // 统计结果写入文件
        ofstream outfile(result_analyse_file, std::ios::app);
        if(!outfile){
            std::cout << "[find_pattern_sssp:start_find] open result_analyse_file failed. " << result_analyse_file <<std::endl;
            exit(0);
        }
        outfile << "nodes_num:" << nodes_num << std::endl;
        outfile << "edges_num:" << edges_num << std::endl;
        outfile << "supernodes_num:" << supernodes_num << std::endl; 
        outfile << "supernodes_comtain_num:" << supernodes_comtain_num  << std::endl; 
        outfile << "inner_edges_num:" << inner_edges_num  << std::endl; 
        outfile << "bound_edges_num:" << bound_edges_num  << std::endl; 
        outfile << "supernodes_comtain_num/nodes_num:" << (supernodes_comtain_num*1.0/nodes_num)  << std::endl; 
    }

    vertex_t getOriginId(const vertex_t newid){
        return vertex_reverse_map[newid];
    }

    vertex_t getNewId(const vertex_t oldid){
        return vertex_map[oldid];
    }

    /**
     * 根据超点id删除超点，注意并未释放数组中点的空间
    */
    void delete_supernode(const vertex_t supernodeid){
        ExpandData<vertex_t, value_t> &expand_v = expand_data[supernodeid];
        // clear Fc
        for(auto u : expand_v.ids){
            Fc[u] = Fc_default_value;
        }
        ExpandData<vertex_t, value_t> &expand_end = expand_data[supernodes_num-1];
        // updata Fc_map
        Fc_map[expand_end.id] = supernodeid;
        Fc_map[expand_v.id] = Fc_map_default_value;
        // clear supernode 
        expand_v.swap(expand_end);
        expand_end.clear();
        supernodes_num--;
    }

    /*
        根据更新边还更新图和压缩结构：
        注意：更新边不涉及新的顶点
    */
    void increment_update(){
        char type;
        vertex_t u, v;
        value_t w;
        for(auto edge : update_edges){
            type = edge.type;
            u = edge.source;
            v = edge.destination;
            w = edge.weight;
            std::cout << "update: " << type << " " << u << "->" << v << " " << w << std::endl;
            if(type == 'a'){
                // + edge(u, v)
                nodes[u].out_adj.emplace_back(std::pair<vertex_t, value_t>(v, w));
                nodes[v].in_adj.emplace_back(std::pair<vertex_t, value_t>(u, w));
                // 1. u is a innner node
                if(Fc[u] != Fc_default_value){
                    ExpandData<vertex_t, value_t> &expand = expand_data[Fc_map[Fc[u]]];
                    // 1.1 inner id -> same inner id
                    if(Fc[v] == Fc[u]){
                        Dijkstra(expand);
                    }
                    // 1.2/1.4 inner id -> out id or supernode' s
                    // else if(Fc[v] == Fc_default_value or expand_data[Fc_map[Fc[v]]].id == v){
                    else{
                        // s->u: dist(u)
                        auto it = std::find_if(expand.inner_edges.begin(), expand.inner_edges.end(), isEqualALL<vertex_t>(u));
                        std::cout << "it=" << it->first << " " << it->second << std::endl;
                        if(it != expand.inner_edges.end()){
                            // u is boundnode
                            auto it_v = std::find_if(expand.bound_edges.begin(), expand.bound_edges.end(), isEqualALL<vertex_t>(v));
                            if(it_v != expand.bound_edges.end()){
                                it_v->second = std::min(it_v->second, it->second + w);
                                std::cout << "--" << it_v->second << " " << (it->second + w) << std::endl;
                            }
                            else{
                                expand.bound_edges.emplace_back(v, it->second + w);
                                 std::cout << "==" << v << " " << (it->second + w) << std::endl;
                            }
                        }
                        else{
                            std::cout << "error.... u not in inner_edges" << std::endl;
                            exit(0);
                        }
                    }
                    // 1.3: inner id -> other inner id, decompress Fc[v]
                    if(Fc[v] != Fc_default_value && expand_data[Fc_map[Fc[v]]].id != v){
                        std::cout << "case: 1.3" << std::endl;
                        delete_supernode(Fc_map[Fc[v]]);
                    }
                }
                // 2. u is a out node
                else{
                    // 2.2 out node -> inner node
                    if(Fc[v] != Fc_default_value && expand_data[Fc_map[Fc[v]]].id != v){
                        std::cout << "case: 2.2" << std::endl;
                        delete_supernode(Fc_map[Fc[v]]);
                    }
                }
            }
            else{
                // - edge(u, v)
                
            }
        }
    }

    ~FindPattern(){
        delete[] nodes;
        delete[] Fc;
        delete[] expand_data;
        delete app_;
    }

public:
    unordered_map<vertex_t, vertex_t> vertex_map; //原id: 新id
    unordered_map<vertex_t, vertex_t> vertex_reverse_map; // 新id: 原id
    unordered_map<vertex_t, vertex_t> Fc_map; // id: superid
    Node<vertex_t, value_t> *nodes;
    vertex_t nodes_num=0;
    vertex_t edges_num=0;
    vertex_t supernodes_num=0;
    vertex_t MAX_HOP=4; // 查找的跳数
    vertex_t MAX_NODE_NUM=100; // 结构内包含的最大顶点数量
    vertex_t MIN_NODE_NUM=4;   // 结构内包含的最小顶点数量
    vertex_t *Fc; // fc[x]=s, s->x
    ExpandData<vertex_t, value_t> *expand_data; // 每个结构内信息
    const int Fc_default_value = -1; // Fc[]的默认值
    const int Fc_map_default_value = -1; // Fc_map的默认值
    std::vector<UpdateEdge<vertex_t, value_t>> update_edges; // 存储更新的边
    IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>>> *app_; 
};  

// int main(int argc,char *argv[]) {
//     // g++ find_pattern_sssp.cc && ./a.out ../input/test_data_sssp_pattern_2.e
//     // g++ find_pattern_sssp.cc && ./a.out /home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31.e
//     // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted.e
//     // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted.e
//     timer_start(true);
//     std::string efile(argv[1]); 
//     FindPatternForSSSP<int, int> finder = FindPatternForSSSP<int, int>();
//     timer_next("load_graph");
//     finder.load(efile);
//     timer_next("find pattern");
//     finder.start();
//     timer_next("wirte file");
//     finder.write_supernode("../out/a_pattern");
//     timer_end(false);

//     return 0;
// }

#endif  // TOOLS_FIND_PATTERN_H_