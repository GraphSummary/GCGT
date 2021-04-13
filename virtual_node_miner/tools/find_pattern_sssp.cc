/*
    查找SSSP中的特殊的结构，即只有一个入度和一个出度的结构
*/
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

using std::vector;
using std::cout;
using std::endl;
using std::unordered_map;
using std::unordered_set;
using std::ifstream;
using std::ofstream;

#define MAX_DIST 0xffff

template<class vertex_t, class value_t>
class Node
{
public:
    vertex_t id;
    vertex_t tag=0;
    std::vector<std::pair<vertex_t, value_t>> in_adj;
    std::vector<std::pair<vertex_t, value_t>> out_adj;
};

template<class vertex_t, class value_t>
class EdgeData
{
public:
    vertex_t source;
    vertex_t destination;
    value_t weight;
    EdgeData(vertex_t s, vertex_t d, value_t w){
        source = s;
        destination = d;
        weight = w;
    }
};

template<class vertex_t, class value_t>
class ExpandData
{
public:
    vertex_t id;
    std::vector<vertex_t> ids; // 属于这个结构的id set
    std::vector<std::pair<vertex_t, value_t>> edges; // s: (d, weight)
    void print_edge(){
        std::cout << "id=" << id << " size=" << edges.size() << std::endl;
        for(auto edge : edges){
            std::cout << edge.first << ": " << edge.second << std::endl;
        }
    }
};

template<class vertex_t, class value_t>
class FindPatternForSSSP{
public:
    void load(const std::string &efile){
        std::ifstream inFile(efile);
        if(!inFile){
            cout << "open file failed. " << efile << endl;
            exit(0);
        }
        cout << "finish read file... " << efile << endl;
        vertex_t u, v;
        value_t w;
        std::vector<EdgeData<vertex_t, value_t>> edges;
        vertex_t max_id;
        while (inFile >> u >> v >> w) {
            assert(u >= 0);
            assert(v >= 0);
            assert(w >= 0);
            max_id = std::max(max_id, u);
            max_id = std::max(max_id, v);
            edges.emplace_back(EdgeData<vertex_t, value_t>(u, v, w));
        }

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

    void Dijkstra(ExpandData<vertex_t, value_t> &expand_data, const vertex_t source){
        // std::cout << "Dijkstra ..." << std::endl;
        std::vector<vertex_t> node_set = expand_data.ids; 
        unordered_map<vertex_t, value_t> dist_map;
        unordered_map<vertex_t, bool> visited;
        for(auto u : node_set){
            dist_map[u] = MAX_DIST;
            visited[u] = 0;
        }
        dist_map[source] = 0;
        vertex_t max_cnt = node_set.size();
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
                vertex_t u = edge.first;
                if(dist_map[u] > dist_map[min_j] + edge.second){
                    dist_map[u] = dist_map[min_j] + edge.second;
                }
            }
        }
        for(auto u : node_set){
            expand_data.edges.emplace_back(std::pair<vertex_t, value_t>(u, dist_map[u]));
            // std::cout << getOriginId(u) << ": " << dist_map[u] << std::endl;
        }

        // expand_data.print_edge();
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

