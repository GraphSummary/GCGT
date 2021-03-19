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
    - 输出两个文件：压缩后的新边文件(eq, u->v)，点文件(eq, u weight, weight表示u通过虚边可以到达的真实点个数)
    - 其中可以指定压缩次数，压缩次数增多在一定情况下可以增加压缩率，但是原始的代码多次压缩会产生虚拟点指向虚拟点的情况，就是在后面压缩时可能会把前面产生的虚拟点也进行压缩了。这种情况将会增加Pagerank迭代次数，故修改代码(通过不将虚拟点加入前缀树来实现)，限制参与压缩的点的范围为真实点。
2. pagerank计算：
    - 要求边数据集无重复边
    - 虚拟点初始化为0，其余点初始化为1-d
    - 跌代过程的顺序为：真实点发消息，虚拟点收消息，虚拟点发消息，真实点接收消息。
