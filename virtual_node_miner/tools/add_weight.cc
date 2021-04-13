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

void add_weight(const std::string &edge_path){
    int pos = edge_path.find_last_of("."); // 找到第一个空格的位置
    std::string base_path = edge_path.substr(0, pos);
    std::string weighted_path = base_path + "_weighted" + ".e";
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
    while(inFile >> u >> v){
        outfile << u << " " << v << " " << 1 << std::endl;
        edge_cnt++;
    }
    std::cout << "finish edge_cnt=" << edge_cnt << std::endl;
}

int main(int argc, char const *argv[]){
    // input:  g++ add_weight.cc && ./a.out /home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31.e
    timer_start(true);
    std::string edge_path = argv[1];
    timer_next("add_weight");
    add_weight(edge_path);
    timer_end(false);

    return 0;
}