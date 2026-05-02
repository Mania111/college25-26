#include "schedule.hpp"
#include <iostream>

/* =================================================== schedule.cpp ======================================================

    Implements all scheduling algorithms and schedule otput logic

    What it does:
    1. Build schedules for multiple machines
    2. Calculate Cmax & sigmaC
    3. Store schedule entries using manual dynamic arrays
    4. Print results
    5. Free memory

    Algorithms:

    B - basic list scheduling
        - tasks are processed in current list order
        - each task is assigned to the first available machine

    L - LPT scheduling
        - tasks sorted by decreasing processing time
        - bigger task id goes first in case of tie
        - then uses basic list scheduling

    S - SPT variant
        - task sorted by LPT order first
        - tasks distributed round-robin to machines
        - each machine executes its tasks in SPT order

    M - mcnaughton algorithm
        - preemptive scheduling
        - computes optimal Cmax as max( sum/m, longest task )
        - fills machines up to Cmax
        - splits tasks if they dont fit

    A - brute force optimal Cmax
        - tries every assignment of tasks to machines
        - used only for very small inputs

    -> no STL containers
    -> sortTasks() is a custom insertion sort
    -> MachineSchedule is used as manual dynamic array

======================================================================================================================*/

// Dynamic array for one machine.

static void initMachine(MachineSchedule& machine) {
    machine.size = 0;
    machine.capacity = 4;
    machine.data = new ScheduleEntry[machine.capacity];
}

static void addEntry(MachineSchedule& machine, int taskId, double time, bool interrupted) {
    if (machine.size == machine.capacity) {
        int newCapacity = machine.capacity * 2;
        ScheduleEntry* newData = new ScheduleEntry[newCapacity];

        for (int i = 0; i < machine.size; i++) {
            newData[i] = machine.data[i];
        }

        delete[] machine.data;

        machine.data = newData;
        machine.capacity = newCapacity;
    }

    machine.data[machine.size].taskId = taskId;
    machine.data[machine.size].time = time;
    machine.data[machine.size].interrupted = interrupted;

    machine.size++;
}

// Creates empty schedule result.

static ScheduleResult createResult(int m) {
    ScheduleResult result;

    result.cmax = 0.0;
    result.sigmaC = 0.0;
    result.machineCount = m;
    result.machines = new MachineSchedule[m];

    for (int i = 0; i < m; i++) {
        initMachine(result.machines[i]);
    }

    return result;
}

// Copies one task array to another.

static void copyTasks(Task* source, Task* target, int n) {
    for (int i = 0; i < n; i++) {
        target[i] = source[i];
    }
}

// LPT order: longest first. If equal, larger id first.

static bool comesBeforeLPT(const Task& a, const Task& b) {
    if (a.p > b.p) return true;
    if (a.p < b.p) return false;
    return a.id > b.id;
}

// SPT order: shortest first. If equal, smaller id first.

static bool comesBeforeSPT(const Task& a, const Task& b) {
    if (a.p < b.p) return true;
    if (a.p > b.p) return false;
    return a.id < b.id;
}

// Own sorting function: insertion sort.

static void sortTasks(Task* tasks, int n, bool (*cmp)(const Task&, const Task&)) {
    for (int i = 1; i < n; i++) {
        Task key = tasks[i];
        int j = i - 1;

        while (j >= 0 && cmp(key, tasks[j])) {
            tasks[j + 1] = tasks[j];
            j--;
        }

        tasks[j + 1] = key;
    }
}

// Public helper used by main cache.

void sortLPT(Task* tasks, int n) {
    sortTasks(tasks, n, comesBeforeLPT);
}

// Finds processor with smallest current time.
// If equal, first processor remains selected.

static int findEarliestMachine(double* machineTime, int m) {
    int best = 0;

    for (int i = 1; i < m; i++) {
        if (machineTime[i] < machineTime[best]) {
            best = i;
        }
    }

    return best;
}

// Common list scheduling logic used by B and L.

static ScheduleResult runListScheduling(Task* tasks, int n, int m) {
    ScheduleResult result = createResult(m);

    double* machineTime = new double[m];

    for (int i = 0; i < m; i++) {
        machineTime[i] = 0.0;
    }

    for (int i = 0; i < n; i++) {
        int machine = findEarliestMachine(machineTime, m);

        double finish = machineTime[machine] + tasks[i].p;
        machineTime[machine] = finish;

        addEntry(result.machines[machine], tasks[i].id, finish, false);

        result.sigmaC += finish;

        if (finish > result.cmax) {
            result.cmax = finish;
        }
    }

    delete[] machineTime;

    return result;
}

// B: basic list scheduling.

ScheduleResult runB(Task* tasks, int n, int m) {
    return runListScheduling(tasks, n, m);
}

// L from already sorted LPT array.

ScheduleResult runLFromSorted(Task* sortedTasks, int n, int m) {
    return runListScheduling(sortedTasks, n, m);
}

// L: sort by LPT, then list scheduling.

ScheduleResult runL(Task* tasks, int n, int m) {
    Task* sorted = new Task[n];

    copyTasks(tasks, sorted, n);
    sortLPT(sorted, n);

    ScheduleResult result = runLFromSorted(sorted, n, m);

    delete[] sorted;

    return result;
}

// S from already sorted LPT array.

