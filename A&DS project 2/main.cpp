#include <iostream>

#include "task.hpp"
#include "treap.hpp"
#include "schedule.hpp"

/* =================================================== main.cpp ======================================================

    Main file of the project. Entry point

    Responsibilities:
    1. Read task
    2. Store tasks in the Treap
    3. Read and execute operations:
        + k p = insert new task at position k
        - id = remove task by id
        B m = basic list scheduling
        L m = LPT scheduling
        S m = SPT variant
        M m = mcnaughton algorithm
        A m = optimal Cmax by brute force
    4. Maintain caches:
        currentCache - current task list copied from Treap
        lptCache - current task list sorted in LPT order
    5. Rebuild caches only after the task list changes - reduces repeated sorting for too many scheduling operations

    Program flow:
        - Treap stores the current task list
        - before scheduling, Treap is copied into currentCache
        - LPT order is stored in lptCache
        - B, M and A use currentCache
        - L and S - lptCache

======================================================================================================================*/

int main() {
    int n;
    std::cin >> n;

    Treap treap;
    int nextId = 1;

    // read initial tasks
    for (int i = 0; i < n; i++) {
        double p;
        std::cin >> p;

        Task task;
        task.id = nextId;
        task.p = p;

        nextId++;

        treap.insertAt(i, task);
    }

    Task* currentCache = nullptr;
    Task* lptCache = nullptr;
    int cacheSize = 0;
    bool cacheDirty = true;

    char op;

    while (std::cin >> op) {
        if (op == '+') {
            int k;
            double p;

            std::cin >> k >> p;

            Task task;
            task.id = nextId;
            task.p = p;

            nextId++;

            treap.insertAt(k - 1, task);

            cacheDirty = true;
        } else if (op == '-') {
            int id;
            std::cin >> id;

            treap.removeById(id);

            cacheDirty = true;
        } else if (op == 'B' || op == 'L' || op == 'S' || op == 'M') {
            int m;
            std::cin >> m;

            // rebuild caches only after task list changed
            if (cacheDirty) {
                delete[] currentCache;
                delete[] lptCache;

                cacheSize = treap.size();

                currentCache = new Task[cacheSize];
                lptCache = new Task[cacheSize];

                treap.copyToArray(currentCache);

                for (int i = 0; i < cacheSize; i++) {
                    lptCache[i] = currentCache[i];
                }

                sortLPT(lptCache, cacheSize);

                cacheDirty = false;
            }

            ScheduleResult result;

            if (op == 'B') {
                result = runB(currentCache, cacheSize, m);
            } else if (op == 'L') {
                result = runLFromSorted(lptCache, cacheSize, m);
            } else if (op == 'S') {
                result = runSFromSorted(lptCache, cacheSize, m);
            } else {
                result = runM(currentCache, cacheSize, m);
            }

            printScheduleResult(result);
            freeScheduleResult(result);
        } else if (op == 'A') {
            int m;
            std::cin >> m;

            // rebuild caches only after task list changed
            if (cacheDirty) {
                delete[] currentCache;
                delete[] lptCache;

                cacheSize = treap.size();

                currentCache = new Task[cacheSize];
                lptCache = new Task[cacheSize];

                treap.copyToArray(currentCache);

                for (int i = 0; i < cacheSize; i++) {
                    lptCache[i] = currentCache[i];
                }

                sortLPT(lptCache, cacheSize);

                cacheDirty = false;
            }

            double copt = runA(currentCache, cacheSize, m);

            std::cout << "Copt: " << copt << "\n";
        }
    }

    delete[] currentCache;
    delete[] lptCache;

    return 0;
}
