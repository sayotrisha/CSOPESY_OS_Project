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

Scheduler::Scheduler() {
    this->numCores = -1;
    this->schedulerRunning = false;
    this->coresAvailable = 0;
}
void Scheduler::start() {
    schedulerRunning = true;
    algorithm = ConsoleManager::getInstance()->getSchedulerConfig();

    std::vector<std::thread> workerThreads(numCores);
    for (int i = 0; i < numCores; i++) {
        workerThreads[i] = std::thread([this, i]() {
            while (schedulerRunning) {
                if (!processQueue.empty()) {
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

            }
            });
    }

    // Wait for all worker threads to finish
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    cout << "scheduler stopped\n" << endl;

    stop();
}

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

    // Ensure the process keeps its original core for FCFS and RR
    if (process->getCPUCoreID() == -1) {
        // If core is not yet assigned, set the current core as the affinity core
        process->setCPUCoreID(core);
    }
    else {
        // Otherwise, ensure the process stays on its assigned core
        core = process->getCPUCoreID();
    }

    if (algorithm == "fcfs") {
        // First-Come, First-Served logic
        for (int i = 0; i < process->getTotalLine(); i++) {
            if (ConsoleManager::getInstance()->getDelayPerExec() != 0) {
                for (int i = 0; i < ConsoleManager::getInstance()->getDelayPerExec(); i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            process->setCurrentLine(process->getCurrentLine() + 1);
        }
    }
	
    else if (algorithm == "rr") {
       // Round-Robin logic
       int quantum = ConsoleManager::getInstance()->getTimeSlice();  // Get RR time slice

       // Process for the duration of the quantum or until the process is finished
       for (int i = 0; i < quantum && process->getCurrentLine() < process->getTotalLine(); i++) {
           if (ConsoleManager::getInstance()->getDelayPerExec() != 0) {
               for (int i = 0; i < ConsoleManager::getInstance()->getDelayPerExec(); i++) {
                   std::this_thread::sleep_for(std::chrono::milliseconds(100));
               }
           }
		   else {
			   std::this_thread::sleep_for(std::chrono::milliseconds(100));
           }
           process->setCurrentLine(process->getCurrentLine() + 1);

       }

       // If process is not finished, re-queue it but retain its core affinity
       if (process->getCurrentLine() < process->getTotalLine()) {
           std::lock_guard<std::mutex> lock(processQueueMutex);
           processQueue.push(process);  // Re-queue the unfinished process
           processQueueCondition.notify_one();
       }
    }

    string timestampFinished = ConsoleManager::getInstance()->getCurrentTimestamp();
    process->setTimestampFinished(timestampFinished);  // Log completion time
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

int Scheduler::getCpuCycles() const {
    return cpuCycles;
}

void Scheduler::setCpuCycles(int cpuCycles) {
    this->cpuCycles = cpuCycles;
}

bool Scheduler::getSchedulerTestRunning() const {
    return schedulerTestRunning;
}

void Scheduler::setSchedulerTestRunning(int schedulerTestRunning) {
    this->schedulerTestRunning = schedulerTestRunning;
}
