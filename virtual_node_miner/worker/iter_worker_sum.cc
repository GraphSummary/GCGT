/*
    summary图上的iterative算法
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
#include "../app/shortestpath.h"
#include "../app/pagerank.h"
#include "../app/php.h"
#include "../graph/node.h"
#include "../graph/edge.h"
#include "./flags.h"
#include "../tools/find_pattern.h"
#include "../utils/bitset.h"
#include "../utils/util.h"
#include <random>

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::ifstream;
using std::ofstream;
using grape::Bitset;

template<class vertex_t, class value_t>
class IterWorkerSum: public FindPattern<vertex_t, value_t>
{
public:
    IterWorkerSum(){
        if(FLAGS_app == "php"){
            this->app_ = new PhpIterateKernel<vertex_t, value_t>();
        }
        else if(FLAGS_app == "pagerank"){
            this->app_ = new PagerankIterateKernel<vertex_t, value_t>();
        }
        else{
            LOG(INFO) << "no this app..." << std::endl;
        }
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
            this->app_->init_c(this->vertex_reverse_map[i], this->nodes[i].recvDelta, data);
            this->nodes[i].oldDelta = this->app_->default_v();
            this->app_->init_v(i, this->nodes[i].value, data);
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

  /**
   * 通过采样确定阈值来筛选数据
   * sample_size: 采样大小
   * return 阈值
   */
   value_t Scheduled(int sample_size) {
     vertex_t all_size = this->nodes_num;
     if (all_size <= sample_size) {
       return this->app_->default_v();
     } else {
       // 去重
       std::unordered_set<int> id_set;
       // random number generator
       std::mt19937 gen(time(0));
       std::uniform_int_distribution<> dis(0, all_size - 1);  // 给定范围 // 构造符合要求的随机数生成器
       // 采样
       vector<value_t> priority;
       priority.resize(sample_size);
       for (int i = 0; i < sample_size; i++) {
         int rand_pos = dis(gen);
         while (id_set.find(rand_pos) != id_set.end()) {
           rand_pos = dis(gen);
         }
         id_set.insert(rand_pos);
         this->app_->priority(priority[i], this->nodes[rand_pos].value, this->nodes[rand_pos].oldDelta);
       }
       // get the cut index, everything larger than the cut will be scheduled
        sort(priority.begin(), priority.end());
        int cut_index = sample_size * FLAGS_portion;  // 选择阈值位置
        value_t threshold = priority[cut_index];  // 获得阈值, 保证一定是 >=0
        return fabs(threshold);
     }
   }

    void start(){
        std::cout << "start..." << std::endl;
        init();
        // print_result();

        vertex_t step = 0;
        // vertex_t contain_cnt = 0;
        unsigned long long int node_send_cnt = 0;
        unsigned long long int super_send_cnt = 0;
        value_t delta_sum = 0;
        value_t itrative_threshold = FLAGS_delta_step_threshold;
        value_t threshold_change_cnt = 0;
        bool is_convergence;
        vertex_t begin = 0;
        vertex_t end = this->nodes_num;
        Bitset curr_modified_, next_modified_;
        curr_modified_.init(end-begin);
        next_modified_.init(end-begin);
        vertex_t active_node_num = 0;
        last_values = new value_t[this->nodes_num];

        /* init last_values */
        for(vertex_t i = 0; i < this->nodes_num; i++){
            last_values[i] = this->nodes[i].value;
        }

        // // 如果源点在超点内部且不是入口，需要将该超点解压
        // vertex_t source_new_id = this->getNewId(FLAGS_php_source);
        // if(this->Fc[source_new_id] != this->Fc_default_value && this->Fc[source_new_id] != source_new_id){
        //     this->delete_supernode(this->Fc[source_new_id]);
        //     LOG(INFO) << "source in supernode...";
        // }
        // LOG(INFO) << this->nodes[0].value << " " << this->nodes[0].oldDelta << " " << this->nodes[0].recvDelta;
        // LOG(INFO) << this->nodes[1].value << " " << this->nodes[1].oldDelta << " " << this->nodes[1].recvDelta;

        while(true){
            step++;
            delta_sum = 0;
            is_convergence = true;
            next_modified_.parallel_clear(4);

            // 采样
            value_t pri = Scheduled(2000);
            LOG(INFO) << pri;

            // send
            for(vertex_t batch = begin; batch < end; batch+=64){
                vertex_t i = batch;
                uint64_t word = curr_modified_.get_word(batch-begin); // 获得数组中的一个元素，每个元素64位，每位表示一个元素
                while (word != 0) {
                    if (word & 1) {
                        Node<vertex_t, value_t>& node = this->nodes[i];
                        // if(node.oldDelta < itrative_threshold && node.oldDelta != this->app_->default_v()){
                        // if(node.oldDelta >= pri && node.oldDelta < itrative_threshold && fabs(node.oldDelta - this->app_->default_v()) * this->nodes_num > FLAGS_convergence_threshold){
                        if(node.oldDelta >= pri && node.oldDelta < itrative_threshold && fabs(node.oldDelta - this->app_->default_v()) * this->nodes_num > FLAGS_convergence_threshold){
                            if(this->Fc[i].size() == 1 && this->Fc[i][0] == i){
                                // 超点send
                                ExpandData<vertex_t, value_t>& supernode = this->expand_data[this->Fc_map[i]];
                                Node<vertex_t, value_t>& node = this->nodes[supernode.id];
                                // for(auto& edge : supernode.edges){ // i -> adj
                                /* send to delat by bound_edges */
                                for(auto& edge : supernode.bound_edges){ // i -> adj
                                    value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                                    value_t sendDelta; // i's 
                                    // this->app_->g_func(node.oldDelta, edge.second, sendDelta);
                                    // this->app_->g_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->g_index_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                                    super_send_cnt++;
                                }
                                /* send to value by inner_edges */
                                for(auto& edge : supernode.inner_edges){ // i -> adj
                                    // value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                                    value_t& value = this->nodes[edge.first].value; // adj's recvDelta
                                    value_t sendDelta; // i's 
                                    // this->app_->g_func(node.oldDelta, edge.second, sendDelta);
                                    // this->app_->g_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->g_index_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->accumulate(value, sendDelta); // sendDelta -> recvDelta
                                    super_send_cnt++;
                                }
                                /* send to oldDelta by inner_delta */
                                for(auto& edge : supernode.inner_delta){ // i -> adj
                                    // value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                                    value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                                    value_t sendDelta; // i's 
                                    // this->app_->g_func(node.oldDelta, edge.second, sendDelta);
                                    // this->app_->g_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->g_index_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                                    super_send_cnt++;
                                }
                                // value_t& recvDelta = supernode.data;
                                // this->app_->accumulate(recvDelta, node.oldDelta); // 暂存oldDelta
                                node.oldDelta = this->app_->default_v(); // delta发完需要清空
                            }
                            else if(!(this->Fc[i].size() == 1 && this->Fc[i][0] == i)){
                                // 普通点send
                                for(auto& edge : node.out_adj){ // i -> adj
                                    value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
                                    value_t sendDelta; // i's 
                                    // this->app_->g_func(node.oldDelta, edge.second, sendDelta);
                                    this->app_->g_func(i, node.oldDelta, node.value, node.out_adj, edge, sendDelta);
                                    this->app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
                                    node_send_cnt++;
                                }
                                node.oldDelta = this->app_->default_v(); // delta发完需要清空
                            }
                        }
                    }
                    ++i;
                    word = word >> 1;
                }
            }

            // receive
            for(vertex_t i = 0; i < this->nodes_num; i++){
                Node<vertex_t, value_t>& node = this->nodes[i];
                if(!(this->Fc[i].size() == 1 && this->Fc[i][0] == i)){ // 超点入口点通过inner_edges将delta作用给自己
                    this->app_->accumulate(node.value, node.recvDelta); // delat -> value
                }
                this->app_->accumulate(node.oldDelta, node.recvDelta); // updata delat
                // if(node.oldDelta != this->app_->default_v()){ // 需要单独判断这个点是否是活跃点, 同时内部点不需要发消息，默认为非活跃
                if(fabs(node.oldDelta - this->app_->default_v()) * this->nodes_num > FLAGS_convergence_threshold){ // 需要单独判断这个点是否是活跃点, 同时内部点不需要发消息，默认为非活跃
                    next_modified_.set_bit(i - begin);
                    // std::cout << i << ": oldDelat" << node.oldDelta << " value=" << node.value << " old-def=" << fabs(node.oldDelta - this->app_->default_v()) << std::endl;
                }
                node.recvDelta = this->app_->default_v();
            }

            is_convergence = termCheck();
            //debug
            // {
            //     for(vertex_t i = 0; i < this->nodes_num; i++){
            //         Node<vertex_t, value_t>& node = this->nodes[i];
            //         std::cout << "step=" << step << " i=" << i << " value=" <<  node.value << " olddelta=" << node.oldDelta <<  std::endl;
            //     }
            // }
            next_modified_.swap(curr_modified_);
            active_node_num = curr_modified_.parallel_count(4);
            if(step % 100 == 0)
                LOG(INFO) << "step=" << step << " curr_modified_=" << active_node_num;

            // 更新阈值
            // if(is_convergence && active_node_num != 0 && step < FLAGS_max_iterater_num){
            //     itrative_threshold += 15 + step*0.1;
            //     threshold_change_cnt++;
            //     std::cout << "local convergence-----itrative_threshold=" << itrative_threshold <<  " threshold_change_cnt=" << threshold_change_cnt << " active_num=" << active_node_num << std::endl;
            //     continue;
            // }

            // if((is_convergence && active_node_num == 0) || step > FLAGS_max_iterater_num){
            if(is_convergence|| step > FLAGS_max_iterater_num){
                break;
            }
        } 
        LOG(INFO) << "app step=" << step << " threshold_change_cnt=" << threshold_change_cnt << " g_cnt=" << this->app_->g_cnt << " f_cnt=" << this->app_->f_cnt;
        // LOG(INFO) << "correct start...";
        // correct value by get oldDelta
        // while(true){
        //     step++;
        //     delta_sum = 0;
        //     is_convergence = true;
        //     next_modified_.parallel_clear(4);
        //     // send
        //     for(vertex_t batch = begin; batch < end; batch+=64){
        //         vertex_t v = batch;
        //         uint64_t word = curr_modified_.get_word(batch-begin); // 获得数组中的一个元素，每个元素64位，每位表示一个元素
        //         while (word != 0) {
        //             if (word & 1) {
        //                 Node<vertex_t, value_t>& node = this->nodes[v];
        //                 if(node.oldDelta < itrative_threshold){  // 注意这个是针对sssp写的
        //                     for(auto edge : node.out_adj){ // i -> adj
        //                         value_t& recvDelta = this->nodes[edge.first].recvDelta; // adj's recvDelta
        //                         value_t sendDelta; // i's 
        //                         // app_->g_func(node.oldDelta, edge.second, sendDelta);
        //                         this->app_->g_func(node.id, node.oldDelta, node.value, edge, sendDelta);
        //                         this->app_->accumulate(recvDelta, sendDelta); // sendDelta -> recvDelta
        //                         node_send_cnt++;
        //                     }
        //                     node.oldDelta = this->app_->default_v(); // delta发完需要清空 
        //                 }
        //             }
        //             ++v;
        //             word = word >> 1;
        //         }
        //     }
        //     // receive
        //     for(vertex_t i = 0; i < this->nodes_num; i++){
        //         Node<vertex_t, value_t>& node = this->nodes[i];
        //         value_t old_value = node.value;
        //         this->app_->accumulate(node.value, node.recvDelta); // delat -> value
        //         // delta_sum += std::fabs(old_value - node.value); // 负的累加在一起会抵消，导致提前小于阈值
        //         // if(old_value != node.value){ // sssp类收敛
        //         if(std::fabs(old_value - node.value) > FLAGS_convergence_threshold){  // pr/php类收敛
        //             is_convergence = false;
        //             // next_modified_.insert(i);
        //             this->app_->accumulate(node.oldDelta, node.recvDelta); // updata delat
        //         }
        //         if(node.oldDelta != this->app_->default_v()){ // 需要单独判断这个点是否时活跃点
        //             next_modified_.set_bit(i - begin);
        //         }
        //         // node.oldDelta = node.recvDelta;
        //         node.recvDelta = this->app_->default_v();
        //     }
        //     next_modified_.swap(curr_modified_);
        //     active_node_num = curr_modified_.parallel_count(4);
        //     if(step % 100 == 0)
        //         LOG(INFO) << "step=" << step << " curr_modified_=" << active_node_num;
        //     // 更新阈值
        //     if(is_convergence && active_node_num != 0 && step < FLAGS_max_iterater_num){
        //         itrative_threshold += 15 + step*0.1;
        //         threshold_change_cnt++;
        //         std::cout << "local convergence-----itrative_threshold=" << itrative_threshold <<  " threshold_change_cnt=" << threshold_change_cnt << std::endl;
        //         continue;
        //     }
        //     if(is_convergence || step > FLAGS_max_iterater_num){
        //         break;
        //     }
        // }

        // LOG(INFO) << "app convergence step=" << step << " threshold_change_cnt=" << threshold_change_cnt << " g_cnt=" << this->app_->g_cnt << " f_cnt=" << this->app_->f_cnt;
        // LOG(INFO) << "node_send_cnt=" << node_send_cnt << " super_send_cnt=" << super_send_cnt << " +=" << (node_send_cnt+super_send_cnt);

        // 统计结果写入文件：
        ofstream fout(FLAGS_result_analyse, std::ios::app);
        fout << "IterWorkerSum_step:" << step << "\n";
        fout << "IterWorkerSum_threshold_change_cnt:" << threshold_change_cnt << "\n";
        fout << "IterWorkerSum_g_cnt:" << this->app_->g_cnt << "\n";
        fout << "IterWorkerSum_f_cnt:" << this->app_->f_cnt << "\n";
        fout << "IterWorkerSum_node_send_cnt:" << node_send_cnt << "\n";
        fout << "IterWorkerSum_super_send_cnt:" << super_send_cnt << "\n";
        fout.close();
    }

    ~IterWorkerSum(){
        delete[] last_values;
    }

