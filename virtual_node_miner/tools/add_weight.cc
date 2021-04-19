/*
 * 对边进行加权：默认权重都为1
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

void add_weight(const std::string &edge_path, const int &is_rand_weight, const int MAX_W){
    int pos = edge_path.find_last_of("."); // 找到第一个空格的位置
    std::string base_path = edge_path.substr(0, pos);
    std::string weighted_path = base_path + "_weighted_" + std::to_string(is_rand_weight) + ".e";
    std::ifstream inFile(edge_path);

    std::cout << edge_path << std::endl;
    std::cout << weighted_path << std::endl;

    if(!inFile){
        std::cout << "open file failed. " << edge_path << std::endl;
        exit(0);
    }
    std::ofstream outfile(weighted_path);
    if(!outfile){
        std::cout << "open file failed. " << weighted_path << std::endl;
        exit(0);
    }
    int u, v;
    double w;
    int edge_cnt = 0;
    // const int MAX_W = 20;
    while(inFile >> u >> v){
        if(is_rand_weight == 0){
            outfile << u << " " << v << " " << 1 << std::endl;
        }
        else{
            w = (rand() % MAX_W) + 1;
            outfile << u << " " << v << " " << w << std::endl;
        }
        edge_cnt++;
    }
    std::cout << "finish edge_cnt=" << edge_cnt << std::endl;
}

int main(int argc, char const *argv[]){
    // input:  g++ add_weight.cc && ./a.out /home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31.e
    timer_start(true);
    std::string edge_path = argv[1];
    int is_rand_weight = std::atoi(argv[2]);
    int MAX_W = std::atoi(argv[2]);
    timer_next("add_weight");
    add_weight(edge_path, is_rand_weight, MAX_W);
    timer_end(false);

    return 0;
}