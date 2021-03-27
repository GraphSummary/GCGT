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
#include "utils/timer.h"

#define VIRTUAL_NODE_START -1

using namespace std;

double d = 0.85;
double threshold = 1e-6;

// 用于压缩图的节点
struct Page
{
    vector<int> outPage; //出邻居, 不能有重复的边
    int outAdjNum = 0; // 出度：即真实连接的点的个数
    double value = 0;
    double oldDelta= 0;
    double recvDelta = 1-d;
    int weight=1; // 如果时真实点则为1，虚拟点则为其通过虚拟边可以到达的真实点的个数
    // void print(){
    //     cout << "value=" << value << ", oldDelta=" << oldDelta << ", recvDelta=" <<  recvDelta << "weight=" << weight << endl;
    //     cout << "outPage=";
    //     for(auto p : outPage){
    //         cout << p << ", ";
    //     }
    //     cout << "\n-------------------" << endl;
    // }
};

class DeltaPageRankSum{
public:
    DeltaPageRankSum(){}
    void read_data(string &line, int& k, vector<int>& data) {
        string linestr(line);
        int pos = linestr.find_first_of(" "); //找到第一个空格的位置
        if (pos == -1) return; 

        int source = atoi(linestr.substr(0, pos).c_str()); //字符串转换成数值  源点
        if(vertex_map.find(source) == vertex_map.end()){
            vertex_map[source] = vertex_num;  // old_id -> new_id
            vertex_reverse_map[vertex_num] = source; // new_id -> old_id
            source = vertex_num++;
        }
        else{
            source = vertex_map[source];
        }
        vector<int> linkvec;
        string links = linestr.substr(pos + 1);
        // cout<<"links:"<<links<<endl;
        if (*(links.end() - 1) != ' ') {
            links = links + " ";
        }
        int spacepos = 0;
        while ((spacepos = links.find_first_of(" ")) != links.npos) {
            int to;
            if (spacepos > 0) {
                to = atoi(links.substr(0, spacepos).c_str());
                int temp = to;
                if(vertex_map.find(to) == vertex_map.end()){
                    vertex_map[to] = vertex_num;
                    vertex_reverse_map[vertex_num] = to;
                    to = vertex_num++;
                }
                else{
                    to = vertex_map[to];
                }
            }
            else{
                cout << "line94[read_data]: " << to << "，"<< line << endl;
            }
            links = links.substr(spacepos + 1);
            linkvec.emplace_back(to);
        }
        k = source;
        data = linkvec;
    }
    //把每个页面的信息（outPage，value，oldDelta，recvDelta）存到向量里
    void getPagesVec(const std::string &compress_edge_path){
        ifstream inFile(compress_edge_path);
        if(!inFile){
            cout << "open file failed. " << compress_edge_path << endl;
        }
        cout << "finish read file..." << compress_edge_path << endl;
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
            // pages[u].outAdjNum += pages[v].weight;
        }
        inFile.close();
    }

    // 读取所有点（包括虚拟点）
    void read_nodes(const std::string &compress_vertex_path) {
        ifstream inFile(compress_vertex_path);
        if(!inFile){
            cout << "open file failed. " << compress_vertex_path << endl;
        }
        cout << "finish read file..." << compress_vertex_path << endl;
        int all_nodes_numble, real_nodes_numble;
        inFile >> all_nodes_numble >> real_nodes_numble;  // 文件第一行表示所有顶点个数
        cout << "all_nodes_numble=" << all_nodes_numble << ", real_nodes_numble=" << real_nodes_numble << endl;
        // 申请node数组
        pages = new Page[all_nodes_numble + 1];
        int u, v, outAdjNum;
        while(inFile >> u >> v >> outAdjNum){
            if(vertex_map.find(u) == vertex_map.end()){
                vertex_map[u] = vertex_num;
                vertex_reverse_map[vertex_num] = u;
                u = vertex_num++;
            }
            else{
                u = vertex_map[u];
            }
            pages[u].weight = v;
            pages[u].outAdjNum = outAdjNum;
            if(v > 1){
                pages[u].recvDelta = 0; // 虚拟点初始值为0
                // 虚拟点在文件最后，记录第一个出现的位置
                if(virtual_node_start == VIRTUAL_NODE_START){
                    virtual_node_start = u;
                }
            }
        }
        cout << "vertex_num=" << vertex_num << endl;
        inFile.close();
    }

    void write_result(const std::string &outPath_compress){
        cout << "\nout path: " << outPath_compress << endl;
        ofstream fout_com(outPath_compress);
        for(int i = 0; i < vertex_num; i++){
            Page& p_ = pages[i];
            if(p_.weight == 1){ // 只写入真实点
                fout_com << vertex_reverse_map[i] << ' ' << p_.value << endl;
            }
        }
        fout_com.close();
    }

    void load_data(const std::string &com_e, const std::string &com_v){
        // com_base_e, com_base_v
        // string v_path = "./out/" + filename + ".v"; // 压缩图点文件，包含超点和源顶点的对应关系
        // string e_path = "./out/" + filename + ".e"; // 压缩图边文件，超点之间的边
        read_nodes(com_v);
        getPagesVec(com_e);
        if(virtual_node_start == -1){
            virtual_node_start = vertex_num;
        }
        cout << "vertex_num=" << vertex_num << ", virtual_node_start=" << virtual_node_start << endl;
    }

    // 在旧的图上进回收: 迭代一次负消息
    // 在新的图上进补收: 迭代一次正消息
    void amend(int type){
        {
            double delta_sum = 0;
            double delta_sum_v = 0;
            // real node send
            for(int i = 0; i < virtual_node_start; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                // double tmpDelta = page.oldDelta / outDegree * d * type; // - or +
                double tmpDelta = page.value / outDegree * d * type; // - or +
                delta_sum += std::fabs(tmpDelta);
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta * pages[p].weight;
                }
            }
            // virtual node receivel
            for(int i = virtual_node_start; i < vertex_num; i++){
                Page& page = pages[i];
                // delta_sum += page.recvDelta;
                // page.value += page.recvDelta;
                page.oldDelta = page.recvDelta;  // 必须更新
                page.recvDelta = 0;
            }
            // virtual node send
            for(int i = virtual_node_start; i < vertex_num; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                double tmpDelta = page.oldDelta / outDegree; // virtual_node 只作转发，故不需要 * type
                delta_sum_v += std::fabs(tmpDelta);
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta * pages[p].weight;
                }
            }
            // real node receive
            // for(int i = 0; i < virtual_node_start; i++){
            //     Page& page = pages[i];
            //     delta_sum += std::abs(page.recvDelta);
            //     page.value += page.recvDelta;
            //     page.oldDelta = page.recvDelta;  // 必须更新
            //     page.recvDelta = 0;
            // }
            cout << "type=" << type << " amend_delta_sum=" << delta_sum <<  " delta_sum_v=" << delta_sum_v << endl;
        }
    }

    int run(){  
        // 将运行次数写入文件
        string resultPath = "./out/result.txt";
        ofstream fout_1(resultPath, ios::app);
        int step = 0; //统计迭代几轮
        int increment_num = 1;
        float delta_sum = 0;
        cout << "开始计算..." << endl;
        computer_num++;
        while(1)
        {
            delta_sum = 0;
            step++;
            // real node send
            for(int i = 0; i < virtual_node_start; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                double tmpDelta = page.oldDelta / outDegree * d;
                // tmpDelta *= (page.weight == 1 ? d : 1); // 真实点应该是1，虚拟点则全部发送出去
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta * pages[p].weight;
                }
            }
            // virtual node receivel
            for(int i = virtual_node_start; i < vertex_num; i++){
                Page& page = pages[i];
                delta_sum += page.recvDelta;
                // page.value += page.recvDelta;
                page.oldDelta = page.recvDelta;  // 必须更新
                page.recvDelta = 0;
            }
            // virtual node send
            for(int i = virtual_node_start; i < vertex_num; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                double tmpDelta = page.oldDelta / outDegree;
                if(std::fabs(tmpDelta) <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta * pages[p].weight;
                }
            }
            // real node receive
            for(int i = 0; i < virtual_node_start; i++){
                Page& page = pages[i];
                // delta_sum += page.recvDelta;
                delta_sum += std::fabs(page.recvDelta);
                page.value += page.recvDelta;
                page.oldDelta = page.recvDelta;  // 必须更新
                page.recvDelta = 0;
            }
            if(delta_sum < threshold){
                break;
            }
            // cout << "step=" << step << ", delta_sum=" << delta_sum << endl;
        }
        cout << "step=" << step << ", Compressed graph convergence" << ", delta_sum=" << delta_sum << endl;
        fout_1 << "compress_graph_" << computer_num << "_step:" << step << endl;
        fout_1.close();
    }
    // ~DeltaPageRankSum(){
    //     delete pages;
    // }
