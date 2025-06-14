#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "Screen.h"

class Scheduler {
public:
    Scheduler(int numCores);
    Scheduler();
    ~Scheduler();
    void start();
    void stop();
    void addProcessToQueue(std::shared_ptr<Screen> process);
    void workerFunction(int core, std::shared_ptr<Screen> process);
    static Scheduler* getInstance();
    static void initialize(int numCores);

    int getCoresUsed() const;
    int getCoresAvailable() const;
    int coresUsed = 0; // Tracks how many cores are currently used
    int coresAvailable; // Tracks how many cores are available

private:
    int numCores;
    bool schedulerRunning;
    std::vector<std::thread> workerThreads;
    std::queue<std::shared_ptr<Screen>> processQueue;
    std::mutex processQueueMutex;
    std::condition_variable processQueueCondition;
    static Scheduler* scheduler;
};

#endif // SCHEDULER_H
