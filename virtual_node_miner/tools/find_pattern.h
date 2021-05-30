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
#include "../app/IterateKernel.h"
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
    void read_data(std::string &line, vertex_t& u, vertex_t& v, value_t& w=1) {
        std::string linestr(line);
        int pos = linestr.find_first_of(" "); //找到第一个空格的位置
        if (pos == -1) return; 

        int source = atoi(linestr.substr(0, pos).c_str()); //字符串转换成数值  源点
        vector<int> linkvec;
        std::string links = linestr.substr(pos + 1);
        // cout<<"links:"<<links<<endl;
        if (*(links.end() - 1) != ' ') {
            links = links + " ";
        }
        int spacepos = 0;
        while ((spacepos = links.find_first_of(" ")) != links.npos) {
            int to;
            if (spacepos > 0) {
                to = atoi(links.substr(0, spacepos).c_str());
            }
            else{
                LOG(INFO) << "[read_data]: " << to << "，"<< line;
            }
            links = links.substr(spacepos + 1);
            linkvec.push_back(to);
        }
        u = source;
        v = linkvec[0];
        if(linkvec.size() > 1){
            w = linkvec[1]; // edge weight
        }
    }

    void load(const std::string &efile){
        // std::ifstream inFile(efile);
        // if(!inFile){
        //    std::cout << "open file failed. " << efile <<std::endl;
        //     exit(0);
        // }
        // std::cout << "finish read file... " << efile <<std::endl;
        // vertex_t u, v;
        // value_t w;
        // std::vector<Edge<vertex_t, value_t>> edges;
        // vertex_t max_id;
        // while (inFile >> u >> v >> w) {
        //     assert(u >= 0);
        //     assert(v >= 0);
        //     assert(w >= 0);
        //     max_id = std::max(max_id, u);
        //     max_id = std::max(max_id, v);
        //     edges.emplace_back(Edge<vertex_t, value_t>(u, v, w));
        // }

        std::cout << "read file... " << efile << std::endl;
        std::ifstream inFile;
        inFile.open(efile);
        if (!inFile) {
            LOG(INFO) << "open file failed. " << efile;
            exit(0);
        }
        char linechr[2024000];
        vertex_t u, v;
        value_t w=1;
        std::vector<Edge<vertex_t, value_t>> edges;
        vertex_t max_id;
        while (inFile.getline(linechr, 2024000)) { //read a line of the input file, ensure the buffer is large enough
            std::string line(linechr);
            if(line.length() < 1 || line[0] == '#'){
                continue;
            }
            read_data(line, u, v, w); //invoke api, get the value of key field and data field
            assert(u >= 0);
            assert(v >= 0);
            assert(w >= 0);
            max_id = std::max(max_id, u);
            max_id = std::max(max_id, v);
            edges.emplace_back(Edge<vertex_t, value_t>(u, v, w));
            // std::cout << u << " " << v << " " << w << std::endl;
        }
        inFile.close();
        std::cout << "finish read file... " << efile <<std::endl;


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
        // find max out degree
        {   
            vertex_t id = 0;
            vertex_t max = -1;
            vertex_t temp = 0;
            for(vertex_t i = 0; i < nodes_num; i++){
                temp = nodes[i].out_adj.size();
                if(temp > max){
                    max = temp;
                    id = i;
                }
            }
            std::cout << "--------max_id=" << id << " max_degree=" << max << std::endl;
        }
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

    /**
     * 求索引时使用
    */
    void init_node(const std::vector<vertex_t> &node_set, const vertex_t& source){
        value_t delta;
        value_t value;
        vector<std::pair<vertex_t, value_t>> data;
        for(auto i : node_set){
            app_->init_c(i, nodes[i].recvDelta, data, source); // 注意：vertex_reverse_map[source]： sssp
            this->nodes[i].oldDelta = app_->default_v();
            app_->init_v(i, nodes[i].value, data);
        }
    }

    /**
     * 正常的初始化
    */
    void init_node(){
        // value_t delta;
        // value_t value;
        // vector<std::pair<vertex_t, value_t>> data;
        // for(auto i : node_set){
        //     app_->init_c(vertex_reverse_map[i], nodes[i].recvDelta, data);
        //     this->nodes[i].oldDelta = app_->default_v();
        //     app_->init_v(i, nodes[i].value, data);
        // }
        value_t delta;
        value_t value;
        vector<std::pair<vertex_t, value_t>> data;
        for(vertex_t i = 0; i < this->nodes_num; i++){
            this->app_->init_c(this->vertex_reverse_map[i], this->nodes[i].recvDelta, data);
            this->nodes[i].oldDelta = this->app_->default_v();
            this->app_->init_v(i, this->nodes[i].value, data);
        }
    }

    void run_to_convergence(std::vector<vertex_t> &node_set, const vertex_t source){
        bool is_convergence = false;
        int step = 0;
        unordered_map<vertex_t, value_t> last_value;
        float Diff = 0;
        /* init last_value */
        for(auto v : node_set){
            last_value[v] = app_->default_v();
        }

        /* 最后需要: value和recvDelta */
        while(true){
            step++;
            is_convergence = true;

            // debug
            // {
            //     for(auto u : node_set){
            //         Node<vertex_t, value_t>& node = nodes[u];
            //         std::cout << "step=" << step << " id=" << getOriginId(u) << " value=" << node.value << " oldDelta=" << node.oldDelta << 
            //         " recvDelta=" << node.recvDelta << std::endl;     
            //     }
            // }   

            // receive
            for(auto v : node_set){
                Node<vertex_t, value_t>& node = nodes[v];
                // value_t old_value = node.value;
                app_->accumulate(node.value, node.recvDelta); // delat -> value
                // if(old_value != node.value){
                // if(fabs(old_value - node.value) > FLAGS_convergence_threshold){
                //     is_convergence = false;
                // }
                app_->accumulate(node.oldDelta, node.recvDelta); // updata delta
                node.recvDelta = app_->default_v();
            }
            // send
            for(auto v : node_set){
                Node<vertex_t, value_t>& node = nodes[v];
                // if(fabs(node.oldDelta - app_->default_v()) > FLAGS_convergence_threshold * 0.001){
                    for(auto &edge : node.out_adj){ // i -> adj
                        // if(Fc[edge.first].find(source) != Fc[edge.first].end()){ // 只发给内部点
                        // if(std::find(Fc[edge.first].begin(), Fc[edge.first].end(), source) == Fc[edge.first].end()){ // 只发给内部点
                        if(std::find(node_set.begin(), node_set.end(), edge.first) == node_set.end()){ // 只发个内部点
                            continue;
                        }
                        value_t& recvDelta = nodes[edge.first].recvDelta; // adj's recvDelta
                        value_t sendDelta; // i's 
                        // app_->g_func(node.oldDelta, edge.second, sendDelta);
                        app_->g_func(node.id, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                        app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                    }
                    node.oldDelta = app_->default_v(); // delta发完需要清空 
                // }
            }
            // check convergence
            Diff = 0;
            for(auto v : node_set){
                Node<vertex_t, value_t>& node = nodes[v];
                Diff += fabs(last_value[v] - node.value);
                last_value[v] = node.value;
            }

            // if(is_convergence || step > 1000){
            if(Diff * node_set.size() <= FLAGS_convergence_threshold || step > 200){
            // if(step == 2){
                if(step > 200){
                    LOG(INFO) << "compress: step>200 Diff=" << Diff;
                }
                break;
            }
        }
    }

    /**
     * min/max
     * 计算超点内部需要的值，以及重新为边界点建立新边
    */
    void init_ExpandData(ExpandData<vertex_t, value_t> &expand_data){
        std::vector<vertex_t> &node_set = expand_data.ids; 
        const vertex_t source = expand_data.id;
        // init supernode
        init_node(node_set, source);
       
        /* iterative calculation */
        run_to_convergence(node_set, source);
        // std::cout << "step=" << step << std::endl;

        /**
         * create index in supernode 
         **/
        expand_data.inner_edges.clear();
        expand_data.bound_edges.clear();
        // 为结构加入直接指向外部点的最短距离边： source -> 外部id
        unordered_map<vertex_t, value_t> bound_map;  // out_node: dist(out_node)
        value_t sendDelta;
        for(auto u : node_set){
            Node<vertex_t, value_t>& node = nodes[u];
            expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(u, node.value)); // 为结构加入直接指向内部点的最短距离边： source -> 内部id
            for(auto &edge : node.out_adj){
                // if(Fc[edge.first] != source){  // 可能会出现发给同一个点
                // if(Fc[edge.first].find(source) == Fc[edge.first].end()){  // 保证edge.first是边界点
                if(std::find(Fc[edge.first].begin(), Fc[edge.first].end(), source) == Fc[edge.first].end()){ // 保证edge.first是边界点
                    // app_->g_func(node.value, edge.second, sendDelta);
                    app_->g_func(node.id, node.value, node.value, node.out_adj, edge, sendDelta);
                    if(bound_map.find(edge.first) == bound_map.end()){
                        // bound_map[edge.first] = node.value + edge.second;
                        bound_map[edge.first] = sendDelta;
                    }
                    else{
                        // bound_map[edge.first] = std::min(bound_map[edge.first], node.value + edge.second);
                        app_->accumulate(bound_map[edge.first], sendDelta);
                    }
                }
            }
        }
        for(auto kv : bound_map){
            expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second));
        } 
    }

    /**
     * sum， 需要修正delta
     * 计算超点内部需要的值，以及重新为边界点建立新边
    */
    void init_ExpandData_iter(ExpandData<vertex_t, value_t> &expand_data){
        std::vector<vertex_t> &node_set = expand_data.ids; 
        const vertex_t source = expand_data.id;
        // init supernode
        init_node(node_set, source);

        /* first iterative calculation */
        run_to_convergence(node_set, source);

        /**
         * create index in supernode 
         **/
        expand_data.inner_edges.clear();
        expand_data.inner_delta.clear();
        expand_data.bound_edges.clear();
        // 为结构加入直接指向外部点的最短距离边： source -> 外部id
        unordered_map<vertex_t, value_t> bound_map;  // out_node: dist(out_node)
        value_t sendDelta;

        for(auto u : node_set){
            Node<vertex_t, value_t>& node = nodes[u];
            /* 过滤掉每个入口都建立索引导致的不必要索引(可能导致误差) */
            /* 分别建立： value和delta的映射 */
            if(fabs(node.value - app_->default_v()) * nodes_num > FLAGS_convergence_threshold ){
                // expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(u, node.value/0.8)); // 为结构加入直接指向内部点的最短距离边： source -> 内部id
                expand_data.inner_edges.emplace_back(std::pair<vertex_t, value_t>(u, app_->g_revfunc(node.value))); // 为结构加入直接指向内部点的最短距离边： source -> 内部id
                // std::cout << getOriginId(u) << " value=" << node.value << std::endl;
                for(auto &edge : node.out_adj){
                    // if(Fc[edge.first] != source){  // 可能会出现发给同一个点
                    // std::cout << getOriginId(u) << "->" << getOriginId(edge.first) << " value=" << node.value << std::endl;
                    if(std::find(Fc[edge.first].begin(), Fc[edge.first].end(), source) == Fc[edge.first].end()){  // 保证edge.first是边界点
                        // app_->g_func(node.value, edge.second, sendDelta);
                        app_->g_func(node.id, node.value, node.value, node.out_adj, edge, sendDelta);
                        if(bound_map.find(edge.first) == bound_map.end()){
                            // bound_map[edge.first] = node.value + edge.second;
                            bound_map[edge.first] = sendDelta;
                        }
                        else{
                            // bound_map[edge.first] = std::min(bound_map[edge.first], node.value + edge.second);
                            app_->accumulate(bound_map[edge.first], sendDelta);
                        }
                    }
                }
            }
            if(fabs(node.recvDelta - app_->default_v()) * nodes_num > FLAGS_convergence_threshold){
                // expand_data.inner_delta.emplace_back(std::pair<vertex_t, value_t>(u, node.oldDelta/0.8)); // 为内部点缓存收敛时剩余的delta
                // value_t allDelat = node.recvDelta;
                // app_->accumulate(allDelat, node.value);
                expand_data.inner_delta.emplace_back(std::pair<vertex_t, value_t>(u, app_->g_revfunc(node.recvDelta))); // 为内部点缓存收敛时剩余的delta
                // expand_data.inner_delta.emplace_back(std::pair<vertex_t, value_t>(u, node.recvDelta)); // 为内部点缓存收敛时剩余的delta
                // expand_data.inner_delta.emplace_back(std::pair<vertex_t, value_t>(u, app_->g_revfunc(allDelat))); // 为内部点缓存收敛时剩余的delta
            }
        }
        // std::cout << "source=" << source << std::endl;
        for(auto kv : bound_map){
            // 新边权= delta/d
            // expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, kv.second / 0.8)); 
            expand_data.bound_edges.emplace_back(std::pair<vertex_t, value_t>(kv.first, app_->g_revfunc(kv.second))); 
        } 
        // debug
        // {
        //     for(auto u : node_set){
        //         Node<vertex_t, value_t>& node = nodes[u];
        //         std::cout << "inner_map: " << u << " " << node.value << std::endl;
        //     }
        // }
        /* reset value */
        // i = 0;
        // for(auto u : node_set){
        //     Node<vertex_t, value_t>& node = nodes[u];
        //     if(FLAGS_php_source != vertex_reverse_map[source]){
        //         node.value = app_->default_v();
        //     }
        //     else{
        //         node.value = old_value[i];
        //         std::cout << "测试： id=" << u << " value=" << node.value << std::endl; 
        //     }
        //     i++;
        // }
    }

    /**
     * 以超点为单位计算各个点的收敛值
    */
    void init_value_iter(const vertex_t superid){
        std::vector<vertex_t> &node_set = expand_data[superid].ids; 
        const vertex_t source = expand_data[superid].id;
        /* init supernode */
        // init_node(node_set);

        /* iterative calculation */
        run_to_convergence(node_set, source);

        /* store value and delta */
        value_t sendDelta = 0;
        for(auto u : node_set){
            Node<vertex_t, value_t>& node = nodes[u];
            // app_->accumulate(init_value[u], node.value);
            // app_->accumulate(init_delta[u], node.recvDelta);
            for(auto &edge : node.out_adj){
                if(std::find(node_set.begin(), node_set.end(), edge.first) == node_set.end()){  // 保证edge.first是边界点
                    app_->g_func(node.id, node.value, node.value, node.out_adj, edge, sendDelta);
                    #pragma omp critical
                    {
                        app_->accumulate(init_delta[edge.first], sendDelta);
                    }
                }
            }
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
            while(!old_q.empty() && visited_nodes.size() <= MAX_NODE_NUM){
                vertex_t q = old_q.front();
                // std::cout << nodes[q].id << ",";
                old_q.pop();
                for(auto edge: nodes[q].out_adj){
                    vertex_t u = edge.first;
                    // if(Fc[u] != Fc_default_value) continue;  // 已经位于其他结构中了
                    if(Fc[u].size() != 0) continue;  // 已经位于其他结构中了
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
                        if (nodes[u].tag == source && in_queue_nodes[u] == 0 && visited_nodes[u] == 0 && u != source){ // source不入队列
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
                            if (in_queue_nodes[u] == 0 && u != source){ // 这种情况，只需要不在队列中都需要加入队列, source不入队列
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
     * 生成超点
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
        // std::cout << visited_nodes.size() << std::endl;
        // check size
        if(visited_nodes.size() < MIN_NODE_NUM){
            // std::cout << "visited_nodes.size() < MIN_NODE_NUM  size=" << visited_nodes.size() << std::endl;
            return;
        }
        // check cost
        // vertex_t old_edge_num = 0; // out edge of every node in supernode
        // vertex_t new_edge_num = 0; // inner node + bound edge
        // for(auto u : visited_nodes){
        //     Node<vertex_t, value_t>& node = nodes[u];
        //     old_edge_num += node.out_adj.size();
        //     for(auto v : node.out_adj){
        //         if(std::find(visited_nodes.begin(), visited_nodes.end(), v.first) == visited_nodes.end()){
        //             new_edge_num += 1;
        //         }
        //     }
        // }
        // if(old_edge_num * 1.0 / (visited_nodes.size() + new_edge_num) < 1. || (old_edge_num - new_edge_num) < new_edge_num){
        //     // std::cout << source << ": save=" << (old_edge_num * 1.0 / (visited_nodes.size() + new_edge_num)) << std::endl;
        //     return;
        // }

        for(auto u : visited_nodes){
            // Fc[u] = source;
            Fc[u].emplace_back(source);
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

    /**
     * 查找多入口的密集超点
    */
    void find_multi_source_supernode(const vertex_t source){
        unordered_set<vertex_t> S; // 属于P,从外界有入边的点
        unordered_set<vertex_t> P; // inner node, 只有内部入边的点
        unordered_set<vertex_t> O; // V^out，内部点的出边指向的外部点
        std::queue<vertex_t> D; // candidate queue
        unordered_set<vertex_t> visited_nodes;
        // add source
        visited_nodes.insert(source);
        P.insert(source);
        for(auto u : nodes[source].in_adj){
            if(u.first != source){
                S.insert(source);
                break;
            }
        }
        for(auto u : nodes[source].out_adj){
            if(visited_nodes.find(u.first) == visited_nodes.end() && Fc[u.first].size() == 0){
                D.push(u.first);
                visited_nodes.insert(u.first);
            }
            if(O.find(u.first) == O.end()){ // 排除自边
                O.insert(u.first);
            }
        }
        // std::cout << "source: " << getOriginId(source) << std::endl;
        float pre_socre = 0; // pre compressed score，这里是整个压缩结构的总体得分，不是待加入点的得分
       /* while(!D.empty() && P.size() <= MAX_NODE_NUM){
            vertex_t d = D.front();
            D.pop();
            // if(Fc[d].size() == 1 && Fc[d][0] == d){  // 作为入口的点排除
            if(Fc[d].size() > 0){  // 作为入口的点排除
                continue;
            }
            // std::cout << "test: d=" << d << std::endl;
            int old_edge_num = 0;
            // int new_in_num = S.size();
            // int new_index_num = P.size() + O.size();
            int inner_edge_num = 0;
            int bound_edge_num = 0;
            // compute old_edge_num
            for(auto v : S){
                // old_edge_num += nodes[v].out_adj.size();
                for(auto u : nodes[v].out_adj){
                    if(P.find(u.first) != P.end() || u.first == d){
                        old_edge_num++; // v -> P
                        inner_edge_num++;
                    }
                    else{
                        bound_edge_num++;
                    }
                }
            }
            for(auto u : P){
                old_edge_num += nodes[u].out_adj.size();
                for(auto v : nodes[u].out_adj){
                    if(P.find(v.first) != P.end() || v.first == d){
                        inner_edge_num++;
                    }
                    else{
                        bound_edge_num++;
                    }
                }
            }
            // if add d to supernode:
            for(auto v : nodes[d].out_adj){
                if(P.find(v.first) != P.end() || v.first == d){
                    inner_edge_num++;
                }
                else{
                    bound_edge_num++;
                }
            }
            old_edge_num += nodes[d].out_adj.size();
            // compute new_in_num
            // unordered_set<vertex_t> S_copy(S); // supernode's source, V^in
            // if(S_copy.find(d) != S_copy.end()){
            //     S_copy.erase(d);
            //     new_in_num--;
            // }
            // for(auto u : nodes[d].in_adj){
            //     if(S_copy.find(u.first) == S_copy.end() && P.find(u.first) == P.end()){
            //         S_copy.insert(u.first);
            //         new_in_num++;
            //     }
            // }
            // compute new_index_num
            // new_index_num += 1; // if d insirt P
            // unordered_set<vertex_t> O_copy(O); // supernode's source, V^in
            // if(O_copy.find(d) != O_copy.end()){
            //     O_copy.erase(d);
            //     new_index_num--;
            // }
            // for(auto u : nodes[d].out_adj){
            //     if(O_copy.find(u.first) == O_copy.end() && P.find(u.first) == P.end()){
            //         O_copy.insert(u.first);
            //         new_index_num++;
            //     }
            // }
            // float now_score = old_edge_num * 1.0 / (new_in_num * new_index_num);
            float now_score = inner_edge_num * 1.0 / (P.size() + bound_edge_num);
            // float now_score = (old_edge_num * 1.0 - new_in_num * new_index_num) / old_edge_num;
            // std::cout << "d=" << getOriginId(d) << " pre_score=" << pre_socre << " now_score=" << now_score << " old_edge_num=" << old_edge_num << " new_in_num=" << new_in_num << " new_index_num=" << new_index_num << std::endl;
            // if(now_score >= pre_socre){
            if((P.size() < 4 && now_score >= pre_socre) || now_score >= 2 ){
                pre_socre = now_score;
                P.insert(d);
                // S.swap(S_copy);
                // O.swap(O_copy);
                for(auto u : nodes[d].out_adj){
                    // if(P.find(u.first) == P.end() && visited_nodes.find(u.first) == visited_nodes.end() && Fc[u.first] == Fc_default_value){
                    if(P.find(u.first) == P.end() && visited_nodes.find(u.first) == visited_nodes.end() && Fc[u.first].size() == 0){
                        D.push(u.first);
                        visited_nodes.insert(u.first);
                    }
                }
                // for(auto u : nodes[d].in_adj){
                //     // if(P.find(u.first) == P.end() && visited_nodes.find(u.first) == visited_nodes.end() && Fc[u.first] == Fc_default_value){
                //     if(P.find(u.first) == P.end() && visited_nodes.find(u.first) == visited_nodes.end()){
                //         D.push(u.first);
                //         visited_nodes.insert(u.first);
                //     }
                // }
            }
        }*/
        
        vertex_t inner_edge_num = 0;
        vertex_t bound_edge_num = nodes[source].out_adj.size();
        vertex_t temp_inner = 0;
        vertex_t temp_bound = 0;
        while(!D.empty() && P.size() <= MAX_NODE_NUM){
            vertex_t d = D.front();
            D.pop();
            // if(Fc[d].size() == 1 && Fc[d][0] == d){  // 作为入口的点排除
            if(Fc[d].size() > 0){  // 作为入口的点排除
                continue;
            }
            // if d add to P:
            bool is_s = false;
            temp_inner = inner_edge_num;
            temp_bound = bound_edge_num;
            for(auto u : nodes[d].in_adj){
                if(P.find(u.first) != P.end()){
                    temp_bound--;
                    temp_inner++;
                }
                else{
                    is_s = true;
                }
            }
            for(auto u : nodes[d].out_adj){
                if(P.find(u.first) == P.end()){
                    temp_bound++;
                }
                else{
                    temp_inner++;
                }
            }
            // update S/P
            unordered_set<vertex_t> temp_O;
            unordered_set<vertex_t> temp_S;
            // for(auto s : S){
            //     for(auto u : nodes[s].in_adj){
            //         if(P.find(u.first) == P.end() && u.first != d){
            //             temp_S.insert(s);
            //         }
            //     }
            // }
            temp_S.insert(S.begin(), S.end());
            for(auto v : nodes[d].out_adj){
                if(S.find(v.first) != S.end()){
                    bool flag = true;
                    for(auto u : nodes[v.first].in_adj){
                        if(P.find(u.first) == P.end() && u.first != d){
                            flag = false;
                            break;
                        }
                    }
                    if(flag){
                        temp_S.erase(v.first);
                    }
                }
            }
            if(is_s){
                temp_S.insert(d);
            }
            temp_O.insert(O.begin(), O.end());
            temp_O.erase(d);
            for(auto u : nodes[d].out_adj){
                if(P.find(u.first) == P.end() && u.first != d){
                    temp_O.insert(u.first);
                }
            }
            float now_score = temp_inner * 1.0 / (temp_bound + P.size());
            // if((P.size() < 4 && now_score >= pre_socre) || (now_score >= 1.2 && temp_S.size() * temp_O.size() < temp_inner * 1)){
            if((P.size() < 4) || (temp_S.size() * temp_O.size() < temp_inner * 0.9)){
                temp_O.swap(O);
                temp_S.swap(S);

                pre_socre = now_score;
                inner_edge_num = temp_inner;
                bound_edge_num = temp_bound;
                P.insert(d);
                for(auto u : nodes[d].out_adj){
                    if(Fc[u.first].size() == 0 && P.find(u.first) == P.end() && visited_nodes.find(u.first) == visited_nodes.end()){
                        D.push(u.first);
                        visited_nodes.insert(u.first);
                    }
                }
            }
        }

        if(P.size() >= MIN_NODE_NUM){
            /* 为每个入口建立一套索引 */
            for(auto source: S){
                Fc[source].emplace_back(source); // 保证，自己在Fc[source][0]的位置
            }
            supernode_ids.emplace_back(supernodes_num);
            for(auto source : S){
                for(auto u : P){
                    if(source != u){
                        Fc[u].emplace_back(source);
                    }
                }  
                // std::cout << std::endl;
                // build a supernode
                vertex_t supernoed_id = supernodes_num;
                Fc_map[source] = supernoed_id;
                expand_data[supernoed_id].id = source;
                expand_data[supernoed_id].ids.insert(expand_data[supernoed_id].ids.begin(), P.begin(), P.end());
                if(P.find(source) == P.end()){
                    expand_data[supernoed_id].ids.emplace_back(source);
                }
                supernodes_num++;
            }
        }
    }

    /**
     * 查找path
    */
    void find_path(const vertex_t source){
        unordered_set<vertex_t> P; // inner node
        vertex_t next_node = source;
        while(nodes[next_node].out_adj.size() == 1 && P.size() < MAX_NODE_NUM && Fc[next_node].size() == 0){
            if(P.find(next_node) == P.end()){
                P.insert(next_node);
                next_node = nodes[next_node].out_adj[0].first;
            }
            else{
                break;
            }
        }
        if(P.size() >= MIN_NODE_NUM){
            Fc[source].emplace_back(source);
            for(auto p : P){
                Fc[p].emplace_back(source);
            }
            vertex_t supernoed_id = supernodes_num;
            Fc_map[source] = supernoed_id;
            expand_data[supernoed_id].id = source;
            supernode_ids.emplace_back(supernoed_id); // 只存一份
            expand_data[supernoed_id].ids.insert(expand_data[supernoed_id].ids.begin(), P.begin(), P.end());
            supernodes_num++;
        }
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
            outfile << "inner_delta(" << expand_data[i].inner_delta.size() << "):\n";
            for(auto edge : expand_data[i].inner_delta){
                outfile << getOriginId(edge.first) << ": " << edge.second << std::endl;
            }
            outfile << "bound_edges(" << expand_data[i].bound_edges.size() << "):\n";
            for(auto edge : expand_data[i].bound_edges){
                outfile << getOriginId(edge.first) << ": " << edge.second << std::endl;
            }
            outfile << "\n";
        }
    }

    void compute_index(const vertex_t j, const vertex_t spn_num){
        for(vertex_t i = supernode_ids[j]; (j < spn_num-1 && i < supernode_ids[j+1]) || (j == spn_num-1 && i < supernodes_num); i++){
            // 利用Dijkstra求最短路径
            // Dijkstra(expand_data[i]);
            if(FLAGS_app == "sssp"){
                init_ExpandData(expand_data[i]); // traversal: sssp
            }
            else if(FLAGS_app == "php" || FLAGS_app == "pagerank"){
                init_ExpandData_iter(expand_data[i]); // iterative: pr, php
            }
            else{
                LOG(INFO) << "no this app....";
                exit(0);
            }
            if(i % 5000 == 0){
            std::cout << "----id=" << i << " computing index" << std::endl;
            }
        }
    }
  
    void start_find(const std::string &result_analyse_file=""){
        // init app.weight_sum
        this->app_->iterate_begin(this->nodes, this->nodes_num);
        // init Fc
        // Fc = new vertex_t[nodes_num+1];
        // for(vertex_t i = 0; i < nodes_num; i++) Fc[i] = Fc_default_value; // 默认值
        Fc.resize(nodes_num);
        // init expand_data
        expand_data = new ExpandData<vertex_t, value_t>[nodes_num];

        std::cout << "start compress" << std::endl;
        /* find single source supernode */
/*        for(vertex_t i = 0; i < nodes_num; i++){
            if(Fc[i] != Fc_default_value) continue;
            vertex_t source = i; // 指定一个源点
            // std::cout << "source=" << vertex_reverse_map[i] << std::endl;
            bool rt = first_phase(source);
            // std::cout << "finish 1 phase" << std::endl;
            if(!rt) continue;
            second_phase(source);
            // std::cout << "finish 2 phase" << std::endl;
            third_phase(source);
            // std::cout << "finish 3 phase" << std::endl;
            if(i % 10000 == 0){
               std::cout << "----id=" << i << " supernodes_num=" << supernodes_num << std::endl;
            }
        }
*/
        /* find multi-source supernode */
        for(vertex_t i = 0; i < nodes_num; i++){
            if(Fc[i].size() > 0) continue;
            find_multi_source_supernode(i);
        
            if(i % 10000 == 0){
               std::cout << "----id=" << i << " supernodes_num=" << supernodes_num << std::endl;
            }
        }
        LOG(INFO) << "find multi-source supernode...";
        /* find path */
        // for(vertex_t i = 0; i < nodes_num; i++){
        //     if(Fc[i].size() > 0) continue;
        //     find_path(i);
        //     if(i % 10000 == 0){
        //        std::cout << "----id=" << i << " supernodes_num=" << supernodes_num << std::endl;
        //     }
        // }
        LOG(INFO) << "find finish... start compute index...";

        vertex_t spn_num = supernode_ids.size();
        /* 为每个结构计算index */
#pragma omp parallel for //num_threads(12)
        for(vertex_t j = 0; j < spn_num; j++){
            compute_index(j, spn_num);
        }
#pragma omp barrier

        LOG(INFO) << "find index... start compute init value...";
        /* 超点内部初始化 */
        init_node();
        init_delta.resize(nodes_num);
        init_value.resize(nodes_num);
        for(vertex_t i = 0; i < this->nodes_num; i++){
            init_delta[i] = this->app_->default_v();
            init_value[i] = this->app_->default_v();
        }
        LOG(INFO) << "--- supernode_ids.size=" << spn_num;
#pragma omp parallel for
        for(vertex_t i = 0; i < spn_num; i++){
            init_value_iter(supernode_ids[i]);
            if(i % 100 == 0){
               std::cout << "----id=" << i << " computing init value" << std::endl;
            }
        }
#pragma omp barrier

        // 统计
        vertex_t inner_edges_num = 0;
        vertex_t bound_edges_num = 0;
        // vertex_t supernodes_comtain_num = supernodes_inner_num + supernodes_bound_num;
        vertex_t supernodes_comtain_num = 0;
        vertex_t max_node_num = 0;
        vertex_t max_inner_edges_num = 0;
        vertex_t max_bound_edges_num = 0;
        for(vertex_t i = 0; i < supernodes_num; i++){
            inner_edges_num += expand_data[i].inner_edges.size();
            bound_edges_num += expand_data[i].bound_edges.size();
            max_node_num = std::max(max_node_num, (int)expand_data[i].ids.size());
            max_inner_edges_num = std::max(max_inner_edges_num, (int)expand_data[i].inner_edges.size());
            max_bound_edges_num = std::max(max_bound_edges_num, (int)expand_data[i].bound_edges.size());
        }
        for(auto i : supernode_ids){
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
        std::cout << "supernodes_index/edges_num=" << ((inner_edges_num+bound_edges_num)*1.0/edges_num)  << std::endl; 
        std::cout << "max_node_num=" << max_node_num << std::endl; 
        std::cout << "max_inner_edges_num=" << max_inner_edges_num << std::endl; 
        std::cout << "max_bound_edges_num=" << max_bound_edges_num << std::endl; 
        std::cout << "MAX_NODE_NUM:" << MAX_NODE_NUM  << std::endl; 
        std::cout << "MIN_NODE_NUM:" << MIN_NODE_NUM  << std::endl; 

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
        outfile << "supernodes_index/edges_num:" << ((inner_edges_num+bound_edges_num)*1.0/edges_num)  << std::endl; 
        outfile << "MAX_NODE_NUM:" << MAX_NODE_NUM  << std::endl; 
        outfile << "MIN_NODE_NUM:" << MIN_NODE_NUM  << std::endl; 
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
        vertex_t del_id = Fc_map[supernodeid]; // 获取superid在数组中的索引
        ExpandData<vertex_t, value_t> &expand_v = expand_data[del_id];
        // clear Fc
        for(auto u : expand_v.ids){
            Fc[u] = Fc_default_value;
        }
        ExpandData<vertex_t, value_t> &expand_end = expand_data[supernodes_num-1];
        // updata Fc_map
        Fc_map[expand_end.id] = del_id;
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
    // void increment_update(){
    //     char type;
    //     vertex_t u, v;
    //     value_t w;
    //     for(auto edge : update_edges){
    //         type = edge.type;
    //         u = edge.source;
    //         v = edge.destination;
    //         w = edge.weight;
    //         std::cout << "update: " << type << " " << u << "->" << v << " " << w << std::endl;
    //         if(type == 'a'){
    //             // + edge(u, v)
    //             nodes[u].out_adj.emplace_back(std::pair<vertex_t, value_t>(v, w));
    //             nodes[v].in_adj.emplace_back(std::pair<vertex_t, value_t>(u, w));
    //             // 1. u is a innner node
    //             if(Fc[u] != Fc_default_value){
    //                 ExpandData<vertex_t, value_t> &expand = expand_data[Fc_map[Fc[u]]];
    //                 // 1.1 inner id -> same inner id
    //                 if(Fc[v] == Fc[u]){
    //                     Dijkstra(expand);
    //                 }
    //                 // 1.2/1.4 inner id -> out id or supernode' s
    //                 // else if(Fc[v] == Fc_default_value or expand_data[Fc_map[Fc[v]]].id == v){
    //                 else{
    //                     // s->u: dist(u)
    //                     auto it = std::find_if(expand.inner_edges.begin(), expand.inner_edges.end(), isEqualALL<vertex_t>(u));
    //                     std::cout << "it=" << it->first << " " << it->second << std::endl;
    //                     if(it != expand.inner_edges.end()){
    //                         // u is boundnode
    //                         auto it_v = std::find_if(expand.bound_edges.begin(), expand.bound_edges.end(), isEqualALL<vertex_t>(v));
    //                         if(it_v != expand.bound_edges.end()){
    //                             it_v->second = std::min(it_v->second, it->second + w);
    //                             std::cout << "--" << it_v->second << " " << (it->second + w) << std::endl;
    //                         }
    //                         else{
    //                             expand.bound_edges.emplace_back(v, it->second + w);
    //                              std::cout << "==" << v << " " << (it->second + w) << std::endl;
    //                         }
    //                     }
    //                     else{
    //                         std::cout << "error.... u not in inner_edges" << std::endl;
    //                         exit(0);
    //                     }
    //                 }
    //                 // 1.3: inner id -> other inner id, decompress Fc[v]
    //                 if(Fc[v] != Fc_default_value && expand_data[Fc_map[Fc[v]]].id != v){
    //                     std::cout << "case: 1.3" << std::endl;
    //                     delete_supernode(Fc_map[Fc[v]]);
    //                 }
    //             }
    //             // 2. u is a out node
    //             else{
    //                 // 2.2 out node -> inner node
    //                 if(Fc[v] != Fc_default_value && expand_data[Fc_map[Fc[v]]].id != v){
    //                     std::cout << "case: 2.2" << std::endl;
    //                     delete_supernode(Fc_map[Fc[v]]);
    //                 }
    //             }
    //         }
    //         else{
    //             // - edge(u, v)
    //         }
    //     }
    // }

    ~FindPattern(){
        delete[] nodes;
        // delete[] Fc;
        delete[] expand_data;
        delete app_;
    }

public:
    unordered_map<vertex_t, vertex_t> vertex_map; //原id: 新id
    unordered_map<vertex_t, vertex_t> vertex_reverse_map; // 新id: 原id
    unordered_map<vertex_t, vertex_t> Fc_map; // superid: index of expand_data
    Node<vertex_t, value_t> *nodes;
    vertex_t nodes_num=0;
    vertex_t edges_num=0;
    vertex_t supernodes_num=0;
    vertex_t MAX_HOP=4; // 查找的跳数
    vertex_t MAX_NODE_NUM=150; // 结构内包含的最大顶点数量
    vertex_t MIN_NODE_NUM=8;   // 结构内包含的最小顶点数量
    // vertex_t *Fc; // fc[x]=s, s->x
    vector<vector<vertex_t>> Fc; // fc[x]=s, s->x
    ExpandData<vertex_t, value_t> *expand_data; // 每个结构内信息
    const int Fc_default_value = -1; // Fc[]的默认值
    const int Fc_map_default_value = -1; // Fc_map的默认值
    std::vector<UpdateEdge<vertex_t, value_t>> update_edges; // 存储更新的边
    IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>>> *app_; 
    std::vector<vertex_t> supernode_ids; // 超点id
    std::vector<value_t> init_value; // 初始value
    std::vector<value_t> init_delta; // 初始delta
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