#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include <glog/logging.h>
#include "./flags.h"
#include "../tools/find_pattern.h"
#include "../app/shortestpath.h"

int main(int argc,char *argv[]) {
    // g++ main_test.cc -lgflags -lglog && ./a.out ../input/test_data_sssp_pattern_2.e ../out/temp

    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true; //设置日志消息是否转到标准输出而不是日志文件, 即true：不写入文件，只在终端显示
    FLAGS_colorlogtostderr = true;  // Set log color

    timer_start(true);
    std::string efile(argv[1]); 
    FindPattern<int, float> finder = FindPattern<int, float>();
    finder.app_ = new ShortestpathIterateKernel<int, float>();
    timer_next("load_graph");
    finder.load(efile);
    timer_next("find pattern");
    finder.start_find();
    timer_next("wirte file");
    finder.write_supernode("../out/a_pattern");
    timer_end(false);


    google::ShutDownCommandLineFlags();
    google::ShutdownGoogleLogging();
    return 0;
}