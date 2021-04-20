app=sssp

for filename in roadNet-TX roadNet-PA roadNet-CA
# for filename in roadNet-CA
do
    for i in 0 1
    do
        # for j in 5 10 20 30 40 999999
        for j in 999999
        do
# path=/home/yusong/dataset/roadNet-TX  # 原图边文件
path=/home/yusong/dataset/${filename}  # 原图边文件
# dataset=roadNet-TX  # 文件名
dataset=${filename} # 文件名
is_rand_weight=$i # 是否是随机加权(0/1)
delta_step_threshold=$j
# 文件
origin_e="${path}/${dataset}.e"
weight_e="${path}/${dataset}_weighted_${is_rand_weight}.e"
base_e="${path}/${dataset}_weighted_${is_rand_weight}_base.e"
result_analyse="traversal_result_analyse.txt" # 运行结果统计
# 写入文件
result="./out/${app}_result_analyse"
echo "app:$app" > "${result}"        # 截断文件后，追加文本
echo "path:$path" >> "${result}"
echo "dataset:$dataset" >> "${result}"
echo "is_rand_weight:$is_rand_weight" >> "${result}"
echo "delta_step_threshold:$delta_step_threshold" >> "${result}"

# 压缩图上计算
g++ ./worker/traversal_worker_sum.cc -lgflags -lglog && ./a.out -base_edge=${base_e} -shortestpath_source=0 -output=./out/sssp_result_sum -delta_step_threshold=${delta_step_threshold} -result_analyse=${result}
echo -e "summary graph computer finish!\n"

# 原图上计算
g++ ./worker/traversal_worker.cc -lgflags -lglog && ./a.out -base_edge=${base_e} -shortestpath_source=0 -output=./out/sssp_result -delta_step_threshold=${delta_step_threshold} -result_analyse=${result}
echo -e "origin graph computer finish!\n"

# 从log中提取数据

# 统计结果
python3 ./tools/result_analysis.py ${result} ${result}

# 比较计算结果：
python3 ./tools/compare_result.py ./out/sssp_result ./out/sssp_result_sum

# clean
rm *.out

echo $filename $i $j finish!
done
done
done