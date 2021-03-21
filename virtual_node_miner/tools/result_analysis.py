#-*- coding: UTF-8 -*-   
'''
    用于解析运行结果: 通过result.txt文件提取本次运行结果，然后通过读取上次保存的csv
    文件，将内容合并，并生成excel和csv文件
    
'''
import re
import pandas as pd

def readFile(path):
    with open(path, 'r+', encoding="utf-8") as f:
        return f.read()

def first(filestr, savepath):
    data_map = {}
    for data in filestr.split('\n'):
        if len(data) <= 1:
            break
        data_map[data.split(':')[0]] = [data.split(':')[1]]
    # print(data_map) 
    df = pd.DataFrame(data_map, index=['']).T
    df.columns = [data_map['name'][0]+"_"+data_map['threshold'][0]]
    print(df.head)
    df.to_excel(savepath + '.xlsx', index=False)
    df.to_csv(savepath + '.csv')

def mergeFile(filestr, savepath):
    data_map = {}
    for data in filestr.split('\n'):
        if len(data) <= 1:
            break
        data_map[data.split(':')[0]] = data.split(':')[1]
    # 读文件：
    try:
        df = pd.read_csv(savepath + '.csv', index_col=0)
    except Exception as f:
        print(f)
        print("打开失败，重新构建新文件...")
        df = pd.DataFrame(data_map, index=['']).T
        df.columns = [data_map['name'][0]+"_"+data_map['threshold'][0]]
    # df.columns = [data_map['edgefile'][0]+"_"+data_map['s_threshold'][0]]
    # newcolumn = data_map['name']+"_"+data_map['threshold']
    newcolumn = len(df.columns)
    # print("新列：", newcolumn, )
    for key, value in data_map.items():
        # print(key, value)
        df.loc[key, newcolumn] = value
    # print(df.head)
    df.to_excel(savepath + '.xlsx')
    df.to_csv(savepath + '.csv')

if __name__ == "__main__":
    filepath = '././out/result.txt'
    savepath = '././out/result_analyse'
    f = readFile(filepath)
    # 仅仅第一次使用
    # first(f, savepath)

    # 合并文件
    mergeFile(f, savepath)
    
    # print(df)
    print("finish!")
