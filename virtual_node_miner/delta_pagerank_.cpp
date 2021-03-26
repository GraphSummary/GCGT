#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <ctime>
#include "utils/timer.h"
#include <math.h>

using namespace std;

double d = 0.85;
double threshold = 1e-6;

struct Page
{
    vector<int> outPage; //出邻居
    double value = 0;
    double oldDelta= 0;
    double recvDelta = 1-d;
};

class DeltaPageRank{
public:
    //把每个页面的信息（outPage，value，oldDelta，recvDelta）存到向量里
    void getPagesVec(const std::string &file)
    {
        ifstream inFile(file);
        if(!inFile){
            cout << "open file failed. " << file << endl;
        }
        cout << "finish read file..." << file << endl;
        int u, v;
        while(inFile >> u >> v){
            if(vertex_map.find(u) == vertex_map.end()){
                vertex_map[u] = vertex_num;
                vertex_reverse_map[vertex_num] = u;
                u = vertex_num++;
            }
            else{
                u = vertex_map[u];
            }
            if(vertex_map.find(v) == vertex_map.end()){
                vertex_map[v] = vertex_num;
                vertex_reverse_map[vertex_num] = v;
                v = vertex_num++;
            }
            else{
                v = vertex_map[v];
            }
            pages[u].outPage.emplace_back(v);
        }
        inFile.close();
    }
    // 读取所有点
    int read_nodes_num(const std::string &compress_vertex_path) {
        ifstream inFile(compress_vertex_path);
        if(!inFile){
            cout << "open file failed. " << compress_vertex_path << endl;
        }
        cout << "finish read file..." << compress_vertex_path << endl;
        int nodes_numble, _raw_num_node;
        inFile >> nodes_numble >> _raw_num_node;  // 文件第一行表示所有顶点个数
        cout << nodes_numble << ", " << _raw_num_node << endl;
        inFile.close();
        return _raw_num_node;
    }

    void write_result(const std::string &outPath){
        cout << "out path: " << outPath << endl;
        timer_next("write_graph");
        ofstream fout(outPath);
        for(int i = 0; i < vertex_num; i++){
            Page& page = pages[i];
            fout << vertex_reverse_map[i] << ' ' << page.value << endl;
        }
        fout.close();
    }

    void load_data(const std::string &base_e, const std::string &com_base_v){
        int nodes_numble = read_nodes_num(com_base_v);
        pages = new Page[nodes_numble + 1]; // 申请node数组
        getPagesVec(base_e);   // 边文件路径
    }

    void amend(int type){
        {
            double delta_sum = 0;
            for(int i = 0; i < vertex_num; i++){
                Page& page = pages[i];
                int outDegree = page.outPage.size();
                // double tmpDelta = page.oldDelta / outDegree * d * type; // type: -1 or 1
                double tmpDelta = page.value / outDegree * d * type; // type: -1 or 1
                delta_sum += std::fabs(tmpDelta);
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta;
                }
            }

            // for(int i = 0; i < vertex_num; i++){
            //     Page& page = pages[i];
            //     page.value += page.recvDelta;
            //     delta_sum += std::fabs(page.recvDelta);
            //     page.oldDelta = page.recvDelta;
            //     page.recvDelta = 0;
            // }
            cout << "---type=" << type << " amend_delta_sum=" << delta_sum << endl;
        }

    }

    int run(){
        int step = 0; //统计迭代几轮
        float delta_sum = 0;
        // 开始迭代
        computer_num++;
        // while(1)
        while(step < 200)
        {
            delta_sum = 0;
            step++;
            // send
            for(int i = 0; i < vertex_num; i++){
                Page& page = pages[i];
                int outDegree = page.outPage.size();
                double tmpDelta = page.oldDelta / outDegree * d;
                // if(tmpDelta <= threshold){
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta;
                }
            }
            // receive
            for(int i = 0; i < vertex_num; i++){
                Page& page = pages[i];
                page.value += page.recvDelta;
                delta_sum += std::fabs(page.recvDelta); // 负的累加在一起会抵消，导致提前小于阈值
                page.oldDelta = page.recvDelta;
                page.recvDelta = 0;
            }
            if(delta_sum < threshold){ // delta_sum 可能为负？？？
                break;
            }
            // cout << "step=" << step << ", receive_delta_sum_r=" << delta_sum << << endl;
        }

        cout << "step=" << step << ", normal graph convergence" << ", delta_sum=" << delta_sum << endl;
        // 将运行次数写入文件
        string resultPath = "./out/result.txt";
        ofstream fout_1(resultPath, ios::app);
        fout_1 << "normal_graph_" << computer_num << "_step:" << step << endl;
        fout_1.close();

        return 0;
    }
    // ~DeltaPageRank(){
    //     delete pages;
    // }
public:
    Page* pages; //pages存放页面向量，大小为N
    unordered_map<int, int> vertex_map; //原id: 新id
    unordered_map<int, int> vertex_reverse_map; // 新id: 原id
    int vertex_num = 0; // 所有点的个数
    static int computer_num;
};

void copy_data(DeltaPageRank &d1, DeltaPageRank &d2, int vertex_num){
    Page* pages = d1.pages;
    Page* inc_pages = d2.pages;
    for(int i = 0; i < vertex_num; i++){
        int x = d1.vertex_reverse_map[i]; // 真实id(x): i -> x
        int y = d2.vertex_map[x]; // 真实id(x): x -> y(新id)  
        inc_pages[y].value = pages[i].value;
        inc_pages[y].oldDelta = pages[i].oldDelta;
        inc_pages[y].recvDelta = pages[i].recvDelta;
    }
}

int DeltaPageRank::computer_num = 0;

int main(int argc, char const *argv[])
{
    // 运行命令: ./a.out 0.0000001 ./input/p2p-31_new.e p2p-31
    // ./a.out $CONVERGENCE_THRESHOLD ${base_e} ${com_base_v} ${updated_e} ${com_updated_v}
    if (argc < 4) {
        printf("incorrect arguments.\n");
        printf("$CONVERGENCE_THRESHOLD ${base_e} ${com_base_v} ${updated_e} ${com_updated_v}\n");
        abort();
    }

    threshold = atof(argv[1]);
    string base_e(argv[2]);
    string com_base_v(argv[3]);
    string updated_e(argv[4]);
    string com_updated_v(argv[5]);
    string outPath1 = "./out/pr_delta_pre_1.txt";
    string outPath2 = "./out/pr_delta_pre.txt";

    // 初始化计时器 
    timer_start(true);

    // 原图计算
    DeltaPageRank deltaPageRank = DeltaPageRank();
    timer_next("load_graph");
    deltaPageRank.load_data(base_e, com_base_v);
    timer_next("compute_graph");
    deltaPageRank.run();
    timer_next("write_result");
    deltaPageRank.write_result(outPath1);

    // 增量计算
    DeltaPageRank deltaPageRank_inc = DeltaPageRank();
    timer_next("inc_load_data");
    deltaPageRank_inc.load_data(updated_e, com_updated_v);

    timer_next("update_graph_data");
    // 新图旧图的切换
    // 回收
    deltaPageRank.amend(-1);
    // 切换数据
    copy_data(deltaPageRank, deltaPageRank_inc, deltaPageRank.vertex_num);
    // deltaPageRank_inc.write_result("./out/pr_delta_pre_test.txt"); //测试
    // 补发
    deltaPageRank_inc.amend(1);

    timer_next("inc_compute");
    deltaPageRank_inc.run();
    timer_next("write_result");
    deltaPageRank_inc.write_result(outPath2);


    timer_end(true, "-nor_graph");
    return 0;
}

