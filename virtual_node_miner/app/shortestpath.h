#ifndef APP_SHORTESTPATH_ITERATEKERNEL_H_
#define APP_SHORTESTPATH_ITERATEKERNEL_H_

#include "IterateKernel.h"
#include "../worker/flags.h"

// DECLARE_string(result_dir);
// DEFINE_int64(num_nodes);
// DECLARE_double(portion);

template<class vertex_t, class value_t>
class ShortestpathIterateKernel : public IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>> > {
public: 
    using adj_list_t = typename std::vector<std::pair<vertex_t, value_t>>;
    value_t imax;
    value_t zero;

    ShortestpathIterateKernel (): zero(0){
        imax = std::numeric_limits<value_t>::max();
    }

    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data){
        if(k == FLAGS_shortestpath_source){
            delta = 0;
        }else{
            delta = imax;
        }
    }
    // 用于sssp类超点内部初始化
    void init_c(const vertex_t& k, value_t& delta, adj_list_t& data, const vertex_t& source){
        if(k == source){
            delta = 0;
        }else{
            delta = imax;
        }
    }
    void init_v(const vertex_t& k, value_t& v, adj_list_t& data){
        v = imax;  
    }
        
    void accumulate(value_t& a, const value_t& b){
        a = std::min(a, b); 
        this->g_cnt++;
    }

    void priority(value_t& pri, const value_t& value, const value_t& delta){
        pri = value - std::min(value, delta); 
    }

    // void g_func(const value_t& delta, const value_t& value, value_t& data){
    void g_func(const vertex_t &k, const value_t& delta, const value_t& value, const std::pair<vertex_t, value_t>& data, value_t& output){
        output = delta + data.second;
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
        return imax;
    }
    
    const value_t& min_delta() const {
         return zero;
    }
    
};

#endif  // APP_SHORTESTPATH_ITERATEKERNEL_H_