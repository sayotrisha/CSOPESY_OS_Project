/**
 *  Scheduler.cpp
 *
 *  Purpose:
 *      This file implements the Scheduler class responsible for CPU process
 *      scheduling in the simulated operating system. It manages a queue of
 *      processes, distributes them across available CPU cores, and executes
 *      them based on a selected algorithm (e.g., FCFS, Round Robin).
 *
 *      It supports concurrency using threads, mutexes, and condition variables,
 *      allowing realistic simulation of multi-core scheduling.
 */
#include "Scheduler.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>

 /*----------------------------------------------------------------------
  |  Function Scheduler(int numCores)
  |
  |  Purpose:  Initializes a Scheduler instance with a given number of CPU cores.
  |      Sets internal state and prepares the synchronization primitives.
  |
  |  Parameters:
  |      numCores (IN) -- number of CPU cores available for scheduling.
  |
  |  Returns:  Nothing (constructor)
  *-------------------------------------------------------------------*/
Scheduler::Scheduler(int numCores)
    : numCores(numCores), schedulerRunning(false),
    coresUsed(0), coresAvailable(numCores),
    processQueueMutex(), processQueueCondition() {}

/*------------------------------------------------------- static -----
 |  Declaration: Scheduler* Scheduler::scheduler = nullptr;
 |
 |  Purpose:  Holds the singleton instance of the Scheduler class.
 |      Allows global access to a single scheduler instance.
 *-------------------------------------------------------------------*/
Scheduler* Scheduler::scheduler = nullptr;

/*---------------------------------------------------------------------
 |  Function initialize(int numCores)
 |
 |  Purpose:  Instantiates the singleton Scheduler instance with the
 |      specified number of CPU cores.
 |
 |  Parameters:
 |      numCores (IN) -- number of CPU cores to initialize.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Scheduler::initialize(int numCores) {
    scheduler = new Scheduler(numCores);
}

/*--------------------------------------------------------------
 |  Function ~Scheduler()
 |
 |  Purpose:  Destructor for the Scheduler. Ensures proper shutdown
 |      by calling stop() to clean up any active threads.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing (destructor)
 *-------------------------------------------------------------------*/
Scheduler::~Scheduler() {
    stop();
}

/*-----------------------------------------------------------------
 |  Function Scheduler()
 |
 |  Purpose:  Default constructor for fallback instantiation.
 |      Initializes default values and disables scheduling logic.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing (constructor)
 *-------------------------------------------------------------------*/
Scheduler::Scheduler() {
    this->numCores = -1;
    this->schedulerRunning = false;
    this->coresAvailable = 0;
}

/*---------------------------------------------------------------------
 |  Function start()
 |
 |  Purpose:  Starts the scheduling system using multiple threads (one per core).
 |      Each worker thread pulls processes from the queue and executes them
 |      using the configured algorithm (FCFS or RR).
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------
 |  Function getCoresUsed()
 |
 |  Purpose:  Returns the number of cores currently in use by active processes.
 |
 |  Parameters: None
 |
 |  Returns:  int -- count of busy cores.
 *-------------------------------------------------------------------*/
int Scheduler::getCoresUsed() const {
    return coresUsed;
}

/*---------------------------------------------------------------------
 |  Function getCoresAvailable()
 |
 |  Purpose:  Returns the number of CPU cores that are currently idle.
 |
 |  Parameters: None
 |
 |  Returns:  int -- count of idle cores.
 *-------------------------------------------------------------------*/
int Scheduler::getCoresAvailable() const {
    return coresAvailable;
}

/*--------------------------------------------------------------------
 |  Function stop()
 |
 |  Purpose:  Gracefully stops the scheduler by marking it inactive
 |      and waking up all worker threads to exit.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        schedulerRunning = false;  // Set scheduler running to false
    }
    processQueueCondition.notify_all();  // Wake up all threads
}

/*--------------------------------------------------------------------
 |  Function workerFunction(int core, std::shared_ptr<Screen> process)
 |
 |  Purpose:  Defines the logic executed by each CPU core thread. Depending on
 |      the selected algorithm, it simulates instruction execution with delay.
 |      For RR, unfinished processes are requeued.
 |
 |  Parameters:
 |      core (IN) -- ID of the core executing the process.
 |      process (IN/OUT) -- shared pointer to the Screen (process) to execute.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------
 |  Function addProcessToQueue(std::shared_ptr<Screen> process)
 |
 |  Purpose:  Adds a new process to the scheduling queue and notifies
 |      a waiting worker thread.
 |
 |  Parameters:
 |      process (IN) -- the Screen object representing a process.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Scheduler::addProcessToQueue(std::shared_ptr<Screen> process) {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        processQueue.push(process);
    }
    processQueueCondition.notify_one();  // Notify one waiting thread
}

/*---------------------------------------------------------------------
 |  Function getInstance()
 |
 |  Purpose:  Returns the singleton instance of the Scheduler class.
 |      Creates it if it hasn't been initialized yet.
 |
 |  Parameters: None
 |
 |  Returns:  Scheduler* -- pointer to the singleton Scheduler instance.
 *-------------------------------------------------------------------*/
Scheduler* Scheduler::getInstance() {
    if (scheduler == nullptr) {
        scheduler = new Scheduler();
    }
    return scheduler;
}

/*---------------------------------------------------------------------
 |  Function getCpuCycles()
 |
 |  Purpose:  Retrieves the number of process batches generated so far.
 |
 |  Parameters: None
 |
 |  Returns:  int -- the number of CPU cycles.
 *-------------------------------------------------------------------*/
int Scheduler::getCpuCycles() const {
    return cpuCycles;
}

/*---------------------------------------------------------------------
 |  Function setCpuCycles(int cpuCycles)
 |
 |  Purpose:  Sets the current number of CPU cycles for tracking batch creation.
 |
 |  Parameters:
 |      cpuCycles (IN) -- updated CPU cycle count.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Scheduler::setCpuCycles(int cpuCycles) {
    this->cpuCycles = cpuCycles;
}

/*----------------------------------------------------------------------
 |  Function getSchedulerTestRunning()
 |
 |  Purpose:  Returns the current status of the scheduler test mode (on/off).
 |
 |  Parameters: None
 |
 |  Returns:  bool -- true if scheduler test is active; false otherwise.
 *-------------------------------------------------------------------*/
bool Scheduler::getSchedulerTestRunning() const {
    return schedulerTestRunning;
}

/*---------------------------------------------------------------------
 |  Function setSchedulerTestRunning(int schedulerTestRunning)
 |
 |  Purpose:  Enables or disables the test mode that continuously generates
 |      dummy processes for simulation.
 |
 |  Parameters:
 |      schedulerTestRunning (IN) -- flag to start or stop test mode.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Scheduler::setSchedulerTestRunning(int schedulerTestRunning) {
    this->schedulerTestRunning = schedulerTestRunning;
}
