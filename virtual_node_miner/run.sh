make clean
make
# edegpath=/home/yusong/code/dataset/grape/google_90w.e  # 原图边文件
path=/home/yusong/dataset/web-uk-2005  # 原图边文件
# path=/home/yusong/dataset/test  # 原图边文件
DATASETNAME=web-uk-2005   # 文件名
INC_RATE=0.05
# 预处理后文件
base_e="${path}/${DATASETNAME}_base.e"
update_e="${path}/${DATASETNAME}_update_${INC_RATE}.e"
updated_e="${path}/${DATASETNAME}_updated_${INC_RATE}.e"
# 压缩后文件
com_base_e="${path}/${DATASETNAME}_base_com.e"
com_base_v="${path}/${DATASETNAME}_base_com.v"
com_updated_e="${path}/${DATASETNAME}_updated_com.e"
com_updated_v="${path}/${DATASETNAME}_updated_com.v"
# 参数
CONVERGENCE_THRESHOLD=0.000001 # 收敛阈值
CLUSTER_THRESHOLD=50 # 压缩参数
VIRTUAL_THRESHOLD=20
# 写入文件
destdir="./out/result.txt"
echo "path:$path" > "${destdir}"        # 截断文件后，追加文本
echo "DATASETNAME:$DATASETNAME" >> "${destdir}"
echo "INC_RATE:$INC_RATE" >> "${destdir}"
echo "CONVERGENCE_THRESHOLD:$CONVERGENCE_THRESHOLD" >> "${destdir}"
echo "CLUSTER_THRESHOLD:$CLUSTER_THRESHOLD" >> "${destdir}"
echo "VIRTUAL_THRESHOLD:$VIRTUAL_THRESHOLD" >> "${destdir}"
# 压缩
./virtual_node_miner ${base_e} ${update_e} ${com_base_e} ${com_base_v} ${com_updated_e} ${com_updated_v} $CLUSTER_THRESHOLD $VIRTUAL_THRESHOLD
echo -e "压缩完成...\n"
# exit

# 压缩图计算
g++ delta_pagerank_sum.cpp
./a.out $CONVERGENCE_THRESHOLD ${com_base_e} ${com_base_v} ${com_updated_e} ${com_updated_v}
# ./a.out $CONVERGENCE_THRESHOLD ${com_updated_e} ${com_updated_v} ${com_updated_e} ${com_updated_v} # 全是修改后的
# ./a.out $CONVERGENCE_THRESHOLD ${com_base_e} ${com_base_v} ${com_base_e} ${com_base_v} # 全是修改前的
# ./a.out 0.000001 /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.v  /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.v
echo -e "压缩图计算完成...\n"

# 原图计算
g++ delta_pagerank_.cpp 
./a.out $CONVERGENCE_THRESHOLD ${base_e} ${com_base_v} ${updated_e} ${com_updated_v}
# ./a.out $CONVERGENCE_THRESHOLD ${updated_e} ${com_updated_v} ${updated_e} ${com_updated_v} # 全是修改后的
# ./a.out $CONVERGENCE_THRESHOLD ${base_e} ${com_base_v} ${base_e} ${com_base_v}  # 全是修改前的
# ./a.out 0.000001 /home/yusong/dataset/web-uk-2005/web-uk-2005_updated.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.v  /home/yusong/dataset/web-uk-2005/web-uk-2005_updated.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.v
# ./a.out 0.000001 /home/yusong/dataset/web-uk-2005/web-uk-2005_base.e /home/yusong/dataset/web-uk-2005/web-uk-2005_base_com.v  /home/yusong/dataset/web-uk-2005/web-uk-2005_updated.e /home/yusong/dataset/web-uk-2005/web-uk-2005_updated_com.v
echo -e "原图计算完成...\n"

# 比较两种计算结果
/usr/bin/python3 ./tools/compare_result.py ./out/pr_delta_pre.txt ./out/pr_delta_sum_com.txt
echo -e "计算结果比较完成...\n"

# 写入excel
/usr/bin/python3 ./tools/result_analysis.py
echo -e "统计结果写入excel...\n"

# # clean
# rm *.out