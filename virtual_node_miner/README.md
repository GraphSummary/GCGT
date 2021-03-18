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
    输入边文件，
    输出两个文件：压缩后的新边文件(eq, u->v)，点文件(eq, u weight, weight表示u通过虚边可以到达的真实点个数)
2. pagerank计算：
    - 要求边数据集无重复边
    - 虚拟点初始化为0，其余点初始化为1-d
    - 