ScheduleResult runSFromSorted(Task* sorted, int n, int m) {
    ScheduleResult result = createResult(m);

    Task** assigned = new Task*[m];
    int* sizes = new int[m];
    int* capacities = new int[m];

    for (int i = 0; i < m; i++) {
        sizes[i] = 0;
        capacities[i] = 4;
        assigned[i] = new Task[capacities[i]];
    }

    // Distribute tasks round-robin to processors.
    for (int i = 0; i < n; i++) {
        int machine = i % m;

        if (sizes[machine] == capacities[machine]) {
            int newCapacity = capacities[machine] * 2;
            Task* newData = new Task[newCapacity];

            for (int j = 0; j < sizes[machine]; j++) {
                newData[j] = assigned[machine][j];
            }

            delete[] assigned[machine];

            assigned[machine] = newData;
            capacities[machine] = newCapacity;
        }

        assigned[machine][sizes[machine]] = sorted[i];
        sizes[machine]++;
    }

    // On each processor execute tasks in SPT order.
    for (int i = 0; i < m; i++) {
        sortTasks(assigned[i], sizes[i], comesBeforeSPT);

        double currentTime = 0.0;

        for (int j = 0; j < sizes[i]; j++) {
            currentTime += assigned[i][j].p;

            addEntry(result.machines[i], assigned[i][j].id, currentTime, false);

            result.sigmaC += currentTime;

            if (currentTime > result.cmax) {
                result.cmax = currentTime;
            }
        }
    }

    for (int i = 0; i < m; i++) {
        delete[] assigned[i];
    }

    delete[] assigned;
    delete[] sizes;
    delete[] capacities;

    return result;
}

// S: sort by LPT, distribute, then execute SPT locally.

ScheduleResult runS(Task* tasks, int n, int m) {
    Task* sorted = new Task[n];

    copyTasks(tasks, sorted, n);
    sortLPT(sorted, n);

    ScheduleResult result = runSFromSorted(sorted, n, m);

    delete[] sorted;

    return result;
}

// M: McNaughton algorithm.

ScheduleResult runM(Task* tasks, int n, int m) {
    ScheduleResult result = createResult(m);

    double sum = 0.0;
    double maxP = 0.0;

    for (int i = 0; i < n; i++) {
        sum += tasks[i].p;

        if (tasks[i].p > maxP) {
            maxP = tasks[i].p;
        }
    }

    double cmax = sum / m;

    if (maxP > cmax) {
        cmax = maxP;
    }

    result.cmax = cmax;

    int machine = 0;
    double currentTime = 0.0;

    for (int i = 0; i < n && machine < m; i++) {
        double remaining = tasks[i].p;
        bool wasSplit = false;

        while (remaining > 0.0 && machine < m) {
            double freeSpace = cmax - currentTime;

            if (remaining <= freeSpace) {
                currentTime += remaining;

                if (wasSplit) {
                    addEntry(result.machines[machine], tasks[i].id, currentTime, true);
                } else {
                    addEntry(result.machines[machine], tasks[i].id, currentTime, false);
                    result.sigmaC += currentTime;
                }

                remaining = 0.0;
            } else {
                addEntry(result.machines[machine], tasks[i].id, cmax, false);
                result.sigmaC += cmax;

                remaining -= freeSpace;

                machine++;
                currentTime = 0.0;
                wasSplit = true;
            }

            if (currentTime == cmax) {
                machine++;
                currentTime = 0.0;
            }
        }
    }

    return result;
}

// Recursive brute force for A.

static void bruteForceA(Task* tasks, int n, int m, int index, double* load, double& best) {
    if (index == n) {
        double currentMax = load[0];

        for (int i = 1; i < m; i++) {
            if (load[i] > currentMax) {
                currentMax = load[i];
            }
        }

        if (currentMax < best) {
            best = currentMax;
        }

        return;
    }

    for (int machine = 0; machine < m; machine++) {
        load[machine] += tasks[index].p;

        double partialMax = load[0];

        for (int i = 1; i < m; i++) {
            if (load[i] > partialMax) {
                partialMax = load[i];
            }
        }

        if (partialMax < best) {
            bruteForceA(tasks, n, m, index + 1, load, best);
        }

        load[machine] -= tasks[index].p;
    }
}

// A: optimal Cmax for very small n.

double runA(Task* tasks, int n, int m) {
    double* load = new double[m];

    for (int i = 0; i < m; i++) {
        load[i] = 0.0;
    }

    double sum = 0.0;

    for (int i = 0; i < n; i++) {
        sum += tasks[i].p;
    }

    double best = sum;

    bruteForceA(tasks, n, m, 0, load, best);

    delete[] load;

    return best;
}

// Prints result in required format.

void printScheduleResult(const ScheduleResult& result) {
    std::cout << "Cmax: " << result.cmax << "\n";
    std::cout << "sigmaC: " << result.sigmaC << "\n";

    for (int i = 0; i < result.machineCount; i++) {
        std::cout << "M" << (i + 1) << ":";

        for (int j = 0; j < result.machines[i].size; j++) {
            ScheduleEntry entry = result.machines[i].data[j];

            if (entry.interrupted) {
                std::cout << "( P" << entry.taskId << " = " << entry.time << " )";
            } else {
                std::cout << "( C" << entry.taskId << " = " << entry.time << " )";
            }
        }

        std::cout << "\n";
    }
}

// Frees memory allocated for schedule result.

void freeScheduleResult(ScheduleResult& result) {
    for (int i = 0; i < result.machineCount; i++) {
        delete[] result.machines[i].data;

        result.machines[i].data = nullptr;
        result.machines[i].size = 0;
        result.machines[i].capacity = 0;
    }

    delete[] result.machines;

    result.machines = nullptr;
    result.machineCount = 0;
    result.cmax = 0.0;
    result.sigmaC = 0.0;
}
