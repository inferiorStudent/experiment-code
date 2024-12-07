#include <unistd.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include "System.h"
#include "Task.h"
using namespace std;

System::System() {
    this->MAX_CORE_NUMBER = sysconf(_SC_NPROCESSORS_ONLN);
    this->MAX_SM_NUMBER = 14;
}

vector<vector<Task>> System::partitionTasks(vector<Task> &taskset, int num_cores, int num_sms) {
    vector<double> utilization(num_cores, 0);
    vector<vector<Task>> partitioned_tasksets(num_cores, vector<Task>());
    double u_sys = 0;
    set<int> unmapped_tasks;
    for (int i = 0; i < taskset.size(); i++) unmapped_tasks.insert(taskset[i].id);
    while (!unmapped_tasks.empty()) {
        // choose task
        double ux = 0;
        int mapped = -1;
        for (auto x : unmapped_tasks) {
            double temp = (taskset[x].wcet + bwEstimate(taskset[x], partitioned_tasksets, num_sms)) / taskset[x].period;
            if (temp > ux) {
                ux = temp;
                mapped = x;
            }
        }
        if (ux > 1 || mapped == -1) return vector<vector<Task>>();
        // choose core
        int m = -1;
        double m_delta = 1;
        for (int i = 0; i < utilization.size(); i++) {
            utilization[i] += ux;
            double u_sys_temp = 0;
            for (int j = 0; j < utilization.size(); j++) {
                u_sys_temp = max(u_sys_temp, utilization[j]);
            }
            double temp_delta = u_sys_temp - u_sys;
            if (temp_delta < m_delta) {
                m_delta = temp_delta;
                m = i;
            }
            utilization[i] -= ux;
        }
        if (m == -1) return vector<vector<Task>>();
        // cout << m << " " << mapped << " " << ux << " " << taskset[mapped].sm_num << endl;
        // update utilization & partition set for every core
        utilization[m] += ux;
        taskset[mapped].cpu_core = m;
        partitioned_tasksets[m].push_back(taskset[mapped]);
        unmapped_tasks.erase(mapped);
        for (int i = 0; i < utilization.size(); i++) {
            u_sys = max(u_sys, utilization[i]);
        }
        if (u_sys > 1) return vector<vector<Task>>();
    }
    return partitioned_tasksets;
}

bool exist_task(const int num_sms, vector<Task> &taskset) {
    for (int i = 0; i < taskset.size(); i++) {
        if (taskset[i].sm_num < num_sms) return true;
    }
    return false;
}

vector<vector<Task>> System::smAlocation(vector<Task> &taskset, int num_cores, int num_sms) {
    for (int i = 0; i < taskset.size(); i++) {
        int num = 1;
        taskset[i].update_task_info(num++);
        while ((double)taskset[i].wcet / taskset[i].period > 1.0) {
            if (num > 14) {
                return vector<vector<Task>>();
            }
            taskset[i].update_task_info(num++);}
    }
    vector<vector<Task>> partitioned_tasksets = this->partitionTasks(taskset, num_cores, num_sms);
    while (partitioned_tasksets.size() == 0 || !this->is_schedulable(partitioned_tasksets)) {
        // choose task
        int x = -1;
        double delta = 100000;
        for (int i = 0; i < taskset.size(); i++) {
            if (taskset[i].GPU_section_time == 0 || taskset[i].sm_num >= num_sms) continue;
            taskset[i].update_task_info(taskset[i].sm_num + 1);
            double u1 = taskset[i].wcet + bwEstimate(taskset[i], partitioned_tasksets, num_cores);
            taskset[i].update_task_info(taskset[i].sm_num - 1);
            double u2 = taskset[i].wcet + bwEstimate(taskset[i], partitioned_tasksets, num_cores);
            double temp_delta = (u1 - u2) / taskset[i].period;

            if (delta > temp_delta) {
                delta = temp_delta;
                x = i;
            }
        }
        if (x == -1) return vector<vector<Task>>();
        taskset[x].update_task_info(taskset[x].sm_num + 1);
        if (!exist_task(num_sms, taskset)) return vector<vector<Task>>();
        partitioned_tasksets = this->partitionTasks(taskset, num_cores, num_sms);
    }
    return partitioned_tasksets;
}


bool cmp(const vector<int> &a, const vector<int> &b) {
    return a[1] > b[1];
}