        while(!old_q.empty() && step < MAX_HOP){
            step++;
            while(!old_q.empty()){
                vertex_t q = old_q.front();
                // std::cout << nodes[q].id << ",";
                old_q.pop();
                for(auto edge: nodes[q].out_adj){
                    vertex_t u = edge.first;
                    if(Fc[u] != u) continue;  // 已经位于其他结构中了
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
        // unordered_set<vertex_t> visited_nodes;
        bool *visited_nodes = new bool[nodes_num];  // 考虑换掉，这样申请内存太浪费了
        bool *in_queue_nodes = new bool[nodes_num];

        for(auto edge : nodes[source].out_adj){
            vertex_t u = edge.first;
            old_q.push(u);
            in_queue_nodes[u] = 1;
            visited_nodes[u] = 1;
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
        // expand_data[supernoed_id].id = supernoed_id;
        expand_data[supernoed_id].id = source;
        expand_data[supernoed_id].ids.insert(expand_data[supernoed_id].ids.begin(), visited_nodes.begin(), visited_nodes.end());
        supernodes_num++;
        // 利用Dijkstra求最短路径
        Dijkstra(expand_data[supernoed_id], source);
    }

    void write_supernode(const std::string &efile){
        ofstream outfile(efile);
        if(!outfile){
            cout << "open file failed. " << efile << endl;
            exit(0);
        }
        for(vertex_t i = 0; i < supernodes_num; i++){
            outfile << getOriginId(expand_data[i].id) << ":\nids(" << expand_data[i].ids.size() << "): ";
            for(auto id : expand_data[i].ids){
                outfile << getOriginId(id) << ", ";
            }
            outfile << "\nedges(" << expand_data[i].edges.size() << "):\n";
            for(auto edge : expand_data[i].edges){
                outfile << getOriginId(edge.first) << ": " << edge.second << std::endl;
            }
        }
    }

    void start(){
        // init Fc
        Fc = new vertex_t[nodes_num+1];
        for(vertex_t i = 0; i < nodes_num; i++) Fc[i] = i;
        // init expand_data
        expand_data = new ExpandData<vertex_t, value_t>[nodes_num/MIN_NODE_NUM];

        for(vertex_t i = 0; i < nodes_num; i++){
            if(Fc[i] != i) continue;
            vertex_t source = i; // 指定一个源点
            bool rt = first_phase(source);
            if(!rt) continue;
            second_phase(source);
            third_phase(source);
            if(i % 10000 == 0){
                cout << "----id=" << i << " supernodes_num=" << supernodes_num << std::endl;
            }
            // break;
        }

        vertex_t supernodes_comtain_num = 0;
        for(vertex_t i = 0; i < supernodes_num; i++){
            supernodes_comtain_num += expand_data[i].ids.size();
        }

        std::cout << "nodes_num=" << nodes_num << " supernodes_num=" << supernodes_num << std::endl; 
        std::cout << "supernodes_comtain_num=" << supernodes_comtain_num  << std::endl; 
        std::cout << "supernodes_comtain_num/nodes_num=" << (supernodes_comtain_num*1.0/nodes_num)  << std::endl; 

        return ;

/*
        vertex_t s = 0; // 指定一个源点
        nodes[s].tag = 1;
        std::queue<vertex_t> old_q;
        std::queue<vertex_t> new_q;
        int step = 0;
        bool *visited_nodes = new bool[nodes_num];
        bool *in_queue_nodes = new bool[nodes_num];

        for(auto u : nodes[s].out_adj){
            nodes[u].tag = 1;
            old_q.push(u);
            in_queue_nodes[u] = 1;
            visited_nodes[u] = 1;
        }
        // for(int i = 0; i < nodes_num; i++){
        //     std::cout << visited_nodes[i] << " ";
        // }
        // for(int i = 0; i < nodes_num; i++){
        //     std::cout << nodes[i].id << ":\nin:";
        //     for(auto in : nodes[i].in_adj){
        //         std::cout << nodes[in].id << ",";
        //     }
        //     std::cout << "\nout:";
        //     for(auto in : nodes[i].out_adj){
        //         std::cout << nodes[in].id << ",";
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;
        while(!old_q.empty()){
            step++;
            while(!old_q.empty()){
                vertex_t q = old_q.front();
                std::cout << nodes[q].id << ",";
                old_q.pop();
                in_queue_nodes[q] = 0;
                // pull 入邻居的tag确定自己的tag
                value_t tag = 1;
                for(auto u : nodes[q].in_adj){
                    tag &= nodes[u].tag;
                    if(tag != 1){
                        break;
                    }
                }
                nodes[q].tag = tag;
                if(tag == 1){
                    for(auto u: nodes[q].out_adj){
                        nodes[u].tag = 1;
                        if (in_queue_nodes[u] == 0 && visited_nodes[u] == 0){
                            new_q.push(u);
                            in_queue_nodes[u] = 1;
                            visited_nodes[u] = 1;
                        }
                    } 
                }
                else{
                    for(auto u : nodes[q].out_adj){
                        Node<vertex_t, value_t> &node = nodes[u];
                        if(node.tag == 1){
                            node.tag = 0;
                            if (in_queue_nodes[u] == 0){ // 这种情况，只需要不再队列中都需要加入队列
                                new_q.push(u);
                                in_queue_nodes[u] = 1;
                                visited_nodes[u] = 1;
                            }
                        }
                    }
                }
            }
            std::cout << "step=" << step << std::endl;
            old_q.swap(new_q);
            std::queue<vertex_t> empty;
            new_q.swap(empty);

            for(vertex_t i = 0; i < nodes_num; i++){
                Node<vertex_t, value_t> &node = nodes[i];
                std::cout << node.id << ":" << node.tag << ", ";
            }
            std::cout << std::endl;
            if(step > 6){
                break;
            }
        }
    */
        
    }

    vertex_t getOriginId(const vertex_t newid){
        return vertex_reverse_map[newid];
    }

    vertex_t getNewId(const vertex_t oldid){
        return vertex_map[oldid];
    }

public:
    unordered_map<int, int> vertex_map; //原id: 新id
    unordered_map<int, int> vertex_reverse_map; // 新id: 原id
    Node<vertex_t, value_t> *nodes;
    vertex_t nodes_num=0;
    vertex_t supernodes_num=0;
    vertex_t MAX_HOP=4; // 查找的跳数
    vertex_t MAX_NODE_NUM=100; // 结构内包含的最大顶点数量
    vertex_t MIN_NODE_NUM=4;   // 结构内包含的最小顶点数量
    vertex_t *Fc; // fc[x]=s, s->x
    ExpandData<vertex_t, value_t> *expand_data; // 每个结构内信息
};  

int main(int argc,char *argv[]) {
    // g++ find_pattern_sssp.cc && ./a.out ../input/test_data_sssp_pattern_2.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted.e
    timer_start(true);
    std::string efile(argv[1]); 
    FindPatternForSSSP<int, int> finder = FindPatternForSSSP<int, int>();
    timer_next("load_graph");
    finder.load(efile);
    timer_next("find pattern");
    finder.start();
    timer_next("wirte file");
    finder.write_supernode("../out/a_pattern");
    timer_end(false);

    return 0;
}