import os
import pandas as pd

    

if __name__ == "__main__":
    # path = r".\out\pr_delta_pre.txt"
    path1 = r"././out/pr_delta_pre.txt"
    df1 = pd.read_csv(path1, header=None, sep=' ')
    df1 = df1.sort_values(by=0, ascending=True)  # 按age排列, ascending=False表示降序，为True为升序，默认为True
    print(df1.head())
    # print(df1.tail())
    print(df1.shape)

    path2 = r"././out/pr_delta_sum_com.txt"
    df2 = pd.read_csv(path2, header=None, sep=' ')
    df2 = df2.sort_values(by=0, ascending=True)  # 按age排列, ascending=False表示降序，为True为升序，默认为True
    print(df2.head())
    # print(df2.tail())
    print(df2.shape)

    if(df1.shape != df2.shape):
        print("shapt not same...", df1.shape, df2.shape)

    sp1 = df1.values
    sp2 = df2.values
    wc = 0.0
    sum1 = 0.0
    sum2 = 0.0
    wc_cnt = 0
    # print(sp1[0][1], sp2[0][1], sp2[0][1]/newdf1.shape[0])
    for i in range(df1.shape[0]):
        a, b = sp1[i][1], sp2[i][1]
        wc += abs(a - b)
        sum1 += a
        sum2 += b
        if(abs(a - b) > 1e-6):
            wc_cnt += 1
        
    print('pre和sum误差: ', wc)
    print('误差点的个数：', wc_cnt)
    print('pr_delta_pre sum1 =', sum1)
    print('pr_delta_sum sum2 =', sum2)
    print(path1) 
    print(path2) 

    

