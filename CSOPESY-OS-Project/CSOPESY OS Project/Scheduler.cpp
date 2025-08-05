#include "Scheduler.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include "FlatMemoryAllocator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "PagingAllocator.h"


Scheduler::Scheduler(int numCores)
    : numCores(numCores), schedulerRunning(false),
    coresUsed(0), coresAvailable(numCores),
    activeThreads(0),
    processQueueMutex(), processQueueCondition() {
}

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
    for (int i = 0; i < numCores; i++) {
        // Launch each core on a separate detached thread

        std::thread([this, i]() {
            while (schedulerRunning) {
                std::shared_ptr<Screen> process;

                {
                    std::unique_lock<std::mutex> lock(processQueueMutex);
                    processQueueCondition.wait(lock, [this]() { return !processQueue.empty() || !schedulerRunning; });

                    if (!schedulerRunning) return;

                    process = processQueue.front();
                    processQueue.pop();
                    //cout << "Process " << process->getProcessName() << "maximum line " << process->getTotalLine() << " current line " << process->getCurrentLine() << endl;

                    ++activeThreads; // Increment active thread count
                }

                void* memoryPtr = nullptr;
                bool isFlatMemory = ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc();
                bool processInMemory = false;


                if (isFlatMemory) {
                    // if process is in memory, get the memory ptr
                    void* tempPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(process->getMemoryRequired(), process->getProcessName(), process);

                    if (tempPtr) {
                        memoryPtr = tempPtr;
                    }

                    // allocate the memory
                    else {
                        memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);
                    }
                }



                // paging
                else {
                    // check if process is in memory
                    processInMemory = PagingAllocator::getInstance()->isProcessInMemory(process->getProcessName());

                    if (processInMemory) {

                        //process->setIsRunning(true);

                    }
                    // allocate the memory
                    else {
                        processInMemory = PagingAllocator::getInstance()->allocate(process);


                    }
                }


                if (memoryPtr || processInMemory) {
                    {
                        std::lock_guard<std::mutex> lock(processQueueMutex);
                        coresAvailable--;
                        coresUsed++;
                    }
                    process->setCPUCoreID(i);
                    process->setIsRunning(true);
                    workerFunction(i, process, memoryPtr);
                }

                // if the process was failed to be allocated
                else {
                    if (algorithm == "fcfs") {
                        addToFrontOfProcessQueue(process);
                    }
                    else {
                        {
                            std::lock_guard<std::mutex> lock(processQueueMutex);
                            coresAvailable--;
                            coresUsed++;
                        }
                        // if flat memory
                        if (isFlatMemory) {
                            // get oldest process
                            std::shared_ptr<Screen> oldestProcess = FlatMemoryAllocator::getInstance()->findOldestProcess();
                            //cout << "Oldest process: " << oldestProcess->getProcessName() << endl;
                            // get memory ptr of the oldest process
                            void* oldestMemoryPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(oldestProcess->getMemoryRequired(), oldestProcess->getProcessName(), oldestProcess);

                            // deallocate the oldest process
                            FlatMemoryAllocator::getInstance()->deallocate(oldestMemoryPtr, oldestProcess);

                            // put the oldest process back to backing store
                            FlatMemoryAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                            // print a file of the backing store
                            FlatMemoryAllocator::getInstance()->visualizeBackingStore();

                            // if the new process is in backing store, remove it from the backing store
                            FlatMemoryAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);

                            // allocate the new process
                            void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);

                            if (memoryPtr) {
                                process->setCPUCoreID(i);
                                process->setIsRunning(true);
                                workerFunction(i, process, memoryPtr);
                            }
                        }
                        // if paging
                        else {
                            // get oldest process
                            string oldestProcessStr = PagingAllocator::getInstance()->findOldestProcess();
                            std::shared_ptr<Screen> oldestProcess = ConsoleManager::getInstance()->getScreenByProcessName(oldestProcessStr);

                            // deallocate the oldest process
                            PagingAllocator::getInstance()->deallocate(oldestProcess);

                            // put the oldest process back to backing store
                            PagingAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                            // print a file of the backing store
                            PagingAllocator::getInstance()->visualizeBackingStore();

                            // if the new process is in backing store, remove it from the backing store
                            PagingAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);

                            // allocate the new process
                            bool processInMemory = PagingAllocator::getInstance()->allocate(process);

                            if (processInMemory) {
                                process->setCPUCoreID(i);
                                workerFunction(i, process, memoryPtr);
                            }
                        }

                    }

                }

                // Update core tracking after process completion
                {
                    std::lock_guard<std::mutex> lock(processQueueMutex);


                    --activeThreads; // Decrement active thread count
                    if (processQueue.empty() && activeThreads == 0) {
                        schedulerRunning = false;
                        processQueueCondition.notify_all();
                        coresUsed = 0;
                        coresAvailable = ConsoleManager::getInstance()->getNumCpu();
                    }
                }
            }
            }).detach(); // Detach thread for independent execution
    }
}

