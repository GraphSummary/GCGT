#ifndef GRAPH_EDGE_H_
#define GRAPH_EDGE_H_
template<class vertex_t, class value_t>
class Edge
{
public:
    vertex_t source;
    vertex_t destination;
    value_t weight;
    Edge(vertex_t s, vertex_t d, value_t w){
        source = s;
        destination = d;
        weight = w;
    }
};
#endif  // GRAPH_EDGE_H_