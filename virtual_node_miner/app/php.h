#ifndef APP_PHP_H_
#define APP_PHP_H_

#include "IterateKernel.h"
#include "../worker/flags.h"
#include "../graph/node.h"
#include <vector>

template<class vertex_t, class value_t>
class PhpIterateKernel : public IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>> > {
public: 
    using adj_list_t = typename std::vector<std::pair<vertex_t, value_t>>;
    value_t zero;
    std::vector<vertex_t> weight_sum;

    PhpIterateKernel (): zero(0){
    }

    void iterate_begin(Node<vertex_t, value_t> *nodes, vertex_t node_num) override{
        weight_sum.resize(node_num);
        auto source_id = FLAGS_php_source;

        for (auto i = 0; i < node_num; i++) {
            Node<vertex_t, value_t>& node = nodes[i];
            for (auto& e : node.out_adj) {
                auto dst = e.first;
                auto weight = e.second;
                if (dst != source_id) {
                    weight_sum[node.id] += weight;
                }
            }
        }
        // 修改权重
        for (auto i = 0; i < node_num; i++) {
            Node<vertex_t, value_t>& node = nodes[i];
            for (auto& e : node.out_adj) {
                e.second = e.second / weight_sum[node.id];
            }
        }
        // for(auto i = 0; i < node_num; i++){
        //     std::cout << "id=" << i << " " << weight_sum[i] << std::endl;
        // }
        // std::cout << "==============" << std::endl;
    }

    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data){
        if(k == FLAGS_php_source){
            delta = 1;
        }else{
            delta = 0;
        }
    }
    // 超点内部初始化
    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data, const vertex_t& source){
        if(k == source){
            delta = 1;
        }else{
            delta = 0;
        }
    }
    void init_v(const vertex_t& k, value_t& v, adj_list_t& data){
        v = 0.0f;
    }
        
    void accumulate(value_t& a, const value_t& b){
        a += b; 
        this->g_cnt++;
    }

    void priority(value_t& pri, const value_t& value, const value_t& delta){
        pri = delta; 
    }

    // void g_func(const vertex_t &k, const value_t& delta, const value_t& value, value_t& data, value_t& output){
    void g_func(const vertex_t &k, const value_t& delta, const value_t& value, const std::pair<vertex_t, value_t>& data, value_t& output){
        // output = data.second * delta * 0.80 / weight_sum[k];
        output = data.second * delta * 0.80;
        this->f_cnt++;
    }

    void g_func(const vertex_t &k, const value_t& delta,const value_t& value, const adj_list_t& data, adj_list_t* output){
        // for(std::vector<std::pair<vertex_t, value_t>>::const_iterator it=data.begin(); it!=data.end(); it++){
        //     std::pair<vertex_t, value_t> target = *it;
        //     value_t outv = delta + target.weight;
        //     output->push_back(make_pair(target.end, outv));
        // }
    }

    const value_t& default_v() const {
        return zero;
    }
    
    const value_t& min_delta() const {
         return zero;
    }
    
};

#endif  // APP_PHP_H_