import networkx as nx
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

plt.figure(figsize=(10,5))
G = nx.DiGraph()
# nodes = ['0', '1', '2', '3']
nodes = set()
edge_path = '/home/yusong/dataset/test/a_updated.e'
f = open(edge_path, 'r+', encoding='utf-8')
while True:
    line = f.readline()
    if line == None or len(line) < 3:
        break
    x, y = line.strip('\n').strip(' ').split(' ')
    nodes.add(x)
    nodes.add(y)
    G.add_edge(x, y)

# G.add_nodes_from(nodes)    #加点集合
# G.add_edge('0' ,'2' )   #一次添加一条边
# G.add_edge('0' ,'1' )   #一次添加一条边
# G.add_edge('0' ,'3' )   #一次添加一条边
# G.add_edge('1' ,'0' )   #一次添加一条边
# G.add_edge('1' ,'3' )   #一次添加一条边
# G.add_edge('2' ,'0' )   #一次添加一条边
# G.add_edge('3' ,'1' )   #一次添加一条边
# G.add_edge('3' ,'2' )   #一次添加一条边

# 画图
# nx.draw(G, with_labels=True , node_size = 2000)
# plt.show()

pr = nx.pagerank(G, alpha=0.85, personalization=None,
             max_iter=100, tol=1.0e-6, nstart=None, weight='weight',
             dangling=None)
# print(pr)
for k, v in pr.items():
    print(k, v * len(nodes))
