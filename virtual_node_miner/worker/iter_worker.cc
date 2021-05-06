/*
    普通图上的traversal算法
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
#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <glog/logging.h>
#include "../app/shortestpath.h"
#include "../app/php.h"
#include "../graph/node.h"
#include "../graph/edge.h"
#include "./flags.h"
#include "../utils/bitset.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::ifstream;
using std::ofstream;
using grape::Bitset;

template<class vertex_t, class value_t>
class IterWorker{
public:
    IterWorker(){
        if(FLAGS_app == "sssp"){
            this->app_ = new ShortestpathIterateKernel<vertex_t, value_t>();
        }
        else if(FLAGS_app == "php"){
            this->app_ = new PhpIterateKernel<vertex_t, value_t>();
        }
        else{
            std::cout << "no this app..." << std::endl;
        }
    }

    void load(const std::string &efile){
        std::ifstream inFile(efile);
        if(!inFile){
            std::cout << "open file failed. " << efile << std::endl;
            exit(0);
        }
        std::cout << "finish read file... " << efile << std::endl;
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
            // std::cout << v << ": " << nodes[v].tag << ", " << u << ": " << nodes[u].tag << std::endl;
        }
        std::cout << "load finied, nodes_num=" << nodes_num << " edges_num=" << edges.size() << std::endl;
    }

    void init(){
        // init app.weight_sum
        this->app_->iterate_begin(this->nodes, this->nodes_num);
        value_t delta;
        value_t value;
        vector<std::pair<vertex_t, value_t>> data;
        for(vertex_t i = 0; i < nodes_num; i++){
            app_->init_c(vertex_reverse_map[i], nodes[i].recvDelta, data);
            this->nodes[i].oldDelta = app_->default_v();
            app_->init_v(i, nodes[i].value, data);
        }
    }

    void write_result(const std::string &output){
        std::cout << "out path: " << output << std::endl;
        ofstream fout(output);
        if(!fout){
            std::cout << "open file failed. " << output << std::endl;
            exit(0);
        }
        for(int i = 0; i < nodes_num; i++){
            Node<vertex_t, value_t>& node = nodes[i];
            fout << vertex_reverse_map[i] << ' ' << node.value << std::endl;
        }
        fout.close();
    }

    void print_result(){
        for(vertex_t i = 0; i < nodes_num; i++){
            Node<vertex_t, value_t>& node = nodes[i];
            std::cout << vertex_reverse_map[i] << ' ' << node.value << ' ' << node.recvDelta << std::endl;  
        }
    }

    void start(){
        init();
        // print_result();
        vertex_t step = 0;
        value_t delta_sum = 0;
        bool is_convergence;
        value_t itrative_threshold = FLAGS_delta_step_threshold;
        value_t threshold_change_cnt = 0;
        unsigned long long int node_send_cnt = 0;
        vertex_t begin = 0;
        vertex_t end = nodes_num;
        Bitset curr_modified_, next_modified_;
        curr_modified_.init(end-begin);
        next_modified_.init(end-begin);
        vertex_t active_node_num = 0;

        while(true){
            step++;
            delta_sum = 0;
            is_convergence = true;
            next_modified_.parallel_clear(4);

            // send
            for(vertex_t batch = begin; batch < end; batch+=64){
                vertex_t v = batch;
                uint64_t word = curr_modified_.get_word(batch-begin); // 获得数组中的一个元素，每个元素64位，每位表示一个元素
                while (word != 0) {
                    if (word & 1) {
                        Node<vertex_t, value_t>& node = nodes[v];
                        // if(node.oldDelta > itrative_threshold || node.oldDelta == app_->default_v()){
                        if(node.oldDelta < itrative_threshold){  // 注意这个是针对sssp写的
                            for(auto edge : node.out_adj){ // i -> adj
                                value_t& recvDelta = nodes[edge.first].recvDelta; // adj's recvDelta
                                value_t sendDelta; // i's 
                                // app_->g_func(node.oldDelta, edge.second, sendDelta);
                                app_->g_func(node.id, node.oldDelta, node.value, edge, sendDelta);
                                app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                                node_send_cnt++;
                            }
                            node.oldDelta = app_->default_v(); // delta发完需要清空 
                        }
                    }
                    ++v;
                    word = word >> 1;
                }
            }

            // receive
            for(vertex_t i = 0; i < nodes_num; i++){
                Node<vertex_t, value_t>& node = nodes[i];
                value_t old_value = node.value;
                app_->accumulate(node.value, node.recvDelta); // delat -> value
                // delta_sum += std::fabs(old_value - node.value); // 负的累加在一起会抵消，导致提前小于阈值
                // app_->accumulate(node.oldDelta, node.recvDelta); // updata delat
                // if(old_value != node.value){ // sssp类收敛
                if(std::fabs(old_value - node.value) > FLAGS_convergence_threshold){  // pr/php类收敛
                    is_convergence = false;
                    // next_modified_.insert(i);
                    app_->accumulate(node.oldDelta, node.recvDelta); // updata delat
                }
                if(node.oldDelta != app_->default_v()){ // 需要单独判断这个点是否时活跃点
                    next_modified_.set_bit(i - begin);
                }
                // node.oldDelta = node.recvDelta;
                node.recvDelta = app_->default_v();
            }
            //debug
            // {
            //     for(vertex_t i = 0; i < nodes_num; i++){
            //         Node<vertex_t, value_t>& node = nodes[i];
            //         std::cout << "step=" << step << " i=" << i << " value=" <<  node.value << " olddelta=" << node.oldDelta <<  std::endl;
            //     }
            // }
            next_modified_.swap(curr_modified_);
            active_node_num = curr_modified_.parallel_count(4);
            if(step % 100 == 0)
                LOG(INFO) << "step=" << step << " curr_modified_=" << active_node_num;

            // 更新阈值
            if(is_convergence && active_node_num != 0 && step < FLAGS_max_iterater_num){
                itrative_threshold += 15 + step*0.1;
                threshold_change_cnt++;
                std::cout << "local convergence-----itrative_threshold=" << itrative_threshold <<  " threshold_change_cnt=" << threshold_change_cnt << std::endl;
                continue;
            }

            if(is_convergence || step > FLAGS_max_iterater_num){
                break;
            }
        }
        LOG(INFO) << "app convergence step=" << step << " threshold_change_cnt=" << threshold_change_cnt << " g_cnt=" << app_->g_cnt << " f_cnt=" << app_->f_cnt;
        LOG(INFO) << "node_send_cnt=" << node_send_cnt;
        // 统计结果写入文件：
        ofstream fout(FLAGS_result_analyse, std::ios::app);
        fout << "IterWorker_step:" << step << "\n";
        fout << "IterWorker_threshold_change_cnt:" << threshold_change_cnt << "\n";
        fout << "IterWorker_g_cnt:" << app_->g_cnt << "\n";
        fout << "IterWorker_f_cnt:" << app_->f_cnt << "\n";
        fout << "IterWorker_node_send_cnt:" << node_send_cnt << "\n";
        fout.close();
    }

    ~IterWorker(){
        delete []nodes;
        delete app_;
    }

public:
    unordered_map<vertex_t, vertex_t> vertex_map; //原id: 新id
    unordered_map<vertex_t, vertex_t> vertex_reverse_map; // 新id: 原id
    Node<vertex_t, value_t> *nodes;
    vertex_t nodes_num=0;
    IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>>> *app_; 
};

int main(int argc,char **argv) {
    // g++ iter_worker.cc -lgflags -lglog && ./a.out -base_edge=../input/test_data_sssp_pattern_base.e -php_source=0 -output=../out/php_result -delta_step_threshold=3
    // g++ traversal_worker.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31_base.e -shortestpath_source=0 -output=../out/sssp_result -delta_step_threshold=3
    // g++ traversal_worker.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted_base.e -shortestpath_source=0 -output=../out/sssp_result -delta_step_threshold=3
    // g++ traversal_worker.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted_base.e -shortestpath_source=0 -output=../out/sssp_result -delta_step_threshold=3 -result_analyse=./out/sssp__result_analyse
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true; //设置日志消息是否转到标准输出而不是日志文件, 即true：不写入文件，只在终端显示
    FLAGS_colorlogtostderr = true;  // Set log color

    timer_start(true);
    std::string base_edge = FLAGS_base_edge;
    std::string output = FLAGS_output;
    IterWorker<int, float> worker = IterWorker<int, float>();
    timer_next("load_graph");
    worker.load(base_edge);
    timer_next("compute");
    worker.start();
    timer_next("write_result");
    worker.write_result(output);
    timer_end(true, "IterWorker", FLAGS_result_analyse);

    google::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}