int Scheduler::getCoresUsed() const {
    return coresUsed;
}

int Scheduler::getCoresAvailable() const {
    return coresAvailable;
}

int Scheduler::getIdleCpuTicks()
{
    return idleCpuTicks;
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        schedulerRunning = false;
    }
    processQueueCondition.notify_all();
}

void Scheduler::workerFunction(int core, std::shared_ptr<Screen> process, void* memoryPtr) {
    string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
    fstream file;
    string fileName = process->getProcessName() + ".txt";
    file.open(fileName, std::ios::app);

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
            process->setCPUCoreID(core);

            std::this_thread::sleep_for(std::chrono::milliseconds(80));
            string printed;
            unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
            auto it = screenMap.find(process->getProcessName());

            if (ConsoleManager::getInstance()->getDelayPerExec() != 0) {
                for (int i = 0; i < ConsoleManager::getInstance()->getDelayPerExec(); i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (file.is_open()) {
                file.seekg(0, std::ios::end);
                if (file.tellg() == 0) {  // not empty
                    file << "Process name: " << process->getProcessName() << std::endl;
                    file << "Logs: " << std::endl << std::endl;
                }
                shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(screenMap.find(process->getProcessName())->second);
                file << "(" << ConsoleManager::getInstance()->getCurrentTimestamp() << ")";
                file << "  Core: " << process->getCPUCoreID();
                file << " \"Hello world from " << process->getProcessName() << "!\"" << std::endl;
            }
            else {
                std::cerr << "Failed to open " << fileName << std::endl;
            }

            process->setCurrentLine(process->getCurrentLine() + 1);
            // Increment active cpu tick
            cpuCycles++;

            if (coresAvailable > 0) {
                idleCpuTicks += coresAvailable;
            }
        }

        // subtract cores utilization
        {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            coresAvailable++;
            coresUsed--;
        }

        // deallocate memory
        if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
            FlatMemoryAllocator::getInstance()->deallocate(memoryPtr, process);
        }
        else {
            PagingAllocator::getInstance()->deallocate(process);
        }


    }

    else if (algorithm == "rr") {
        // Round-Robin logic
        int quantum = ConsoleManager::getInstance()->getTimeSlice();  // Get RR time slice

        process->setCPUCoreID(core);
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        string coreIDstr;
        string printed;
        unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
        auto it = screenMap.find(process->getProcessName());

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

            if (file.is_open()) {
                file.seekg(0, std::ios::end);
                if (file.tellg() == 0) {  // not empty
                    file << "Process name: " << process->getProcessName() << std::endl;
                    file << "Logs: " << std::endl << std::endl;
                }
                shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(screenMap.find(process->getProcessName())->second);
                file << "(" << ConsoleManager::getInstance()->getCurrentTimestamp() << ")";
                file << "  Core: " << process->getCPUCoreID();
                file << " \"Hello world from " << process->getProcessName() << "!\"" << std::endl;
            }
            else {
                std::cerr << "Failed to open " << fileName << std::endl;
            }

            //cout << "running rr" << process->getProcessName() << " " << process->getCurrentLine() << " " << process->getTotalLine() << endl;
            process->setCurrentLine(process->getCurrentLine() + 1);
            PagingAllocator::getInstance()->setNumPagedIn(PagingAllocator::getInstance()->getNumPagedIn() + 1);
            PagingAllocator::getInstance()->setNumPagedOut(PagingAllocator::getInstance()->getNumPagedOut() + 1);

            // Increment active cpu tick
            cpuCycles++;

            if (coresAvailable > 0) {
                idleCpuTicks += coresAvailable;
            }
        }

        //if process is not finished, re-queue it but retain its core affinity
        if (process->getCurrentLine() < process->getTotalLine()) {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            processQueue.push(process);  // Re-queue the unfinished process
            processQueueCondition.notify_one();
        }


        process->setIsRunning(false);
        // subtract cores utilization
        {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            coresAvailable++;
            coresUsed--;
        }

    }

    file.close();
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

void Scheduler::addToFrontOfProcessQueue(std::shared_ptr<Screen> process) {
    std::lock_guard<std::mutex> lock(processQueueMutex);

    // Create a temporary queue and add the new process at the front
    std::queue<std::shared_ptr<Screen>> tempQueue;
    tempQueue.push(process);

    // Add the remaining processes to the temporary queue
    while (!processQueue.empty()) {
        tempQueue.push(processQueue.front());
        processQueue.pop();
    }

    // Replace the original queue with the temporary queue
    processQueue = std::move(tempQueue);

    processQueueCondition.notify_all();  // Notify all waiting threads
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
