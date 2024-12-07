#ifndef SYSTEM_H
#define SYSTEM_H

#include "Task.h"

class System {
public:
    int MAX_SM_NUMBER;
    int MAX_CORE_NUMBER;

    System();
    
    vector<vector<Task>> partitionTasks(vector<Task> &taskset, int num_cores, int num_sms);
    vector<vector<Task>> smAlocation(vector<Task> &taskset, int num_cores, int num_sms);
    double bwEstimate(Task &task, vector<vector<Task>> &partitioned_tasksets, int num_sms);
    bool is_schedulable(vector<vector<Task>> &partitioned_tasksets);
};

#endif