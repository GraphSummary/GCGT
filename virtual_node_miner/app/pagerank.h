#ifndef APP_PAGERANK_H_
#define APP_PAGERANK_H_

#include "IterateKernel.h"
#include "../worker/flags.h"
#include "../graph/node.h"
#include <vector>

template<class vertex_t, class value_t>
class PagerankIterateKernel : public IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>> > {
public: 
    using adj_list_t = typename std::vector<std::pair<vertex_t, value_t>>;
    value_t zero=0;

    PagerankIterateKernel (): zero(0){
    }

    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data){
        delta = 0.2;
    }
    // 超点内部初始化
    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data, const vertex_t& source){
        delta = 0.2;
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
    void g_func(const vertex_t &k, const value_t& delta, const value_t& value, const adj_list_t& adj_edges, const std::pair<vertex_t, value_t>& data, value_t& output){
        output = delta * 0.80 / (int) adj_edges.size();
        this->f_cnt++;
    }

    // 超点索引计算
    value_t g_refunc(const vertex_t& k, value_t& delta, adj_list_t& data){

        return data;
    }

    void g_func(const vertex_t &k, const value_t& delta, const value_t& value, const adj_list_t& data, adj_list_t* output){
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

#endif  // APP_PAGERANK_H_