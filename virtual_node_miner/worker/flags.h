#ifndef WORKER_FLAGS_H_
#define WORKER_FLAGS_H_

DEFINE_double(convergence_threshold, 0.000001, "iterate convergence threshold");
DEFINE_double(delta_step_threshold, 999999999, "delta step threshould");
DEFINE_string(base_edge, "", "old edge file");
DEFINE_string(output, "", "result output file");
DEFINE_string(result_analyse, "", "result analyse file");
DEFINE_uint32(max_iterater_num, 1000, "max_iterater_num");
DEFINE_int64(sssp_source, -1, "source");
DEFINE_int64(php_source, -1, "source");
DEFINE_string(app, "", "application name");

#endif  // WORKER_FLAGS_H_