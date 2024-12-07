#include <iostream>
#include <cmath>
#include "Task.h"
using namespace std;

Task::Task() {
    this->sm_locked = false;
    this->cpu_core = -1;
}

Task::Task(int id, int comp_time, int GPU_section_time, int sm_num) :
        id(id),
        wcet(comp_time),
        GPU_section_time(GPU_section_time),
        gpu_segment(vector<double>()),
        period(0),
        deadline(0),
        remaining_time(0),
        cpu_core(-1),
        sm_num(sm_num),
        sm_locked(false) {}

void Task::Reset() {
    this->remaining_time = this->wcet;
    this->deadline += this->period;
}

bool Task::get_SMs() {
    int executed_time = this->wcet - this->remaining_time;
    for (int i = 0; i < this->gpu_segment.size(); i += 2) {
        int down = round(this->gpu_segment[i] * this->wcet);
        if (executed_time == down) return true;
    }
    return false;
}

bool Task::release_SMs() {
    int executed_time = this->wcet - this->remaining_time;
    for (int i = 0; i < this->gpu_segment.size(); i += 2) {
        int down = round(this->gpu_segment[i] * this->wcet);
        int up = round(this->gpu_segment[i + 1] * this->GPU_section_time);
        if (executed_time == up) return true;
    }
    return false;
}

bool Task::is_on_cpu() {
    int executed_time = this->wcet - this->remaining_time;
    for (int i = 0; i < this->gpu_segment.size(); i += 2) {
        int down = round(this->gpu_segment[i] * this->wcet);
        int up = round(this->gpu_segment[i + 1] * this->GPU_section_time);
        if (executed_time >= down && executed_time <= up) {
            return false;
        }
    }
    return true;
}

void Task::update_task_info(int n) {
    this->wcet -= this->GPU_section_time;
    this->GPU_section_time = round((double)this->GPU_section_time / (double)n * this->sm_num);
    this->wcet += this->GPU_section_time;
    this->sm_num = n;
}