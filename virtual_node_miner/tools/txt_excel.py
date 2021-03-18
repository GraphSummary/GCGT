import pandas as pd

path = '/home/yusong/code/grahp-summary/out/pr_delta_sum.txt'

df = pd.read_csv(path, sep=" ", header=None, index_col=0)
df = df.sort_values(by=0, ascending=True)  # 按age排列, ascending=False表示降序，为True为升序，默认为True

print(df)
df.to_excel(path[:-4] + '.xlsx')