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

class Delete_add_edge
{
public:
    std::vector<adjlist> _adjlists;
    size_t _num_node;
    size_t _raw_num_node;
    size_t _num_edge;
    size_t _raw_num_edge;
    std::unordered_map<int, int> vertex_map; //原id: 新id
    std::unordered_map<int, int> vertex_reverse_map; // 新id: 原id

    bool load_graph(const std::string &file_path) {
        auto load_start = time(nullptr);

        FILE *f = fopen(file_path.c_str(), "r");

        if (f == 0) {
            std::cout << "file cannot open! " << file_path << std::endl;
            return false;
        }

        std::vector<std::pair<int, int>> edges;

        int u = 0;
        int v = 0;
        _num_node = 0;
        while (fscanf(f, "%d %d", &u, &v) > 0) {
            assert(u >= 0);
            assert(v >= 0);
            // _num_node = std::max(_num_node, size_t(u + 1));
            // _num_node = std::max(_num_node, size_t(v + 1));
            // edges.emplace_back(std::pair<int, int>(u, v));
            if(vertex_map.find(u) == vertex_map.end()){
                vertex_map[u] = _num_node;
                vertex_reverse_map[_num_node] = u;
                u = _num_node++;
            }
            else{
                u = vertex_map[u];
            }
            if(vertex_map.find(v) == vertex_map.end()){
                vertex_map[v] = _num_node;
                vertex_reverse_map[_num_node] = v;
                v = _num_node++;
            }
            else{
                v = vertex_map[v];
            }
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

    void run(const std::string &edge_path, const float rate){
        // web-uk-2002-all_base.e
        int pos = edge_path.find_last_of("b"); // 找到第一个空格的位置
        std::string base_path = edge_path.substr(0, pos);
        std::string updated_path = base_path + "updated.e";
        std::string update_path = base_path + "update.e";
        FILE *f_updated = fopen(updated_path.c_str(), "w");
        FILE *f_update = fopen(update_path.c_str(), "w");
        if (f_updated == nullptr) {
            std::cout << "graph file cannot create!" << updated_path << std::endl;
            exit(0);
        }
        if (f_update == nullptr) {
            std::cout << "graph file cannot create!" << update_path << std::endl;
            exit(0);
        }
        timer_next("load_data");
        load_graph(edge_path);
        int update_num = _num_edge * rate;
        bool is_add = true;

        printf("load finied, node_num=%d edge_num=%d\n", _num_node, _num_edge);

        std::vector<int>::iterator it;
        int u, v;
        int add_num = 0;
        int del_num = 0;
        std::cout << "update_num=" << update_num << std::endl;
        timer_next("add_and_del");
        while(update_num > 0){
            update_num--;
            if(is_add){ // add edge
                is_add = false;
                int cnt = 20;
                while(cnt--){
                    u = rand() % _num_node;
                    v = rand() % _num_node;
                    it = find(_adjlists[u].begin(), _adjlists[u].end(), v);
                    if(it == _adjlists[u].end()){
                        // printf("a %d %d cnt=%d\n", u, v, update_num);
                        // printf("a %d %d cnt=%d\n", vertex_reverse_map[u], vertex_reverse_map[v], update_num);
                        fprintf(f_update, "a %d %d\n", vertex_reverse_map[u], vertex_reverse_map[v]);
                        _adjlists[u].emplace_back(v);
                        add_num++;
                        break;
                    }
                }
            }
            else{ // delete edge
                is_add = true;
                int cnt = 20;
                while(cnt--){
                    u = rand() % _num_node;
                    if(_adjlists[u].size() > 2){
                        v = rand() % (_adjlists[u].size());
                        // printf("d %d %d\n", vertex_reverse_map[u], vertex_reverse_map[_adjlists[u][v]]);
                        fprintf(f_update, "d %d %d\n", vertex_reverse_map[u], vertex_reverse_map[_adjlists[u][v]]);
                        _adjlists[u][v] = _adjlists[u][_adjlists[u].size()-1];
                        _adjlists[u].pop_back();
                        del_num++;
                        break;
                    }
                }
            }
        }
        // 写入updated文件：
        timer_next("write_data");
        for (int u = 0; u < _num_node; u++) {
            for (int v : _adjlists[u]) {
                fprintf(f_updated, "%d %d\n", vertex_reverse_map[u], vertex_reverse_map[v]);
            }
        }
        fclose(f_updated);
        fclose(f_update);
        printf("all_num=%d add_num=%d del_num=%d rate=%f\n", _num_edge, add_num, del_num, (add_num+del_num)*1.0/_num_edge);
        std::cout << update_path << std::endl;
        std::cout << updated_path << std::endl;
    }
};

int main(int argc, char const *argv[])
{
    // ./a.out /home/yusong/dataset/web-uk-2002-all/web-uk-2002-all_base.e 0.01
    timer_start(true);
    std::string edge_path = argv[1];
    float rate = atof(argv[2]);

    Delete_add_edge deal = Delete_add_edge();
    deal.run(edge_path, rate);

    timer_end(false, "-del_add_edge");

    return 0;
}

