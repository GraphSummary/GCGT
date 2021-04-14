#include "../tools/find_pattern_sssp.h"

int main(int argc,char *argv[]) {
    // g++ main_test.cc && ./a.out ../input/test_data_sssp_pattern_2.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/code/a_autoInc/AutoInc/dataset/p2p-31.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/p2p-Gnutella31/p2p-Gnutella31_weighted.e
    // g++ find_pattern_sssp.cc && ./a.out /home/yusong/dataset/inf-roadNet-CA/inf-roadNet-CA_weighted.e
    timer_start(true);
    std::string efile(argv[1]); 
    FindPatternForSSSP<int, float> finder = FindPatternForSSSP<int, float>();
    timer_next("load_graph");
    finder.load(efile);
    timer_next("find pattern");
    finder.start();
    timer_next("wirte file");
    finder.write_supernode("../out/a_pattern");
    timer_end(false);

    return 0;
}