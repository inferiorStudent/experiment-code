#include <iostream>
#include <vector>
#include "Task.h"
#include "Scheduler.h"
#include "System.h"
using namespace std;

#define MAX_SIMULATION_TIME 200000

void display(vector<vector<Task>> &partitioned_tasksets) {
    for (int i = 0; i < partitioned_tasksets.size(); i++) {
        cout << "core " << i << ": ";
        for (int j = 0; j < partitioned_tasksets[i].size(); j++) {
            cout << partitioned_tasksets[i][j].id << "[" << partitioned_tasksets[i][j].period
            << ", " << partitioned_tasksets[i][j].sm_num << ", " << partitioned_tasksets[i][j].wcet
            << ", " << partitioned_tasksets[i][j].GPU_section_time << "]\t";
        }
        cout << endl;
    }
    cout << "==========================" << endl;
}

int main() {
    System sys;
    Scheduler sche;
    int task_num, schedulable = 0, partition = 0;
    int cnt = 1;
    while (cin >> task_num) {

        /* taskset initialization */

        vector<Task> taskset;
        int sec_num;
        double gpu_ratio, sec_ratio;
        for (int i = 0; i < task_num; i++) {
            Task task;
            cin >> task.id >> task.wcet >> gpu_ratio >> task.period >> sec_num;
            for (int j = 0; j < sec_num; j++) {
                cin >> sec_ratio;
                task.gpu_segment.push_back(sec_ratio);
            }
            for (int j = 0; j < sec_num; j += 2) {
                task.gpu_segment[j + 1] += task.gpu_segment[j];
            }
            if (sec_num != 0) task.GPU_section_time = gpu_ratio * task.wcet;
            task.remaining_time = task.wcet;
            task.deadline = task.period;
            task.sm_num = sys.MAX_SM_NUMBER;

            taskset.push_back(task);
        }
        vector<vector<Task>> partitioned_tasksets;
        int i = 0;
        for (i = sys.MAX_CORE_NUMBER; i > 6; i--) {
            partitioned_tasksets = sys.smAlocation(taskset, i, sys.MAX_SM_NUMBER);
            /* feasibility test */
            if (partitioned_tasksets.size() != 0) {
                partition++;
                break;

                // display the partitioned result
                // display(partitioned_tasksets);
                // break;
            }
        }
        if (partitioned_tasksets.size() != 0) {
            /* scheduling simulation */
            if (sche.P_EDF(partitioned_tasksets, MAX_SIMULATION_TIME, sys.MAX_SM_NUMBER)) {
                schedulable++;
                cout << cnt << ": success" << endl;
            } else {
                cout << cnt << ": fail" << endl;
                // display(partitioned_tasksets);
            }
        } else {
            cout << cnt << ": x" << endl;
        }
        cnt++;
    }
    cout << "result: " << schedulable << "/" << partition << endl;
    return 0;
}