#include "IterateKernel.cc"


// DECLARE_string(result_dir);
// DEFINE_int64(num_nodes);
// DECLARE_double(portion);
DEFINE_int64(shortestpath_source, 0, "source");

template<class vertex_t, class value_t>
struct ShortestpathIterateKernel : public IterateKernel<vertex_t, value_t, std::vector<std::pair<vertex_t, value_t>> > {
    value_t imax;
    value_t zero;
    unsigned long long int g_cnt = 0;
    unsigned long long int f_cnt = 0;

    ShortestpathIterateKernel (): zero(0){
        imax = std::numeric_limits<value_t>::max();
    }

    void init_c(const vertex_t& k, value_t& delta, std::vector<std::pair<vertex_t, value_t>>& data){
        if(k == FLAGS_shortestpath_source){
            delta = 0;
        }else{
            delta = imax;
        }
    }
    void init_v(const vertex_t& k,value_t& v,std::vector<std::pair<vertex_t, value_t>>& data){
        v = imax;  
    }
        
    void accumulate(value_t& a, const value_t& b){
        a = std::min(a, b); 
        g_cnt++;
    }

    void priority(value_t& pri, const value_t& value, const value_t& delta){
        pri = value - std::min(value, delta); 
    }

    void g_func(const value_t& delta, const value_t& value, value_t& data){
        data = delta + value;
        f_cnt++;
    }

    void g_func(const vertex_t &k, const value_t& delta,const value_t& value, const std::vector<std::pair<vertex_t, value_t>>& data, std::vector<std::pair<vertex_t, value_t> >* output){
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


