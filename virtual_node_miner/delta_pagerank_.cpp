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

//把每个页面的信息（outPage，value，oldDelta，recvDelta）存到向量里
void getPagesVec(unordered_map<int, Page>& pages, string file)
{
    // string file = "./input/google.e";
    // string file = "./input/goo_new.e";
    ifstream inFile(file);
    int u, v;
    while(inFile >> u >> v)
        pages[u].outPage.push_back(v);
    cout << "finish read file..." << file << endl;
}

int main(int argc, char const *argv[])
{
    // 运行命令: ./a.out ./input/p2p-31_new.e
    unordered_map<int, Page> pages; //pages存放页面向量
    getPagesVec(pages, argv[1]);   // 边文件路径
    threshold = atof(argv[2]);     // 设置收敛阈值
    int cnt = 1; //统计迭代几轮
    double start = clock();

    // 开始迭代
    while(1)
    {
        int shouldStop = 0; //根据oldPR与newPR的差值 判断是否停止迭代
        float delta_sum = 0;
        //对于i->j，根据公式，页面j能接收到的delta值为：
        //recvDelta(j) = oldDelta(i) / outDegree
        //因此每个顶点i向出邻居发送的delta值为：oldDelta(i)
        // for(auto &page: pages)
        // {
        for(auto &p_: pages){
            Page& page = p_.second;
            int outDegree = page.outPage.size();
            double tmpDelta = page.oldDelta / outDegree;
            if(tmpDelta <= threshold){
                continue;
            }
            // for(int j = 0; j < outDegree; j++)
            //     pages[page.outPage[j]].recvDelta += tmpDelta * d;
            for(auto p : page.outPage)
                pages[p].recvDelta += tmpDelta * d;
        }

        for(auto &p_: pages){
            Page& page = p_.second;
            // if(page.recvDelta < threshold)
            //     shouldStop++;
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
//    for(auto page: pages)
//        printf("%.9lf\n", page.value/N);
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
    for(auto &p_: pages){
        fout << p_.first << ' ' << p_.second.value << endl;
        // cout << p_.first << ' ' << p_.second.value << endl;
    }
    fout.close();
    return 0;
}

