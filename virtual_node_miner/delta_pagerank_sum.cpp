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
// 用于普通图的节点
struct Page_de
{
    // unordered_set<int> outPage; //出邻居, 可能有重复的边
    vector<int> outPage; //出邻居, 不能有重复的边
    double value = 0;
    double oldDelta= 0;
    double recvDelta = 1-d;
    Page_de(){}
    Page_de(double value_, double oldDelta_, double recvDelta_){
        value = value_;
        oldDelta = oldDelta;
        recvDelta = recvDelta_;
    }
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
    void getPagesVec(string compress_edge_path){
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
            pages[u].outAdjNum += pages[v].weight;
        }
        inFile.close();
    }

    // 加载新图的边
    // void getPagesVec_new(string edge_new_path)
    // {
    //     cout << "read file... " << edge_new_path << endl;
    //     ifstream inFile(edge_new_path);
    //     int u, v;
    //     while(inFile >> u >> v){
    //         // pages_new[u].outPage.insert(v);
    //         if(vertex_map.find(u) == vertex_map.end()){
    //             vertex_map[u] = vertex_num;
    //             vertex_reverse_map[vertex_num] = u;
    //             u = vertex_num++;
    //         }
    //         else{
    //             u = vertex_map[u];
    //         }
    //         if(vertex_map.find(v) == vertex_map.end()){
    //             vertex_map[v] = vertex_num;
    //             vertex_reverse_map[vertex_num] = v;
    //             v = vertex_num++;
    //         }
    //         else{
    //             v = vertex_map[v];
    //         }
    //         pages_new[u].outPage.emplace_back(v);
    //     }
    //     inFile.close();
    // }

    // 读取所有点（包括虚拟点）
    void read_nodes(string compress_vertex_path) {
        ifstream inFile(compress_vertex_path);
        if(!inFile){
            cout << "open file failed. " << compress_vertex_path << endl;
        }
        cout << "finish read file..." << compress_vertex_path << endl;
        int nodes_numble, v_nodes_numble;
        inFile >> nodes_numble >> v_nodes_numble;  // 文件第一行表示所有顶点个数
        // 申请node数组
        pages = new Page[nodes_numble + 1];
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
            pages[u].weight = v;
            if(v > 1){
                pages[u].recvDelta = 0; // 虚拟点初始值为0
                if(virtual_node_start == VIRTUAL_NODE_START){
                    virtual_node_start = u;
                }
            }
        }
        inFile.close();
    }

    int run(string edge_new_path_, string filename){   
        string v_path = "./out/" + filename + ".v"; // 压缩图点文件，包含超点和源顶点的对应关系
        string e_path = "./out/" + filename + ".e"; // 压缩图边文件，超点之间的边
        string edge_new_path = edge_new_path_;  // 原始图文件，即解压之后的图
        string outPath = "./out/pr_delta_sum.txt";   // 保存最终计算结果
        string outPath_compress = "./out/pr_delta_sum_com.txt";   // 保存第一次收敛结果
        
        auto load_data_time = clock();
        read_nodes(v_path);
        getPagesVec(e_path);
        cout << "load_data_time=" << (clock() - load_data_time)/ CLOCKS_PER_SEC << endl;
        if(virtual_node_start == -1){
            virtual_node_start = vertex_num;
        }

        cout << "vertex_num=" << vertex_num << ", virtual_node_start=" << virtual_node_start << endl;
        // 将运行时间写入文件
        string resultPath = "./out/result.txt";
        ofstream fout_1(resultPath, ios::app);

        int step = 0; //统计迭代几轮
        int increment_num = 1;
        double pre_time = 0;
        double sum_time = 0;
        double start = clock();

        float delta_sum = 0;
        while(1)
        {
            int shouldStop = 0; //根据oldPR与newPR的差值 判断是否停止迭代
            delta_sum = 0;

            // real node send
            for(int i = 0; i < virtual_node_start; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                double tmpDelta = page.oldDelta / outDegree * d;
                // tmpDelta *= (page.weight == 1 ? d : 1); // 真实点应该是1，虚拟点则全部发送出去
                if(tmpDelta <= threshold){
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
                page.value += page.recvDelta;
                page.oldDelta = page.recvDelta;  // 必须更新
                page.recvDelta = 0;
            }
            // virtual node send
            for(int i = virtual_node_start; i < vertex_num; i++){
                Page& page = pages[i];
                int outDegree = page.outAdjNum;
                double tmpDelta = page.oldDelta / outDegree;
                if(tmpDelta <= threshold){
                    continue;
                }
                for(auto p : page.outPage){
                    pages[p].recvDelta += tmpDelta * pages[p].weight;
                }
            }
            // real node receive
            for(int i = 0; i < virtual_node_start; i++){
                Page& page = pages[i];
                delta_sum += page.recvDelta;
                page.value += page.recvDelta;
                page.oldDelta = page.recvDelta;  // 必须更新
                page.recvDelta = 0;
            }
            step++;
            // cout << "step=" << step << ", receive_delta_sum_r=" << delta_sum << ", " << "send_delta_sum_r=" << send_delta_sum_r << endl;
            // cout << "step=" << step << ", delta_sum=" << delta_sum << endl;
            if(delta_sum < threshold){
                break;
            }
        }
        cout << "step=" << step << ", Compressed graph convergence" << ", delta_sum=" << delta_sum << endl;
        pre_time = (clock()-start) / CLOCKS_PER_SEC;
        cout << "time: " << pre_time << "s" << endl;
        fout_1 << "compress_graph_1st_time:" << pre_time << endl;
        fout_1 << "compress_graph_1st_step:" << step << endl;
        // 测试: 输出压缩计算的结果
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
};

int main(int argc, char const *argv[])
{
    // 运行命令: ./a.out 0.0000001 ./input/p2p-31_new.e p2p-31
    threshold = atof(argv[1]);
    DeltaPageRankSum deltaPageRankSum = DeltaPageRankSum();
    string edge_new_path(argv[2]);
    string filename(argv[3]);
    deltaPageRankSum.run(edge_new_path, filename);
    return 0;
}
