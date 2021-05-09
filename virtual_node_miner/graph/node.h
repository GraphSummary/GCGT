#ifndef GRAPH_NODE_H_
#define GRAPH_NODE_H_

template<class vertex_t, class value_t>
class Node
{
public:
    vertex_t id;
    vertex_t tag;
    value_t value = 0;
    value_t oldDelta= 0;
    value_t recvDelta = 0;
    std::vector<std::pair<vertex_t, value_t>> in_adj;
    std::vector<std::pair<vertex_t, value_t>> out_adj;
};

/*
    图中的特殊结构，即超点
*/
template<class vertex_t, class value_t>
class ExpandData
{
public:
    vertex_t id;
    value_t data=0; // 用于sum类暂存数据
    std::vector<vertex_t> ids; // 属于这个结构的id set
    // std::vector<vertex_t> inner_ids; // 属于这个结构的内部id set(指不与外界点有连边的点)
    // std::vector<vertex_t> bound_ids; // 属于这个结构的边界id set(指与外界点有连边的点)
    // std::vector<std::pair<vertex_t, value_t>> edges; // s: (d, weight) 
    std::vector<std::pair<vertex_t, value_t>> inner_edges; // s: (d, weight). d is an inner node.
    std::vector<std::pair<vertex_t, value_t>> bound_edges; // s: (d, weight). d is a bound node.
    std::vector<std::pair<vertex_t, value_t>> inner_delta; // s: (d, oldDelta). d is a inner node.
    void print_edge(){
        std::cout << "id=" << id << " size=" << ids.size() << std::endl;
        std::cout << "inner_edges:" << std::endl;
        for(auto edge : inner_edges){
            std::cout << edge.first << ": " << edge.second << std::endl;
        }
        std::cout << "bound_edges:" << std::endl;
        for(auto edge : bound_edges){
            std::cout << edge.first << ": " << edge.second << std::endl;
        }
    }
    void swap(ExpandData & x){
        std::swap(id, x.id);
        ids.swap(x.ids);
        inner_edges.swap(x.inner_edges);
        bound_edges.swap(x.bound_edges);
    }
    void clear(){
        ids.clear();
        bound_edges.clear();
        inner_edges.clear();
    }
};

#endif  // GRAPH_NODE_H_