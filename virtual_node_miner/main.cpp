#include <iostream>
#include "virtual_node_miner.hpp"
#include <string>
#include <vector>
#include <cstdio>
#include <sys/resource.h>
#include "utils/timer.h"


int main(int argc,char *argv[]) {
    timer_start(true);
    auto start_com = clock();
    if (argc != 9) {
        printf("incorrect arguments.\n");
        printf("${base_e} ${update_e} ${com_base_e} ${com_base_v} ${com_updated_e} ${com_updated_v} $CLUSTER_THRESHOLD $VIRTUAL_THRESHOLD\n");
        abort();
    }
    std::string base_e(argv[1]);
    std::string update_e(argv[2]);
    std::string com_base_e(argv[3]);
    std::string com_base_v(argv[4]);
    std::string com_updated_e(argv[5]);
    std::string com_updated_v(argv[6]);
    int CLUSTER_THRESHOLD = atoi(argv[7]);
    int VIRTUAL_THRESHOLD = atoi(argv[8]);

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
    // 原图压缩
    timer_next("load_graph");
    vnminer.load_graph(base_e);
    timer_next("compress_base");
    vnminer.compress(5);
    timer_next("write_com_base_e");
    vnminer.write_graph(com_base_e); // write edge file
    timer_next("write_com_base_v");
    vnminer.computeX(); // compute x[v] v in V, The number of real nodes that can be reached from v using virtual edges
    vnminer.computeY(); // y[v]: v's real outadjsum.
    vnminer.write_vertex(com_base_v); // 

    // 增量压缩
    // timer_next("increment_compress");
    vnminer.increment_compress(update_e);
    timer_next("write_com_updated_e");
    vnminer.write_graph(com_updated_e); // write edge file
    timer_next("write_com_updated_v");
    vnminer.computeX(); // compute x[v] v in V, The number of real nodes that can be reached from v using virtual edges
    vnminer.computeY(); // y[v]: v's real outadjsum.
    vnminer.write_vertex(com_updated_v); // 

    // 测试
    // std::cout << "=====测试=========" << std::endl;
    // timer_next("decompress");
    // vnminer.decompress(); // 解压，用于测试压缩是否正确
    // timer_next("write_decompress_e");
    // vnminer.write_de_graph("./out/de_edge.e"); // write edge file

    timer_end(true, "-compress");

    return 0;
}