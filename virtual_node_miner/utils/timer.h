/**
 * NOLINT(legal/copyright)
 *
 * The file examples/analytical_apps/timer.h is referred and derived from project
 * atlarge-research/graphalytics-platforms-powergraph,
 *
 *    https://github.com/atlarge-research/graphalytics-platforms-powergraph/
 * blob/master/src/main/c/utils.hpp
 *
 * which has the following license:
 *
 *  Copyright 2015 Delft University of Technology
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef EXAMPLES_ANALYTICAL_APPS_TIMER_H_
#define EXAMPLES_ANALYTICAL_APPS_TIMER_H_

#include <stddef.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>

/**
 * Timers for LDBC benchmarking, referred and derived from project
 * atlarge-research/graphalytics-platforms-powergraph.
 */
static double timer() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static bool timer_enabled;
static std::vector<std::pair<std::string, double>> timers;

static void timer_start(bool enabled = true) {
  timers.clear();
  timer_enabled = enabled;
}

static void timer_next(const std::string& name) {
  if (timer_enabled) {
    timers.emplace_back(std::make_pair(name, timer()));
  }
}

static void timer_end(bool iswrite = false, std::string appname="", std::string resultPath = "./out/result.txt") {
  if (timer_enabled) {
    timer_next("end");

    std::cout << "Timing results:" << std::endl;
    for (size_t i = 0; i < timers.size() - 1; i++) {
      std::string& name = timers[i].first;
      double time = timers[i + 1].second - timers[i].second;

      std::cout << appname << " - " << name << ":" << time << " sec" << std::endl;
    }
    if(timers.size() >= 1){
      std::cout << appname << " all time: " << timers[timers.size() - 1].second - timers[0].second << " sec" << std::endl;
    }
    if (iswrite){
        // 将运行时间写入文件
        // std::string resultPath = "./out/result.txt";
        std::cout << "时间写入文件：" << resultPath << std::endl;
        std::ofstream fout(resultPath, std::ios::app);
        for (size_t i = 0; i < timers.size() - 1; i++) {
          std::string& name = timers[i].first;
          double time = timers[i + 1].second - timers[i].second;

          fout << appname << "_" << name << ":" << time << " sec" << std::endl;
        }
        if(timers.size() >= 1){
          fout << appname << "_all time:" << timers[timers.size() - 1].second - timers[0].second << " sec" << std::endl;
        }
        fout.close();
    }
    timers.clear();
  }
}

#endif  // EXAMPLES_ANALYTICAL_APPS_TIMER_H_
