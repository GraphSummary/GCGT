#include <iostream>
#include "virtual_node_miner.hpp"
#include <string>
#include <vector>
#include <cstdio>
#include <sys/resource.h>


int main(int argc,char *argv[]) {
    auto start_com = time(nullptr);
    if (argc != 6) {
        printf("incorrect arguments.\n");
        printf("<input_graph> <output_graph_edge> <output_graph_vertex> <CLUSTER_THRESHOLD> <VIRTUAL_THRESHOLD>\n");
        abort();
    }
    std::string input_path(argv[1]);
    std::string output_path_e(argv[2]);
    std::string output_path_v(argv[3]);
    int CLUSTER_THRESHOLD = atoi(argv[4]);
    int VIRTUAL_THRESHOLD = atoi(argv[5]);

    const rlim_t kStackSize = 1 * 1024 * 1024 * 1024;
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }

    virtual_node_miner vnminer(CLUSTER_THRESHOLD, VIRTUAL_THRESHOLD);
    vnminer.load_graph(input_path);
    vnminer.compress(5);
    vnminer.write_graph(output_path_e); // write edge file
    vnminer.computeX(); // compute x[v] v in V, The number of real nodes that can be reached from v using virtual edges
    vnminer.write_vertex(output_path_v); // 

    std::cout << "compress time=" << time(nullptr) - start_com << std::endl;

    return 0;
}