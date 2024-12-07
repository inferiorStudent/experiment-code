# Simulation: real-time scheduling with GPU resource contention

1. This is the experiment code of my unfinished *graduation thesis* regarding to real time scheduling with partitioned-EDF and SM-based allocation algorithm(SMAA). Algorithm for comparison, STGM-busy, WFD-NA, WFD-SMA and RA-NA will be coded in near future, no later than the last day of this year!
2. Under the directory `${pwd}/`, use the command `make` to compile the project and get the executive file `main`.
3. File `${pwd}/tasksets/test.cpp` is just for code testing, nothing for the whole project.
4. Change some parameters (utilization `U` or the macro MAX_TASKSET_NUMBER) in file `${pwd}/tasksets/uunifast.cpp` and generate the tasksets. Input the double type *utilization* then write into the file "U" whose name reserves a decimal fraction. The binary executable file needs the input from "U" by using command `./main < 1000taskset/6.0-2`, taking "1000taskset/6.0-2" as an example.
5. Three sections "generate tasksets", "allocation & partitioning (feasibility test)" and "scheduling simulation" in SMAA have all been finished which has been tested correctedly with 1000 tasksets in Dec. 7th, 2024.

# range of parameters for generation
|param|range|
|--|--|
|Number of CPU cores|    2~8|
|Number of tasks|        2 *core_number-2 ~ 4 *core_number|
|Number of GPU segments| 0~3|
|Task Period|            100~500|
|G_i / WCET|             0.1~0.2|

last updating time: Dec. 7th, 2024
