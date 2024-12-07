#include <iostream>
#include <queue>
#include <set>
#include <algorithm>
#include <vector>
#include "Scheduler.h"
using namespace std;

bool CompareDeadline (const Task &a, const Task &b) {
    return a.deadline < b.deadline;
}

typedef struct que_elem {
    int id;
    int core_id;
    int sm_num;
} que_elem;

/* 离散时间模拟 */
bool Scheduler::P_EDF(vector<vector<Task>> &partitioned_tasksets, int simulation_time, const int num_sms) {
    const int core_number = partitioned_tasksets.size();
    int sm_available = num_sms;
    queue<que_elem> waiting;
    set<int> mark;         // 记录队列状态
    for (int current_time = 0; current_time < simulation_time; current_time++) {
        for (int core_id = 0; core_id < core_number; core_id++) {
            auto& core_tasksets = partitioned_tasksets[core_id];
            if (core_tasksets.size() == 0) continue;

            // 根据 最小截止时间 排序
            sort(core_tasksets.begin(), core_tasksets.end(), CompareDeadline);

            // 确定在CPU上运行的任务 先选取第一个还没完成的任务
            int id = 0;
            bool idle = true;
            for (int i = 0; i < core_tasksets.size(); i++) {
                // 找第一个 开始时间 不超过当前时间的任务 作为调度的任务
                if (current_time >= core_tasksets[i].deadline - core_tasksets[i].period) {
                    id = i;
                    idle = false;
                    break;
                }
            }
            if (idle) continue;     // 没找到task 说明该核进入空闲状态
            for (int i = 0; i < core_tasksets.size(); i++) {  // 已经进入GPU执行的任务优先
                if (core_tasksets[i].sm_locked) {
                    id = i;
                    break;
                }
            }
            auto& task = core_tasksets[id];

            // 检验是否调度失败
            if ((current_time > task.deadline) || 
                (current_time == task.deadline && task.remaining_time != 0)) {
                cout << '\t' << current_time << " " << task.remaining_time << endl;
                return false;
            }

            // task execution & access SMs with FIFO queue
            if (task.remaining_time == 0) {
                // 完成执行
                task.Reset();
            } else {
                if (task.GPU_section_time == 0) { // 没有GPU片段
                    task.remaining_time--;
                    continue;
                }
                if (task.sm_locked) {
                    // 释放资源
                    if (task.release_SMs()) {
                        task.sm_locked = false;
                        sm_available += task.sm_num;
                    }
                    task.remaining_time--;
                } else {
                    if (task.is_on_cpu()) {                     // 如果在CPU上则正常运行
                        task.remaining_time--;
                    } else if (task.get_SMs()) {
                        if (!mark.empty() && mark.count(task.id) == 1) {   // 如果在队列中
                            // 如果是队头元素
                            if (!waiting.empty() && waiting.front().id == task.id 
                                && task.sm_num <= sm_available) {
                                waiting.pop();
                                mark.erase(task.id);
                                sm_available -= task.sm_num;
                                task.sm_locked = true;
                                task.remaining_time--;
                                /* 插队 */
                            }
                        } else {                                // 如果不在队列里 即刚来的
                            if (!waiting.empty() || task.sm_num > sm_available) {
                                waiting.push({task.id, core_id, task.sm_num});
                                mark.insert(task.id);
                            } else {
                                // 队列空 且资源够
                                sm_available -= task.sm_num;
                                task.sm_locked = true;
                                task.remaining_time--;
                            }
                        }
                    }
                }
            }
        }
        if (!waiting.empty() && waiting.front().sm_num <= sm_available) {
            que_elem temp = waiting.front();
            int i = 0;
            for (i = 0; i < partitioned_tasksets[temp.core_id].size(); i++) {
                if (partitioned_tasksets[temp.core_id][i].id == temp.id) break;
            }
            auto& task = partitioned_tasksets[temp.core_id][i];
            task.sm_locked = true;
            waiting.pop();
            mark.erase(task.id);
            sm_available -= task.sm_num;
            /* 插队 */
        }
    }
    return true;
}