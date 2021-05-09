## Build
```make```

## Execution
```./virtual_node_miner <input_graph> <output_graph>```

The input graph should be in the traditional edgelist format, i.e.,

```
<u1> <v1>
<u2> <v2>
...
<uE> <vE>
```

# 相关笔记：
0. 文件命名介绍：
    - 例如下载的数据集为： web-uk-2002-all.e
    - 经过预处理（保证每个点都有出度）： web-uk-2002-all_base.e
    - web-uk-2002-all_base.e ---(增删)---> web-uk-2002-all_update.e(增删边操作) 和 web-uk-2002-all_updated.e(变化之后的边)
    - web-uk-2002-all_base.e ---(压缩1)---> web-uk-2002-all_base_com.e 和 web-uk-2002-all_base_com.v
    - web-uk-2002-all_updated.e ---(压缩2)---> web-uk-2002-all_updated_com.e 和 web-uk-2002-all_updated_com.v
    - 两次压缩可以一起操作：web-uk-2002-all_base.e ---(压缩1)---> web-uk-2002-all_base_com.e 和 web-uk-2002-all_base_com.v + web-uk-2002-all_update.e ---(压缩2)---> web-uk-2002-all_updated_com.e 和 web-uk-2002-all_updated_com.v

1. virtual_node_miner.hpp：
    - 输入边文件，
    - 输出两个文件：压缩后的新边文件(eq, u->v)，点文件(第一行连个数表示所有点个数和真实点个数，后面的每行三个数，eq, u weight outAdjNum, weight表示u通过虚边可以到达的真实点个数, outAdjNum表示u的真实出度； 点的编号规律为前面为真实点，后面一般为虚拟点，可以通过weight区分，weight大于1的点为虚拟点，所以假设第一个虚拟点编号为x，这0-(x-1)为真实点，x-max_id为虚拟点)。
    点文件例子： 表示一共五个点，三个真实点(0, 3, 5), 两个虚拟点(6,7)
        ```
            5 3
            0 1 2
            3 1 5
            5 1 3
            6 2 3
            7 3 2
        ```
    - 其中可以指定压缩次数，压缩次数增多在一定情况下可以增加压缩率，但是原始的代码多次压缩会产生虚拟点指向虚拟点的情况，就是在后面压缩时可能会把前面产生的虚拟点也进行压缩了。这种情况将会增加Pagerank迭代次数，故修改代码(通过不将虚拟点加入前缀树来实现)，限制参与压缩的点的范围为真实点。

2. pagerank计算：
    - 要求边数据集无重复边
    - 要求数据集的每个点必须要有出度，不满足的可以用./tools/deal_dataset.py处理：
        ```shell
        python3 deal_dataset.py /home/yusong/dataset/web_uk_2005/web-uk-2005.e
        ``` 
    - 虚拟点初始化为0，其余点初始化为1-d
    - 迭代过程的顺序为：真实点发消息，虚拟点收消息，虚拟点发消息，真实点接收消息。虚拟点和真实点不分开发送达不到合并转发的效果。
        看代码实现，如果时同步的写法其实上面的顺序可以不用管，因为都缓存到recvDelta了；如果时异步的写法，即只用一个delta,并收到的直接作用上去，则需要考虑顺序。

3. pagerank增量：
    - 回收和补发都只给delta, 不直接作用到value上。

4. SSSP:
    思路：
        原始数据 -> 添加权重(如果有则不需要) -> 顶点重新编号，将无向边转为有向边 -> 图结构提取 -> 计算SSSP
        发送消息： 结构内部的点(inner+bound)只有收敛后才发送，与该结构相连的外部点，会通过bound_edges每次更新。
        每个结构内部计算sssp可以并行。
    
5. php:
    对数据集依赖比较大：
        weight_e：原图加权后直接用
        base_e：加权图，由无向图调整为有向图
        对于有图，可以直接用weight_e进行压缩计算。
    方法：
        跟虚拟点压缩类似，最好是能够先全部发给超点，在超点处聚合，然后在发出去。
    参数：
        - 数据集
        - 超点大小：[min, max]
        - source点

# 增量的
1  1  8467.24000
0  2  9953.30000
2  3     2.39979
4  4     1.89526
5  5     1.67938
# 直接算
1  1  8459.26000
0  2  9943.92000
2  3     2.39943
4  4     1.89499
5  5     1.67913
少量误差：

I0415 16:50:53.171880 25415 traversal_worker_sum.cc:128] app convergence step=215 delta_sum=0 
g_cnt=755431745 f_cnt=334,670,940
I0415 16:50:53.171883 25415 traversal_worker_sum.cc:130] contain_cnt=246448265
I0415 16:50:53.171885 25415 traversal_worker_sum.cc:131] node_send_cnt=101,039,168 super_send_cnt=233,631,772 +=334670940
node_send_cnt=101039168= super_send_cnt=233631772
compute:28.6248 sec
compute:29.6407 sec(加了设置默认值)

delta-step:
I0417 13:37:29.369709  7564 traversal_worker_sum.cc:191] 
app convergence step=388 delta_sum=0 g_cnt=764,763,232 f_cnt=3,104,807
node_send_cnt=1,040,966 super_send_cnt=2,063,841 +=3104807
compute:12.2093 sec

delta-step: inner node, bound node
I0418 10:22:07.756012  5518 traversal_worker_sum.cc:209] 
app convergence step=388 threshold_change_cnt=19 g_cnt=763,472,451 f_cnt=2,953,091
node_send_cnt=1,040,966 super_send_cnt=1,912,125 +=2953091
compute:9.89653 sec

ordinary:
I0417 13:44:19.651692  7722 traversal_worker_sum.cc:193] 
app convergence step=215 delta_sum=0 g_cnt=437,706,241 f_cnt=9,731,577
node_send_cnt=3,240,743 super_send_cnt=6,490,834 +=9731577
compute:7.32159 sec

ordinary: inner node, bound node
I0418 10:01:57.659659  5172 traversal_worker_sum.cc:209] 
app convergence step=214 threshold_change_cnt=0 g_cnt=429,564,207 f_cnt=7,132,748
node_send_cnt=3,240,743 super_send_cnt=3,892,005 +=7132748
compute:6.46778 sec

---------------------------------------

I0416 09:47:06.073511  7940 traversal_worker.cc:161] app convergence step=556 delta_sum=0 
g_cnt=2,306,464,823 f_cnt=130,250,799
node_send_cnt=130250799
compute:27.2144 sec
compute:31.3448 sec(加了设置默认值)

delta-step:
I0417 13:40:15.126871  7645 traversal_worker.cc:203] 
app convergence step=572 delta_sum=0 g_cnt=1,124,136,859 f_cnt=2,760,388
node_send_cnt=2,760,388
compute:14.765 sec

ordinary:
I0417 13:52:29.370517  7957 traversal_worker.cc:203] 
app convergence step=556 delta_sum=0 g_cnt=1,092,824,427 f_cnt=2,760,388
node_send_cnt=2760388
compute:13.8689 sec


