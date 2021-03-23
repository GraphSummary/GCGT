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
