#include <iostream>
#include "../utils/bitset.h"
#include "../utils/timer.h"
#include "../tools/find_pattern_sssp.h"

using Bitset = grape::Bitset;

void print_bitset(Bitset &bs, int begin, int end){
    std::cout << "----------------" << std::endl;
    for(int batch = begin; batch < end; batch+=64){
        int v = batch;
        std::cout << "batch=" << batch << std::endl;
        uint64_t word = bs.get_word(batch-begin); // 获得数组中的一个元素，每个元素64位，每位表示一个元素
        while (word != 0) {
            if (word & 1) {
                std::cout << v << "\n";
            }
            ++v;
            word = word >> 1;
        }
    }
}

int test_bitset(){

    int begin = 70;
    int end = 128;

    Bitset bs;
    bs.init(end-begin);

    // 指定位置设置为1
    bs.set_bit(72 - begin);
    bs.set_bit(80 - begin);
    bs.set_bit(70 - begin);
    bs.set_bit(100 - begin);
    // 重置为0
    bs.reset_bit(72 - begin);
    
    print_bitset(bs, begin, end);
    std::cout << "bs.count=" << bs.count() << std::endl;

    Bitset bs2;
    bs2.init(end-begin);
    bs2.set_bit(84 - begin);
    print_bitset(bs2, begin, end);
    std::cout << "bs2.count=" << bs2.count() << std::endl;

    bs.parallel_clear(4); // 四个线程并行清理
    bs.swap(bs2); // 交换两个bitset的内容

    print_bitset(bs, begin, end);
    std::cout << "bs.count=" << bs.count() << std::endl;

    print_bitset(bs2, begin, end);
    std::cout << "bs2.count=" << bs2.count() << std::endl;

    return 0;
}

void test_find_pattern_sssp(char *argv[]){
    // g++ test2.cc -pthread -fopenmp && ./a.out ../input/test_data_sssp_pattern_2.e ../input/test_data_sssp_pattern_2_update.e
    // g++ test2.cc -pthread -fopenmp && ./a.out /home/yusong/dataset/roadNet-CA/roadNet-CA_weighted_0_base.e
    // g++ test2.cc -pthread -fopenmp && ./a.out /home/yusong/dataset/roadNet-CA/roadNet-CA_weighted_0_base.e
    timer_start(true);
    std::string efile(argv[1]); 
    std::string update_efile(argv[2]); 
    FindPatternForSSSP<int, int> finder = FindPatternForSSSP<int, int>();
    timer_next("load_graph");
    finder.load(efile);
    timer_next("find_pattern");
    finder.start_find();
    timer_next("wirte_file");
    finder.write_supernode("../out/a_pattern_1");

    // increment
    timer_next("load_update");
    finder.load_update(update_efile);
    timer_next("inc_update");
    finder.increment_update();
    timer_next("wirte_file");
    finder.write_supernode("../out/a_pattern_2");

    timer_end(false);

}

int main(int argc,char *argv[]){

    // test1_bitset()

    test_find_pattern_sssp(argv);

    return 0;
}