/*
    summary图上的traversal算法
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include <math.h>
#include <queue>
#include <cassert>
#include <algorithm>
#include <memory.h>
#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <glog/logging.h>
#include "../utils/timer.h"
#include "../app/shortestpath.cc"
#include "../graph/node.h"
#include "../graph/edge.h"
#include "./flags.h"
#include "../tools/find_pattern_sssp.h"


template<class vertex_t, class value_t>
class TraversalWorkerSum: public FindPatternForSSSP<vertex_t, value_t>
{
public:
    TraversalWorkerSum(){
        app_ = new ShortestpathIterateKernel<vertex_t, value_t>();
    }

    void print_result(){
        for(vertex_t i = 0; i < this->nodes_num; i++){
            Node<vertex_t, value_t>& node = this->nodes[i];
            std::cout << this->vertex_reverse_map[i] << ' ' << node.value << ' ' << node.recvDelta << std::endl;  
        }
    }

    void init(){
        value_t delta;
        value_t value;
        vector<std::pair<vertex_t, value_t>> data;
        for(vertex_t i = 0; i < this->nodes_num; i++){
            app_->init_c(this->vertex_reverse_map[i], this->nodes[i].recvDelta, data);
            this->nodes[i].oldDelta = app_->default_v();
            app_->init_v(i, this->nodes[i].value, data);
        }
    }

    void write_result(const std::string &output){
        std::cout << "out path: " << output << std::endl;
        ofstream fout(output);
        if(!fout){
            std::cout << "open file failed. " << output << std::endl;
            exit(0);
        }
        for(int i = 0; i < this->nodes_num; i++){
            Node<vertex_t, value_t>& node = this->nodes[i];
            fout << this->vertex_reverse_map[i] << ' ' << node.value << std::endl;
        }
        fout.close();
    }

    void start(){
        std::cout << "start..." << std::endl;
        init();
        // print_result();

        vertex_t step = 0;
        vertex_t contain_cnt = 0;
        vertex_t node_send_cnt = 0;
        vertex_t super_send_cnt = 0;
        value_t delta_sum = 0;
        while(true){
            delta_sum = 0;
            // receive
            for(vertex_t i = 0; i < this->nodes_num; i++){
                Node<vertex_t, value_t>& node = this->nodes[i];
                value_t old_value = node.value;
                app_->accumulate(node.value, node.recvDelta); // delat -> value
                delta_sum += std::fabs(old_value - node.value); // 负的累加在一起会抵消，导致提前小于阈值
                // app_->accumulate(node.oldDelta, node.recvDelta); // updata delat
                node.oldDelta = node.recvDelta;
                node.recvDelta = app_->default_v();
            }
            // send
            for(vertex_t i = 0; i < this->nodes_num; i++){
                Node<vertex_t, value_t>& node = this->nodes[i];
                if(this->Fc[i] != this->Fc_default_value){ // 测试：仅仅用来统计内点次数
                    contain_cnt++;
                }
                if(node.oldDelta == app_->default_v()){
                    continue;
                }
                if(this->Fc[i] == i){
                    // 超点send
                    ExpandData<vertex_t, value_t>& supernode = this->expand_data[this->Fc_map[i]];
                    Node<vertex_t, value_t>& node = this->nodes[supernode.id];
                    if(node.oldDelta == app_->default_v()){
                        continue;
                    }
                    for(auto& edge : supernode.edges){ // i -> adj
                        value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                        value_t sendDelta; // i's 
                        app_->g_func(node.oldDelta, edge.second, sendDelta);
                        app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                        super_send_cnt++;
                    }
                }
                else if(this->Fc[i] == this->Fc_default_value){
                    // 普通点send
                    for(auto& edge : node.out_adj){ // i -> adj
                        value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                        value_t sendDelta; // i's 
                        app_->g_func(node.oldDelta, edge.second, sendDelta);
                        app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                        node_send_cnt++;
                    }
                }
                else{
                    continue;
                }
                node.oldDelta = app_->default_v(); // delta发完需要清空
            }
            // for(vertex_t i = 0; i < this->supernodes_num; i++){
            //     ExpandData<vertex_t, value_t>& supernode = this->expand_data[i];
            //     Node<vertex_t, value_t>& node = this->nodes[supernode.id];
            //     if(node.oldDelta == app_->default_v()){
            //         continue;
            //     }
            //     for(auto& edge : supernode.edges){ // i -> adj
            //         value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
            //         value_t sendDelta; // i's 
            //         app_->g_func(node.oldDelta, edge.second, sendDelta);
            //         app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
            //         super_send_cnt++;
            //     }
            //     node.oldDelta = app_->default_v();
            // }
            // print_result();
            step++;
            LOG(INFO) << "step=" << step << " delta_sum=" << delta_sum;
            if(delta_sum < FLAGS_convergence_threshold || step > 1000){
                break;
            }
        }        
        LOG(INFO) << "app convergence step=" << step << " delta_sum=" << delta_sum << " g_cnt=" << app_->g_cnt <<
        " f_cnt=" << app_->f_cnt;
        LOG(INFO) << "contain_cnt=" << contain_cnt;
        LOG(INFO) << "node_send_cnt=" << node_send_cnt << " super_send_cnt=" << super_send_cnt << " +=" << (node_send_cnt+super_send_cnt);
        printf("node_send_cnt=%d= super_send_cnt=%d\n", node_send_cnt, super_send_cnt);
    }

protected:
    ShortestpathIterateKernel<vertex_t, value_t> *app_;

};


int main(int argc,char **argv) {
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=../input/test_data_sssp_pattern_base.e -shortestpath_source=0 -output=../out/sssp_result_sum
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31_base.e -shortestpath_source=0 -output=../out/sssp_result_sum
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted_base.e -shortestpath_source=0 -output=../out/sssp_result_sum
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted_base.e -shortestpath_source=0 -output=../out/sssp_result_sum

    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true; //设置日志消息是否转到标准输出而不是日志文件, 即true：不写入文件，只在终端显示
    FLAGS_colorlogtostderr = true;  // Set log color

    timer_start(true);
    std::string base_edge = FLAGS_base_edge;
    std::string output = FLAGS_output;
    TraversalWorkerSum<int, float> worker = TraversalWorkerSum<int, float>();
    timer_next("load_graph");
    worker.load(base_edge);
    timer_next("find_pattern");
    worker.start_find();
    timer_next("write_pattern");
    worker.write_supernode("../out/a_pattern");
    timer_next("compute");
    worker.start();
    timer_next("write_result");
    worker.write_result(output);
    timer_end(true, "TraversalWorkerSum"+base_edge, "../out/traversal_run_time");

    google::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}