'''
    处理数据集，将没有出度的点添加一条随机指向一个编号的点的边
'''
import sys
import time

def deal_edge(file_path):
    may_no_out = set() # 全部顶点
    have_out_edge = set() # 有出度的顶点
    # 读入文件
    print('read file...', file_path)
    # 先将原边写入文件，再将新边写入文件
    newpath = file_path[:file_path.rindex('.')]+'_new.e'
    print(newpath)
    print('write file...', newpath)
    f_w = open(newpath, 'w+', encoding='utf-8')
    f = open(file_path, 'r+', encoding='utf-8')
    origin = -1
    while True:
        line = f.readline()
        if line == None or len(line) < 2:
            break
        f_w.write(line)
        num_list = line.strip('\t').strip(' ').strip('\n').split(' ')
        if(len(num_list) != 2):
            print("line check: ", len(num_list), num_list)
            exit(0)
        u = int(num_list[0])
        v = int(num_list[1])
        if origin == -1:
            origin = u
        if u not in have_out_edge:
            have_out_edge.add(u) # 标记为有出边
        if v not in have_out_edge:
            may_no_out.add(v)
    f.close()

    if origin == -1:
        print("一条出边都没有...")
        exit(0)
    # 对于没有出边的则添加一条边
    # 新加边并写入：
    add_new_edge = 0
    for u in may_no_out:
        if u not in have_out_edge:
            # print("没有出度的id: ", u)
            # return
            f_w.write(str(u) + " " + str(origin) + "\n")
            add_new_edge += 1
    print("add_new_edge=", add_new_edge)
    f_w.close()

if __name__ == "__main__":
    # 运行命令： python3 ./tools/deal_dataset.py /home/yusong/dataset/web-uk-2002-all/web-uk-2002-all.e
    # 读入文件路径
    print('参数列表:', str(sys.argv))
    # file_path = input().strip(' ')
    file_path = sys.argv[1].strip(' ')
    print('start deal data...')
    time_start=time.time()
    deal_edge(file_path)   #  处理数据
    time_end=time.time()
    print('deal data finish!')  
    print('time cost',time_end-time_start,'s') 