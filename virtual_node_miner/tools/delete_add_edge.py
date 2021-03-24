'''
    对数据集进行增删边操作，并保证不会将唯一一条出边删除
'''
import sys
import time
import random

def deal_edge(file_path, chage_rate):
    update_path = file_path[:file_path.rindex('_base')]+'_update.e'  # 增删的边
    updated_path = file_path[:file_path.rindex('_base')]+'_updated.e' # 增删之后的边
    print('read file...', file_path)
    print('write file...', update_path)
    print('write file...', updated_path)
    f_w_update = open(update_path, 'w+', encoding='utf-8')
    f_w_updated = open(updated_path, 'w+', encoding='utf-8')
    f_r = open(file_path, 'r+', encoding='utf-8')
    have_out_edge = set() # 有出度的顶点
    all_num = 0
    add_num = 0
    del_num = 0
    while True:
        line = f_r.readline()
        if line == None or len(line) < 2:
            break
        all_num += 1
        num_list = line.strip('\t').strip(' ').strip('\n').split(' ')
        if(len(num_list) != 2):
            print("line check: ", len(num_list), num_list)
            exit(0)
        u = int(num_list[0])
        v = int(num_list[1])
        if u not in have_out_edge:
            have_out_edge.add(u) # 标记为有出边
            f_w_updated.write(line)
        else:
            p = random.random()
            if p < chage_rate:
                is_add = random.randint(0,1)
                if is_add == 0: # delete this edge
                    f_w_update.write('d ' + line) # 记录变化
                    del_num += 1
                else:
                    if len(have_out_edge) >= 2:
                        u, v = random.sample(have_out_edge, 2)
                        f_w_update.write('a ' + str(u) + ' ' + str(v) + '\n') # 记录变化
                        f_w_updated.write(str(u) + ' ' + str(v) + '\n') # 加入的边
                        add_num += 1
                    f_w_updated.write(line) # 此边不删除也要放入
            else:
                f_w_updated.write(line)
    print('all_num=%d add_num=%d del_num=%d update=%d rate=%f' % (all_num, add_num, del_num, add_num+del_num, (add_num+del_num)/all_num * 1.0))

    f_r.close()
    f_w_update.close()
    f_w_updated.close()

if __name__ == "__main__":
    # 运行命令： python3 ./tools/delete_add_edge.py /home/yusong/dataset/web-uk-2002-all/web-uk-2002-all_base.e 0.01
    # 读入文件路径
    print('参数列表:', str(sys.argv))
    # file_path = input().strip(' ')
    file_path = sys.argv[1].strip(' ')
    chage_rate = float(sys.argv[2].strip(' '))
    print('start deal data...')
    time_start=time.time()
    deal_edge(file_path, chage_rate)   #  处理数据
    time_end=time.time()
    print('delete_add_edge finish!')  
    print('time cost',time_end-time_start,'s')