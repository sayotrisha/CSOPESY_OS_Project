#include "Scheduler.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>

Scheduler::Scheduler(int numCores)
    : numCores(numCores), schedulerRunning(false),
    coresUsed(0), coresAvailable(numCores),
    processQueueMutex(), processQueueCondition() {}

Scheduler* Scheduler::scheduler = nullptr;

void Scheduler::initialize(int numCores) {
    scheduler = new Scheduler(numCores);
}

Scheduler::~Scheduler() {
    stop();
}

Scheduler::Scheduler() : numCores(-1), schedulerRunning(false) {}
void Scheduler::start() {
    schedulerRunning = true;

    std::vector<std::thread> workerThreads(numCores);

    for (int i = 0; i < numCores; i++) {
        workerThreads[i] = std::thread([this, i]() {
            while (schedulerRunning) {
                std::shared_ptr<Screen> process;

                // Lock the queue for popping the next process
                {
                    std::unique_lock<std::mutex> lock(processQueueMutex);
                    // Wait for a process to be added to the queue
                    processQueueCondition.wait(lock, [this]() { return !processQueue.empty() || !schedulerRunning; });

                    if (!schedulerRunning) {
                        return; // Exit thread if the scheduler is stopping
                    }

                    // Pop the next process from the queue
                    process = processQueue.front();
                    processQueue.pop();
                    coresUsed++;  // Increment cores used
                    coresAvailable--;  // Decrement available cores
                }

                // Set the core ID for the process being processed
                process->setCPUCoreID(i); // Assign the core ID to the process

                // Process the worker function
                workerFunction(i, process);

                // After processing, update cores used and available
                {
                    std::lock_guard<std::mutex> lock(processQueueMutex);
                    coresUsed--;  // Decrement cores used
                    coresAvailable++;  // Increment available cores
                }
            }
            });
    }

    // Wait for all worker threads to finish
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    stop();
}

/*void Scheduler::start() {
    schedulerRunning = true;

    // Create a thread pool with a fixed number of worker threads
    std::vector<std::thread> workerThreads(numCores);

    for (int i = 0; i < numCores; i++) {
        workerThreads[i] = std::thread([this, i]() {
            while (schedulerRunning) {
                std::shared_ptr<Screen> process;

                // Lock the queue for popping the next process
                {
                    std::unique_lock<std::mutex> lock(processQueueMutex);
                    // Wait for a process to be added to the queue
                    processQueueCondition.wait(lock, [this]() { return !processQueue.empty() || !schedulerRunning; });

                    if (!schedulerRunning) {
                        return; // Exit thread if the scheduler is stopping
                    }

                    // Pop the next process from the queue
                    process = processQueue.front();
                    processQueue.pop();
                    coresUsed++;  // Increment cores used
                    coresAvailable--;  // Decrement available cores
                }

                // Process the worker function
                workerFunction(i, process);

                // After processing, update cores used and available
                {
                    std::lock_guard<std::mutex> lock(processQueueMutex);
                    coresUsed--;  // Decrement cores used
                    coresAvailable++;  // Increment available cores
                }
            }
            });
    }

    // Wait for all worker threads to finish
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    stop();
}*/

int Scheduler::getCoresUsed() const {
    return coresUsed;
}

int Scheduler::getCoresAvailable() const {
    return coresAvailable;
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        schedulerRunning = false;  // Set scheduler running to false
    }
    processQueueCondition.notify_all();  // Wake up all threads
}

void Scheduler::workerFunction(int core, std::shared_ptr<Screen> process) {
    string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
    for (int i = 0; i < process->getTotalLine(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        process->setCurrentLine(process->getCurrentLine() + 1);
        process->setCPUCoreID(core);
    }
    string timestampFinished = ConsoleManager::getInstance()->getCurrentTimestamp();
    process->setTimestampFinished(timestampFinished);
}

void Scheduler::addProcessToQueue(std::shared_ptr<Screen> process) {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        processQueue.push(process);
    }
    processQueueCondition.notify_one();  // Notify one waiting thread
}

Scheduler* Scheduler::getInstance() {
    if (scheduler == nullptr) {
        scheduler = new Scheduler();
    }
    return scheduler;
}