double local_est(vector<vector<Task>> &partitioned_tasksets, int num_sms) {
    double local_bw = 0;
    vector<vector<int>> worst_sec(partitioned_tasksets.size(), vector<int>(2, 0));
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        int temp_sm = 0, index = -1;
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            if (partitioned_tasksets[i][j].GPU_section_time == 0) continue;
            if (temp_sm < partitioned_tasksets[i][j].sm_num) {
                temp_sm = partitioned_tasksets[i][j].sm_num;
                index = j;
            }
        }
        if (index == -1) continue;
        int temp_sec_wcet = 0, sec_wcet;
        for (int k = 0; k < partitioned_tasksets[i][index].gpu_segment.size(); k += 2) {
            sec_wcet = partitioned_tasksets[i][index].gpu_segment[k + 1] - 
                partitioned_tasksets[i][index].gpu_segment[k];
            if (temp_sec_wcet < sec_wcet) {
                temp_sec_wcet = sec_wcet;
            }
        }
        worst_sec[i][1] = temp_sm;
        worst_sec[i][0] = temp_sec_wcet;
    }
    sort(worst_sec.begin(), worst_sec.end(), cmp);
    int sm_available = num_sms, temp_sec_wcet = 0;
    for (int i = 0; i < worst_sec.size(); i++) {
        if (sm_available >= worst_sec[i][1]) {
            sm_available -= worst_sec[i][1];
            temp_sec_wcet = worst_sec[i][0];
        } else {
            sm_available = num_sms;
            local_bw += temp_sec_wcet;
        }
    }
    return local_bw;
}

double System::bwEstimate(Task &task, vector<vector<Task>> &partitioned_tasksets, int num_sms) {
    double BWest = 0;
    vector<vector<int>> worst_task(partitioned_tasksets.size(), vector<int>(3, 0));
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            if (worst_task[i][1] < partitioned_tasksets[i][j].sm_num) {
                worst_task[i][0] = partitioned_tasksets[i][j].GPU_section_time;
                worst_task[i][1] = partitioned_tasksets[i][j].sm_num;
            }
            // worst_task[i][0] = max(worst_task[i][0], partitioned_tasksets[i][j].GPU_section_time);
            // worst_task[i][1] = max(worst_task[i][1], partitioned_tasksets[i][j].sm_num);
        }
    }
    double factor = 0;
    for (int i = 0; i < worst_task.size(); i++) factor += worst_task[i][1];
    factor /= num_sms;
    sort(worst_task.begin(), worst_task.end(), cmp);
    int temp_gpu_wcet = 0, sm_available = num_sms;
    for (int i = 0; i < worst_task.size(); i++) {
        if (sm_available >= worst_task[i][1]) {
            sm_available -= worst_task[i][1];
            temp_gpu_wcet = max(temp_gpu_wcet, worst_task[i][0]);
        } else {
            BWest += temp_gpu_wcet;
            temp_gpu_wcet = 0;
            sm_available = num_sms;
        }
    }
    return factor ? BWest / factor : BWest;
}

bool cmp_priority(const Task &a, const Task &b) {
    return a.period > b.period;
}

bool System::is_schedulable(vector<vector<Task>> &partitioned_tasksets) {
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        // priority sorts from the smallest to biggest
        sort(partitioned_tasksets[i].begin(), partitioned_tasksets[i].end(), cmp_priority);
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            // find the task with the max gpu execution time in core-i, whose priority <= task-j
            int max_gpu_sec_wcet = 0;
            double u = 0;
            for (int k = 0; k < j; k++) {
                // global waiting time
                u += (partitioned_tasksets[i][k].wcet + bwEstimate(partitioned_tasksets[i][k], 
                    partitioned_tasksets, this->MAX_SM_NUMBER)) / partitioned_tasksets[i][k].period;
                for (int l = 0; l < partitioned_tasksets[i][k].gpu_segment.size(); l += 2) {
                    int temp = partitioned_tasksets[i][k].gpu_segment[l + 1] - 
                        partitioned_tasksets[i][k].gpu_segment[l];
                    if (max_gpu_sec_wcet < temp) {
                        max_gpu_sec_wcet = temp;
                    }
                }
                
            }
            // local waiting time
            double B = max_gpu_sec_wcet + local_est(partitioned_tasksets, this->MAX_SM_NUMBER);
            u += B / partitioned_tasksets[i][j].period;
            if (u > 1) return false;
        }
    }
    return true;
}