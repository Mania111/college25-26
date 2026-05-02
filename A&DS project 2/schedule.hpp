#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include "task.hpp"

/* =================================================== schedule.hpp ======================================================

    Declares scheduling algorithms and helpers

    1. runB() - basic list scheduling using current task order
    2. runL() - LPT scheduling: longest processing time first
    3. runS() - special SPT variant: first sort by LPT, distribute tasks round-robin, then execute each machine in the SPT order
    4. runM() - McNaughton algorithm for preemptive scheduling
    5. runA() - "bruteforce" search for optimal Cmax for very small inputs

    Cache helpers: sortLPT(), runLFromSorted(). runSFromSorted()
    Output helpers: printScheduleResult(), freeScheduleResult()

======================================================================================================================*/

ScheduleResult runB(Task* tasks, int n, int m);
ScheduleResult runL(Task* tasks, int n, int m);
ScheduleResult runS(Task* tasks, int n, int m);
ScheduleResult runM(Task* tasks, int n, int m);

double runA(Task* tasks, int n, int m);
// dostaje: tablica zadań, liczba zadań, liczba maszyn
// zwraca: Cmax, sigmaC, harmonogram dla każdej maszyny

void sortLPT(Task* tasks, int n);

ScheduleResult runLFromSorted(Task* sortedTasks, int n, int m);
ScheduleResult runSFromSorted(Task* sortedTasks, int n, int m);

void printScheduleResult(const ScheduleResult& result);
void freeScheduleResult(ScheduleResult& result);

#endif
