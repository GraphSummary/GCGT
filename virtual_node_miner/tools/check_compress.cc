/*
    检查压缩结果是否正确
    加载压缩图和未压缩图进行比较
*/
/*
 * 进行边的增删
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include <string>
#include <stdlib.h>
#include <cmath>
#include <cassert>
#include <algorithm>
#include "../utils/timer.h"

using adjlist = std::vector<int>;

class Graph
{
public:
    std::vector<adjlist> _adjlists;
    size_t _num_node;
    size_t _raw_num_node;
    size_t _num_edge;
    size_t _raw_num_edge;
    std::vector<int> _x; // _x[v]: v in V, the number of real nodes that can be reached from v using virtual edges
    std::vector<int> _y; // _y[v]: v's real outadjsum.

    bool load_graph(const std::string &file_path) {
        auto load_start = time(nullptr);

        FILE *f = fopen(file_path.c_str(), "r");

        if (f == 0) {
            std::cout << "file cannot open! " << file_path << std::endl;
            exit(0);
        }

        std::vector<std::pair<int, int>> edges;

        int u = 0;
        int v = 0;
        _num_node = 0;
        while (fscanf(f, "%d %d", &u, &v) > 0) {
            assert(u >= 0);
            assert(v >= 0);
            _num_node = std::max(_num_node, size_t(u + 1));
            _num_node = std::max(_num_node, size_t(v + 1));
            edges.emplace_back(std::pair<int, int>(u, v));
        }
        fclose(f);

        std::sort(edges.begin(), edges.end());

        _num_edge = edges.size();

        _adjlists.clear();
        _adjlists.resize(_num_node);
        for (auto edge : edges) {
            _adjlists[edge.first].emplace_back(edge.second);
        }

        return true;
    }
};

bool compare(Graph& de_compress_graph, Graph& normal_graph){
    std::cout << "_num_edge: " << de_compress_graph._num_edge << ", " << normal_graph._num_edge << std::endl;
    std::cout << "_num_node: " << de_compress_graph._num_node << ", " << normal_graph._num_node << std::endl;
    // if(de_compress_graph._num_edge != normal_graph._num_edge || de_compress_graph._num_node != normal_graph._num_node){
    //     return false;
    // }
    for(int i = 0; i < de_compress_graph._num_node; i++){
        // std::cout << i << std::endl;
        if(de_compress_graph._adjlists[i].size() != normal_graph._adjlists[i].size()){
            std::cout << "i: " << i << ", size: " << de_compress_graph._adjlists[i].size() << ", " << normal_graph._adjlists[i].size() << std::endl;
            for(int q : de_compress_graph._adjlists[i]){
                std::cout << q << ", ";
            }
            std::cout << std::endl;
            for(int q : normal_graph._adjlists[i]){
                std::cout << q << ", ";
            }
            std::cout << std::endl;
            return false;
        }
        std::sort(de_compress_graph._adjlists[i].begin(), de_compress_graph._adjlists[i].end());
        std::sort(normal_graph._adjlists[i].begin(), normal_graph._adjlists[i].end());
        for(int j = 0; j < de_compress_graph._adjlists[i].size(); j++){
            if(normal_graph._adjlists[i][j] != de_compress_graph._adjlists[i][j]){
                std::cout << "i: " << i << ", adj: " << de_compress_graph._adjlists[i][j] << ", " << normal_graph._adjlists[i][j] << std::endl;
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char const *argv[])
{
    // g++ ./tools/check_compress.cc && ./a.out ./out/de_edge.e /home/yusong/dataset/web-uk-2005/web-uk-2005_base.e 
    // g++ ./tools/check_compress.cc && ./a.out ./out/de_edge.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_0.01.e
    // g++ ./tools/check_compress.cc && ./a.out ./out/de_edge.e /home/yusong/dataset/test/a_updated_0.01.e
    timer_start(true);
    std::string compress_path = argv[1];
    std::string normal_path = argv[2];

    timer_next("load compress graph");
    Graph de_compress_graph = Graph();
    de_compress_graph.load_graph(compress_path);
    timer_next("load normal graph");
    Graph normal_graph = Graph();
    normal_graph.load_graph(normal_path);

    std::cout << "load finish..." << std::endl;

    timer_next("compare");
    bool f = compare(de_compress_graph, normal_graph);
    if(f){
        std::cout << "same..." << std::endl;
    }
    else{
        std::cout << "not same..." << std::endl;
    }

    timer_end(false, "-del_add_edge");

    return 0;
}



