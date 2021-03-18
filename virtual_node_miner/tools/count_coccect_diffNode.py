'''
    统计校正集中涉及到的不同点的个数
'''

def readFile(path):
    with open(path, 'r') as f:
        return f.read()

path = "/home/yusong/code/grahp-summary/out/google_90w_s0_e1.2_correction.e"
file = readFile(path)

node = set()
for line in file.split('\n'):
    if len(line) < 2:
        break
    f, x, y = line.split(' ')
    node.add(int(x))
    node.add(int(y))

print(len(node))
nodes = list(node)
nodes.sort()
print(nodes[:100])