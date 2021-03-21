make clean && make
# edegpath=/home/yusong/code/dataset/grape/google_90w.e # 原图边文件
edegpath=/home/yusong/dataset/web-uk-2002-all/web-uk-2002-all_new.e # 原图边文件
name=google_90w # 生成的文件名
threshold=0.000001 # 收敛阈值
CLUSTER_THRESHOLD=50 # 压缩参数
VIRTUAL_THRESHOLD=10
# 写入文件
destdir="./out/result.txt"
echo "edgefile:$edegpath" > "${destdir}"        # 截断文件后，追加文本
echo "threshold:$threshold" >> "${destdir}"
echo "name:$name" >> "${destdir}"
echo "CLUSTER_THRESHOLD:$CLUSTER_THRESHOLD" >> "${destdir}"
echo "VIRTUAL_THRESHOLD:$VIRTUAL_THRESHOLD" >> "${destdir}"
# 压缩
./virtual_node_miner  ${edegpath} ./out/${name}.e ./out/${name}.v $CLUSTER_THRESHOLD $VIRTUAL_THRESHOLD
# 压缩图计算
g++ delta_pagerank_sum.cpp
./a.out $threshold ${edegpath} ${name}
echo -e "压缩图计算完成...\n"
# 原图计算
g++ delta_pagerank_.cpp 
./a.out $edegpath $threshold
echo -e "原图计算完成...\n"

# 比较两种计算结果
/usr/bin/python3 /home/yusong/code/GCGT/virtual_node_miner/tools/compare_result_pagerank.py
echo -e "计算结果比较完成...\n"

# 写入excel
/usr/bin/python3 /home/yusong/code/GCGT/virtual_node_miner/tools/result_analysis.py
echo -e "统计结果写入excel...\n"

# # clean
# rm *.out