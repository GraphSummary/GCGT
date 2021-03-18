#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <ctime>

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
    void getPagesVec(string file)
    {
        cout << "finish read file..." << file << endl;
        ifstream inFile(file);
        if(!inFile){
            cout << "open file failed. " << file << endl;
        }
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
    int read_nodes_num(string compress_vertex_path) {
        ifstream inFile(compress_vertex_path);
        if(!inFile){
            cout << "open file failed. " << compress_vertex_path << endl;
        }
        int nodes_numble, _raw_num_node;
        inFile >> nodes_numble >> _raw_num_node;  // 文件第一行表示所有顶点个数
        cout << nodes_numble << ", " << _raw_num_node << endl;
        inFile.close();
        return _raw_num_node;
    }

    int run(string old_dege_file, string filename)
    {
        string v_path = "./out/" + filename + ".v"; // 压缩图点文件，包含超点和源顶

        int nodes_numble = read_nodes_num(v_path);
        pages = new Page[nodes_numble + 1]; // 申请node数组
        getPagesVec(old_dege_file);   // 边文件路径
        
        int cnt = 1; //统计迭代几轮
        double start = clock();

        // 开始迭代
        while(1)
        {
            int shouldStop = 0; //根据oldPR与newPR的差值 判断是否停止迭代
            float delta_sum = 0;
            for(int i = 0; i < nodes_numble; i++){
                Page& page = pages[i];
                int outDegree = page.outPage.size();
                double tmpDelta = page.oldDelta / outDegree * d;
                if(tmpDelta <= threshold){
                    continue;
                }
                for(auto p : page.outPage)
                    pages[p].recvDelta += tmpDelta;
            }

            for(int i = 0; i < nodes_numble; i++){
                Page& page = pages[i];
                page.value += page.recvDelta;
                delta_sum += page.recvDelta;
                page.oldDelta = page.recvDelta;
                page.recvDelta = 0;
            }
            if(delta_sum < threshold){
                break;
            }
            cnt++;
        }

        printf("%s%d\n", "step = ", cnt);
        cout << "time: " << (clock()-start) / CLOCKS_PER_SEC<< "s\n";
        // 将运行时间写入文件
        string resultPath = "./out/result.txt";
        ofstream fout_1(resultPath, ios::app);
        fout_1 << "normal_graph_time:" << (clock()-start) / CLOCKS_PER_SEC << endl;
        fout_1 << "normal_graph_step:" << cnt << endl;
        fout_1.close();

        string outPath = "./out/pr_delta_pre.txt";
        cout << "out path: " << outPath << endl;
        ofstream fout(outPath);
        for(int i = 0; i < nodes_numble; i++){
            Page& page = pages[i];
            fout << vertex_reverse_map[i] << ' ' << page.value << endl;
        }
        fout.close();
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
};

int main(int argc, char const *argv[])
{
    // 运行命令: ./a.out 0.0000001 ./input/p2p-31_new.e p2p-31
    threshold = atof(argv[1]);
    DeltaPageRank deltaPageRank = DeltaPageRank();
    string edge_new_path(argv[2]);
    string filename(argv[3]);
    deltaPageRank.run(edge_new_path, filename);
    return 0;
}

