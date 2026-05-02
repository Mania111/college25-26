#ifndef TASK_HPP
#define TASK_HPP

/* =================================================== task.hpp ======================================================

    Defines basic data types

    1. Task (id, priority) - represents one job/task
    2. ScheduleEntry (taskId, time, if interrupted) - represents one output entry in the schedule
    3. MachineSchedule (manual vector - stores all schedule entries for one machine)
    4. ScheduleResult (Cmax, sigmaC and schedules for all machines) - stores the final result of one scheduling operation

======================================================================================================================*/

struct Task {
    int id;
    double p;
};

struct ScheduleEntry {
    int taskId;
    double time;
    bool interrupted; // false=C[id], true=P[id]
};

struct MachineSchedule {
    ScheduleEntry* data;
    int size;
    int capacity;
};

struct ScheduleResult {
    double cmax;
    double sigmaC;

    MachineSchedule* machines;
    int machineCount;
};

#endif