public:
    // unordered_map<int, Page> pages; //pages存放页面向量，大小为N
    Page* pages; //pages存放页面向量，大小为N
    unordered_map<int, int> vertex_map; //原id: 新id
    unordered_map<int, int> vertex_reverse_map; // 新id: 原id
    int virtual_node_start=VIRTUAL_NODE_START; // 虚拟点的第一个id
    int vertex_num = 0; // 所有点的个数
    static int computer_num;
};

void copy_data(DeltaPageRankSum &d1, DeltaPageRankSum &d2, int vertex_num){
    Page* pages = d1.pages;
    Page* inc_pages = d2.pages;
    for(int i = 0; i < vertex_num; i++){
        if(pages[i].weight == 1){ // 只写入真实点
            int x = d1.vertex_reverse_map[i]; // 真实id(x): i -> x
            int y = d2.vertex_map[x]; // 真实id(x): x -> y(新id)  
            inc_pages[y].value = pages[i].value;
            inc_pages[y].oldDelta = pages[i].oldDelta;
            inc_pages[y].recvDelta = pages[i].recvDelta;
        }
    }
}

//初始化静态成员变量
int DeltaPageRankSum::computer_num = 0;

int main(int argc, char const *argv[])
{   
    // 运行命令: ./a.out 0.0000001 ./input/p2p-31_new.e p2p-31
    // 提取参数： $CONVERGENCE_THRESHOLD ${com_base_e} ${com_base_v} ${com_updated_e} ${com_updated_v}
    threshold = atof(argv[1]);
    string com_base_e(argv[2]);     // 原图压缩文件
    string com_base_v(argv[3]);
    string com_updated_e(argv[4]);  // 新图压缩文件
    string com_updated_v(argv[5]);
    string outPath_compress_1 = "./out/pr_delta_sum_com_1.txt";  // 保存结果
    string outPath_compress_2 = "./out/pr_delta_sum_com.txt";  // 保存结果

    // 初始化计时器 
    timer_start(true);
    // 原图计算
    DeltaPageRankSum deltaPageRankSum = DeltaPageRankSum();
    timer_next("load data");
    deltaPageRankSum.load_data(com_base_e, com_base_v);
    timer_next("compute");
    deltaPageRankSum.run();
    timer_next("write result");
    deltaPageRankSum.write_result(outPath_compress_1);

    // 增量计算
    DeltaPageRankSum deltaPageRankSum_inc = DeltaPageRankSum();
    timer_next("inc_load_data");
    deltaPageRankSum_inc.load_data(com_updated_e, com_updated_v);

    timer_next("update_graph_data");
    // 新图旧图的切换
    // 回收
    deltaPageRankSum.amend(-1);
    // 切换数据
    copy_data(deltaPageRankSum, deltaPageRankSum_inc, deltaPageRankSum.vertex_num);
    // 补发
    deltaPageRankSum_inc.amend(1);

    timer_next("inc_compute");
    deltaPageRankSum_inc.run();
    timer_next("write_result");
    deltaPageRankSum_inc.write_result(outPath_compress_2);
 
    timer_end(true, "-com_graph");
    return 0;
}
