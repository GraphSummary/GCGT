/*
    处理road类型数据集
    思路：将无向图上从源点做BFS,按照BFS访问顺序重新编号，然后将原无向图中编号替换，将大顶点编号指向小编号的边进行反转，
    然后得到的图当作有向图使用，此过程中没有破坏图的结构，只是调整了某些边的方向。
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
#include <memory.h>
#include "../graph/node.h"
#include "../graph/edge.h"
#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <glog/logging.h>

using std::vector;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::ifstream;
using std::ofstream;

DEFINE_uint64(shortestpath_source, 0, "shortestpath_source");
DEFINE_string(base_edge, "", "old edge file");
DEFINE_string(output, "", "result output file");
DEFINE_bool(directed, false, "directed edge");

template<class vertex_t, class value_t>
class DealRoadDataSet{
public:
    void load_weight(const std::string &efile){
        std::ifstream inFile(efile);
        if(!inFile){
           std::cout << "open file failed. " << efile <<std::endl;
            exit(0);
        }
        std::cout << "finish open file... " << efile <<std::endl;
        vertex_t u, v;
        value_t w;
        vertex_t max_id=0; // 必须初始化，不然随机到一个很大的值，则它会被认为最大id
        std::cout << "start read edge..." << std::endl;
        while (inFile >> u >> v >> w) {
            assert(u >= 0);
            assert(v >= 0);
            assert(w >= 0);
            max_id = std::max(max_id, u);
            max_id = std::max(max_id, v);
            edges.emplace_back(Edge<vertex_t, value_t>(u, v, w));
        }
        std::cout << "finish read edge..." << std::endl;
        nodes = new Node<vertex_t, value_t>[max_id+1]; 
        std::cout << "start emplace_back edge..." << std::endl;
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
            nodes[u].id = -1; // 标记为没有分配编号
            nodes[v].id = -1;
            nodes[u].out_adj.emplace_back(std::pair<vertex_t, value_t>(v, w));
            if(!FLAGS_directed){
                nodes[v].out_adj.emplace_back(std::pair<vertex_t, value_t>(u, w));
            }
            // std::cout << v << ": " << nodes[v].tag << ", " << u << ": " << nodes[u].tag << std::endl;
        }
        std::cout << "finish emplace_back edge..." << std::endl;
        std::cout << "load finied, nodes_num=" << nodes_num << " edges_num=" << edges.size() << std::endl;
    }

    void load_and_write(const std::string &efile){
        ofstream outfile(efile);
        if(!outfile){
           std::cout << "open file failed. " << efile <<std::endl;
            exit(0);
        }
        std::cout << "finish open file... " << efile <<std::endl;
        vertex_t u, v;
        value_t w;
        vertex_t max_id;
        vertex_t write_edge = 0;
        for (auto edge : edges) {
            u = edge.source;
            v = edge.destination;
            w = edge.weight;
            u = oldindex_newindex_map[u];
            v = oldindex_newindex_map[v];
            if(FLAGS_directed){
                if(u < v){
                    outfile << u << ' ' << v << ' ' << w << std::endl;
                    write_edge++;
                }
            }
            else{
                outfile << std::min(u, v) << ' ' << std::max(u, v) << ' ' << w << std::endl;
                write_edge++;
            }
        }
        std::cout << "write finied, nodes_num=" << re_index << " edges_num=" << write_edge << std::endl;
    }

    vertex_t getOriginId(const vertex_t newid){
        return vertex_reverse_map[newid];
    }

    vertex_t getNewId(const vertex_t oldid){
        return vertex_map[oldid];
    }

    void print_index(){
        for(auto e : oldindex_newindex_map){
            std::cout << e.first << " " << e.second << std::endl;
        }    
    }

    void start(){
        std::queue<vertex_t> que;
        bool *visited_nodes = new bool[nodes_num]();
        vertex_t source = getNewId(FLAGS_shortestpath_source);
        que.emplace(source);
        nodes[source].id = re_index++;  //重新分配编号
        visited_nodes[source] = 1;

        while(!que.empty()){
            vertex_t q = que.front();
            que.pop();
            for(auto edge : nodes[q].out_adj){
                if(visited_nodes[edge.first] == 0){
                    nodes[edge.first].id = re_index++;
                    que.emplace(edge.first);
                    visited_nodes[edge.first] = 1;
                }
            }
        }

        for(vertex_t i = 0; i < nodes_num; i++){
            // std::cout << getOriginId(i) << ": " << nodes[i].id << std::endl;
            if(nodes[i].id != -1){
                oldindex_newindex_map[getOriginId(i)] = nodes[i].id;
            }
            else{
                oldindex_newindex_map[getOriginId(i)] = re_index++;  //对于还为编号的点(即孤立的一块图)，直接编号，反正从源点不可达
            }
        }

        // print_index();

    }
    ~DealRoadDataSet(){
        delete []nodes;
    }

public:
    unordered_map<int, int> vertex_map; //原id: 新id
    unordered_map<int, int> vertex_reverse_map; // 新id: 原id
    unordered_map<int, int> oldindex_newindex_map; // 原id: 重新编号
    Node<vertex_t, value_t> *nodes;
    std::vector<Edge<vertex_t, value_t>> edges;
    vertex_t nodes_num=0;
    vertex_t re_index=0;
};

int main(int argc,char **argv) {
    // g++ deal_road_dataset.cc -lgflags -lglog && ./a.out -base_edge=../input/test_data_sssp_pattern_2.e -shortestpath_source=0 -output=../input/test_data_sssp_pattern_base.e -directed=true
    // g++ deal_road_dataset.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31.e -shortestpath_source=1 -output=/home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31_base.e -directed=true
    // g++ deal_road_dataset.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted.e -shortestpath_source=1 -output=/home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted_base.e -directed=true
    // g++ deal_road_dataset.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted.e -shortestpath_source=1 -output=/home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted_base.e -directed=true

    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true; //设置日志消息是否转到标准输出而不是日志文件, 即true：不写入文件，只在终端显示
    FLAGS_colorlogtostderr = true;  // Set log color

    timer_start(true);
    std::string base_edge = FLAGS_base_edge;
    std::string output = FLAGS_output;
    DealRoadDataSet<int, float> worker = DealRoadDataSet<int, float>();
    timer_next("load_graph");
    worker.load_weight(base_edge);
    timer_next("deal_data");
    worker.start();
    timer_next("write result");
    worker.load_and_write(output);
    timer_end(false);

    google::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}