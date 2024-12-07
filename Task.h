#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <vector>
using namespace std;

class Task {
public:
    int id;
    int wcet;                  // 任务总执行时间
    int GPU_section_time;      // GPU总访问时间
    vector<double> gpu_segment;// 片段执行时间ratio 2k: 开始时间占period百分比  2k+1: GPU时间百分比
    int period;                // 周期
    int deadline;              // 截止时间
    int remaining_time;        // 剩余执行时间
    int cpu_core;              // 所属核(后面没用到)
    int sm_num;                // 分配的SM个数
    bool sm_locked;            // 获取SM资源

    Task();
    Task(int id, int comp_time, int GPU_section_time, int sm_num);
    void Reset();
    bool get_SMs();            // 准备获取SM
    bool release_SMs();        // 准备释放SM
    bool is_on_cpu();          // 是否在CPU上执行
    void update_task_info(int n);
};

#endif