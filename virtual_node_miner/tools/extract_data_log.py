'''
    从log中提取需要统计的数据并写入指定文件
'''
import sys

if __name__ == "__main__":
    # 运行命令： python3 ./log result.txt
    # filepath = './log'
    filepath = sys.argv[1].strip(' ')
    # 读取文件
    f = open(filepath, 'r+', encoding="utf-8")

    # 解析数据
    data = {'total': {}}
    for line in f:
        # line = "[W2] -- received.MTYPE_PULL_REQUEST : 481.00"
        matchObj = re.match( r'\[(.*)\] -- ([a-zA-Z._]+) : ([0-9.]+)', line)
        if matchObj:
            # print(matchObj.group())
            if matchObj.group(1) not in data:
                data[matchObj.group(1)] = {}
            data[matchObj.group(1)][matchObj.group(2)] = float(matchObj.group(3))
            if matchObj.group(2) not in data['total']:
                data['total'][matchObj.group(2)] = float(matchObj.group(3))
            else:
                data['total'][matchObj.group(2)] += float(matchObj.group(3))
        total_time = re.match(r'.*\] Total runtime: ([0-9.]+)', line)
        if total_time:
            print('total_time: ', total_time.group(1))
        worker_time = re.match(r'^ 0: (.*)', line)
        if worker_time:
            print('worker_time: ', worker_time.group(1))
    f.close()
    # print(data['total'])