private:

    bool termCheck() {
        terminate_checking_time_ -= grape::GetCurrentTime();
        float diff_sum = 0;

        for (vertex_t u = 0; u < this->nodes_num; u++) {
            diff_sum += fabs(last_values[u] - this->nodes[u].value);
            last_values[u] = this->nodes[u].value;
        }

        // LOG(INFO) << "Diff: " << diff_sum;

        terminate_checking_time_ += grape::GetCurrentTime();

        return diff_sum < FLAGS_convergence_threshold;
    }

    value_t* last_values;
    float terminate_checking_time_ = 0;
};


int main(int argc,char **argv) {
    // g++ iter_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=../input/test_data_sssp_pattern_base.e -php_source=0 -output=../out/sssp_result_sum -delta_step_threshold=3 -result_analyse=../out/sssp_result_analyse
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31_base.e -shortestpath_source=0 -output=../out/sssp_result_sum  -delta_step_threshold=3  -result_analyse=../out/sssp_result_analyse
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted_base.e -shortestpath_source=0 -output=../out/sssp_result_sum -delta_step_threshold=3  -result_analyse=../out/sssp_result_analyse
    // g++ traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=/home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted_0_base.e -shortestpath_source=0 -output=../out/sssp_result_sum -delta_step_threshold=3 -result_analyse=../out/sssp__result_analyse

    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true; //设置日志消息是否转到标准输出而不是日志文件, 即true：不写入文件，只在终端显示
    FLAGS_colorlogtostderr = true;  // Set log color

    timer_start(true);
    std::string base_edge = FLAGS_base_edge;
    std::string output = FLAGS_output;
    IterWorkerSum<int, float> worker = IterWorkerSum<int, float>();
    timer_next("load_graph");
    worker.load(base_edge);
    timer_next("find_pattern");
    worker.start_find(FLAGS_result_analyse);
    // return 0; // 测试
    // timer_next("write_pattern");
    // worker.write_supernode("./out/a_pattern");
    timer_next("compute");
    worker.start();
    timer_next("write_result");
    worker.write_result(output);
    timer_end(true, "IterWorkerSum", FLAGS_result_analyse);

    google::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}