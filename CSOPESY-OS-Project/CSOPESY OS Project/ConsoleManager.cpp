#pragma once
using namespace std;

#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"
#include <iostream>
#include <unordered_map>
#include "Screen.h"
#include "Colors.h"
#include <random>

// stores the created instance of console manager
ConsoleManager* ConsoleManager::consoleManager = nullptr;

ConsoleManager::ConsoleManager() {
};

void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
    ConsoleManager::getInstance()->initializeConfiguration();
}

void ConsoleManager::initializeConfiguration() {
    FILE* file;
    /*   This gets the input configuration from the config.txt file   */
    errno_t err = fopen_s(&file, "config.txt", "r");
    if (err != 0) {
        cout << "Error opening file" << endl;
    }

    else {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            string str = line;
            size_t space_pos = str.find(" ");

            if (space_pos == string::npos) continue; // Skip malformed lines

            string key = str.substr(0, space_pos);
            string value = str.substr(space_pos + 1);
            value.erase(remove(value.begin(), value.end(), '\n'), value.end()); // Remove newline

            if (key == "num-cpu") {
                ConsoleManager::getInstance()->setNumCpu(stoi(value));
            }
            else if (key == "scheduler") {
                value.erase(remove(value.begin(), value.end(), '\"'), value.end()); // Remove quotes
                ConsoleManager::getInstance()->setSchedulerConfig(value);
            }
            else if (key == "quantum-cycles") {
                ConsoleManager::getInstance()->setTimeSlice(stoi(value));
            }
            else if (key == "min-ins") {
                ConsoleManager::getInstance()->setMinIns(stoi(value));
            }
            else if (key == "max-ins") {
                ConsoleManager::getInstance()->setMaxIns(stoi(value));
            }
            else if (key == "delay-per-exec") {
                ConsoleManager::getInstance()->setDelayPerExec(stoi(value));
            }
            else if (key == "batch-process-freq") {
                ConsoleManager::getInstance()->setBatchProcessFrequency(stoi(value));
            }
            else if (key == "max-overall-mem") {
                ConsoleManager::getInstance()->setMaxOverallMem(stoi(value));
            }
            else if (key == "mem-per-frame") {
                ConsoleManager::getInstance()->setMemPerFrame(stoi(value));
            }
            else if (key == "min-mem-per-proc") {
                ConsoleManager::getInstance()->setMinMemPerProc(stoi(value));
            }
            else if (key == "max-mem-per-proc") {
                ConsoleManager::getInstance()->setMaxMemPerProc(stoi(value));
            }

        }
        fclose(file);
    }


    Scheduler* scheduler = Scheduler::getInstance();
}

void ConsoleManager::schedulerTest() {
    static int process_counter = 0;
    process_counter++;

    while (Scheduler::getInstance()->getSchedulerTestRunning()) {
        for (int i = 0; i < ConsoleManager::getInstance()->getBatchProcessFrequency(); i++) {
            process_counter++;
            string processName = "P" + std::to_string(process_counter);
            shared_ptr<BaseScreen> processScreen = make_shared<Screen>(processName, 0, ConsoleManager::getInstance()->getCurrentTimestamp(), ConsoleManager::getInstance()->getMinMemPerProc());
            shared_ptr<Screen> screenPtr = static_pointer_cast<Screen>(processScreen);
            Scheduler::getInstance()->addProcessToQueue(screenPtr);
            ConsoleManager::getInstance()->registerConsole(processScreen);
            ConsoleManager::getInstance()->cpuCycles++;

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        

    }
}

int ConsoleManager::getCpuCycles() {
    return this->cpuCycles;
}

void ConsoleManager::drawConsole() {
    if (this->switchSuccessful) {
        system("cls");
        string consoleName = this->getCurrentConsole()->getConsoleName();

        if (this->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
            this->printHeader();
        }
        else {
            if (this->screenMap.contains(consoleName)) {
                cout << "Screen Name: " << YELLOW << this->screenMap[consoleName]->getConsoleName() << RESET << endl;
                cout << "Current line of instruction / Total line of instruction: ";
                cout << YELLOW << this->screenMap[consoleName]->getCurrentLine() << RESET;
                cout << BLUE << "/" << RESET << YELLOW << this->screenMap[consoleName]->getTotalLine() << RESET << endl;
                cout << "Timestamp: " << YELLOW <<  this->screenMap[consoleName]->getTimestamp() << RESET << endl;
            }
        }
    }
}

void ConsoleManager::destroy() {
    consoleManager->scheduler.stop(); // Stop the scheduler
    delete consoleManager;
}

string ConsoleManager::getCurrentTimestamp() {
    // Get current time as time_t object
    time_t currentTime = time(nullptr);
    // Create tm structure to store local time
    tm localTime;
    // Convert time_t to tm structure 
    localtime_s(&localTime, &currentTime);
    // Create a buffer to store the formatted time
    char timeBuffer[100];
    // Format the time (MM/DD/YYYY, HH:MM:SS AM/PM)
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", &localTime);
    return timeBuffer;
}

void ConsoleManager::registerConsole(shared_ptr<BaseScreen> screenRef) {
    this->screenMap[screenRef->getConsoleName()] = screenRef; //it should accept MainScreen and ProcessScreen
}

void ConsoleManager::switchConsole(string consoleName)
{
    if (this->screenMap.contains(consoleName)) {
        this->currentConsole = this->screenMap[consoleName];
        this->consoleName = consoleName;

        if (consoleName == MAIN_CONSOLE) {
            this->drawConsole();
        }

        this->switchSuccessful = true;
    }
    else {
        cout << RED << "Console name " << consoleName << " not found. Was it initialized?" << RESET << endl;
        this->switchSuccessful = false;
    }
}

void ConsoleManager::displayProcessList() {
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    cout << "\nCPU Utilization: " << cpuUtilization << "%" << endl;
    cout << "Cores used: " << coresUsed << endl;
    cout << "Cores available: " << coresAvailable << endl;
    cout << BLUE << "-----------------------------------" << RESET << endl;
    cout << "Running processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);

        if (screenPtr && !screenPtr->isFinished()) {

            auto coreID = screenPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            cout << "Name: " << screenPtr->getProcessName() << BLUE << " (" << RESET << YELLOW
                << screenPtr->getTimestamp() << RESET << BLUE << ") " << RESET
                << "Core" << BLUE << ": " << RESET << YELLOW << coreIDstr << "   "
                << screenPtr->getCurrentLine() << RESET << BLUE << "/"
                << YELLOW << screenPtr->getTotalLine() << "   " << RESET << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);


        if (screenPtr && screenPtr->isFinished()) {
            cout << "Name: " << screenPtr->getProcessName() << BLUE << " (" << RESET << YELLOW
                << screenPtr->getTimestamp() << RESET << BLUE << ") " << RESET
                << "   " << "Finished" << "   "
                << YELLOW << screenPtr->getCurrentLine() << RESET << BLUE << "/" << RESET
                << YELLOW << screenPtr->getTotalLine() << RESET << "   " << endl;
        }
    }
    cout << BLUE << "-----------------------------------" << RESET << endl;
}

void ConsoleManager::reportUtil() {
    std::ostringstream logStream;
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    // Log CPU utilization and core details
    logStream << "\nCPU Utilization: " << cpuUtilization << "%" << std::endl;
    logStream << "Cores used: " << coresUsed << std::endl;
    logStream << "Cores available: " << coresAvailable << std::endl;
    logStream << "-----------------------------------" << std::endl;
    logStream << "Running processes:" << std::endl;

    // Log details of running processes
    for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Screen>(pair.second);
        if (screenPtr && !screenPtr->isFinished()) {
            auto coreID = screenPtr->getCPUCoreID();
            std::string coreIDstr = (coreID == -1) ? "N/A" : std::to_string(coreID);

            logStream << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestamp() << " | "
                << "Core: " << coreIDstr << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << std::endl;
        }
    }

    logStream << "\nFinished processes:" << std::endl;

    // Log details of finished processes
    for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Screen>(pair.second);
        if (screenPtr && screenPtr->isFinished()) {
            logStream << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestampFinished() << " | "
                << "Finished" << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << std::endl;
        }
    }

    logStream << "-----------------------------------" << std::endl;

	// Write the log data to a file in text_files directory
    std::ofstream file("text_files/csopesy-log.txt", std::ios::out);
    if (file.is_open()) {
        file << logStream.str(); // Write log contents to file
        file.close();
        cout << "Report generated at text_files/csopesy-log.txt" << std::endl;
    }
    else {
        cerr << "Error: Could not open file for writing." << std::endl;
    }
}

void ConsoleManager::initializeAllocators() {
    FlatMemoryAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
    PagingAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
}

std::shared_ptr<Screen> ConsoleManager::getScreenByProcessName(const std::string& processName) {
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(processName);
    if (it != screenMap.end()) {
        std::shared_ptr<Screen> process = std::dynamic_pointer_cast<Screen>(it->second);
        if (process) {
            return process;  // Successfully casted to Screen
        }
    }
    return nullptr;  // Process not found
}

int ConsoleManager::getNumCpu() {
    return this->num_cpu;
}

string ConsoleManager::getSchedulerConfig() {
    return this->schedulerConfig;
}

int ConsoleManager::getTimeSlice() {
    return this->timeSlice;
}

int ConsoleManager::getBatchProcessFrequency() {
    return this->batchProcessFrequency;
}

int ConsoleManager::getMinIns() {
    return this->minIns;
}

int ConsoleManager::getMaxIns() {
    return this->maxIns;
}

int ConsoleManager::getDelayPerExec() {
    return this->delayPerExec;
}

void ConsoleManager::setNumCpu(int num_cpu) {
    this->num_cpu = num_cpu;
}

void ConsoleManager::setSchedulerConfig(string scheduler) {
    this->schedulerConfig = scheduler;
}

void ConsoleManager::setTimeSlice(int timeSlice) {
    this->timeSlice = timeSlice;
}

void ConsoleManager::setBatchProcessFrequency(int batchProcessFrequency) {
    this->batchProcessFrequency = batchProcessFrequency;
}

void ConsoleManager::setMinIns(int minIns) {
    this->minIns = minIns;
}

void ConsoleManager::setMaxIns(int maxIns) {
    this->maxIns = maxIns;
}

void ConsoleManager::setDelayPerExec(int delayPerExec) {
    this->delayPerExec = delayPerExec;
}


void ConsoleManager::printProcess(string enteredProcess) {
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(enteredProcess);

    if (it == screenMap.end()) {
        cout << RED << "Process: '" << enteredProcess << "' not found." << RESET << endl;
        return;
    }
    shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(it->second);
    if (!screenPtr) {
        cout << RED << "Screen '" << enteredProcess << "' is not a process screen." << RESET << endl;
        return;
    }

    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);

        //check if process name exits
        if (screenPtr->getProcessName() == enteredProcess) {

            //check if process is finished
            if (screenPtr && screenPtr->isFinished()) {
                shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(screenMap.find(enteredProcess)->second);

                auto coreID = screenPtr->getCPUCoreID();
                string coreIDstr;
                if (coreID == -1) {
                    coreIDstr = "N/A";
                }
                else {
                    coreIDstr = to_string(coreID);
                }

                cout << BLUE << "Process Name: " << enteredProcess << endl;
                cout << "Logs:" << endl;
                cout << "(" << screenPtr->getTimestamp() << ")  "
                    << "Core: " << coreIDstr << "  " << RESET;
                screenPtr->createFile();
                screenPtr->viewFile();
            }
            else {
                cout << RED << "Process is not yet finished" << RESET << endl;
            }

        }

    }
}

void ConsoleManager::printProcessSmi() {
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    cout << "--------------------------------------------------" << endl;
    cout << "|    PROCESS-SMI V01.00 Driver Version 01.00      |" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "CPU Utilization: " << cpuUtilization << "%" << endl;
    getMemoryUsage();

    cout << "===================================================" << endl;
    cout << "Running processes and memory usage:" << endl;
    cout << "---------------------------------------------------" << endl;

    // Iterate through screenMap to get running processes and their memory usage
    for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Screen>(pair.second);
        if (screenPtr && !screenPtr->isFinished() && screenPtr->getIsRunning() && screenPtr->getMemoryUsage() != 0) {  // Only show running processes
            size_t memoryUsage = screenPtr->getMemoryUsage();
            cout << "Process: " << screenPtr->getProcessName()
                << " | Memory: " << memoryUsage << " KB" << endl;
        }
    }

    cout << "===================================================" << endl << endl;
}

void ConsoleManager::getMemoryUsage() {
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << "Memory Usage: " << FlatMemoryAllocator::getInstance()->visualizeMemory() << endl;
    }
    else {
        PagingAllocator::getInstance()->visualizeMemory();
    }
}

void ConsoleManager::printVmstat() {
    cout << "Total Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() << " KB" << endl;
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << "Used Memory: " << FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << " KB" << endl;
        cout << "Free Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() - FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << " KB" << endl;
    }
    else {
        cout << "Used Memory: " << PagingAllocator::getInstance()->getUsedMemory() << " KB" << endl;
        cout << "Free Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() - PagingAllocator::getInstance()->getUsedMemory() << " KB" << endl;
    }


    cout << "Idle CPU Ticks: " << Scheduler::getInstance()->getIdleCpuTicks() << endl;
    cout << "Active CPU Ticks: " << Scheduler::getInstance()->getCpuCycles() << endl;
    cout << "Total CPU Ticks: " << Scheduler::getInstance()->getCpuCycles() + Scheduler::getInstance()->getIdleCpuTicks() << endl;
    cout << "Num paged in: " << PagingAllocator::getInstance()->getNumPagedIn() << endl;
    cout << "Num paged out: " << PagingAllocator::getInstance()->getNumPagedOut() << endl << endl;
}


shared_ptr<BaseScreen> ConsoleManager::getCurrentConsole()
{
    return this->currentConsole;
}

void ConsoleManager::setCurrentConsole(shared_ptr<BaseScreen> screenRef)
{
    this->currentConsole = screenRef;
}

ConsoleManager* ConsoleManager::getInstance()
{
    return consoleManager;
}

void ConsoleManager::exitApplication() {
    this->running = false;
    Scheduler::getInstance()->stop();
}

bool ConsoleManager::isRunning() {
    return this->running;
}

unordered_map<string, shared_ptr<BaseScreen>> ConsoleManager::getScreenMap() {
    return this->screenMap;
}

void ConsoleManager::setInitialized(bool initialized) {
    this->initialized = initialized;
}

bool ConsoleManager::getInitialized() {
    return this->initialized;
}

void ConsoleManager::setMaxOverallMem(size_t maxOverallMem) {
    this->maxOverallMem = maxOverallMem;
}

void ConsoleManager::setMemPerFrame(size_t memPerFrame) {
    this->memPerFrame = memPerFrame;
}

void ConsoleManager::setMinMemPerProc(size_t minMemPerProc) {
    this->minMemPerProc = minMemPerProc;
}

void ConsoleManager::setMaxMemPerProc(size_t maxMemPerProc) {
    this->maxMemPerProc = maxMemPerProc;
}

size_t ConsoleManager::getMaxOverallMem() {
    return this->maxOverallMem;
}

size_t ConsoleManager::getMemPerFrame() {
    return this->memPerFrame;
}

size_t ConsoleManager::getMinMemPerProc() {
    return this->minMemPerProc;
}

size_t ConsoleManager::getMaxMemPerProc() {
    return this->maxMemPerProc;
}

void ConsoleManager::setNumPages() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinMemPerProc(), ConsoleManager::getInstance()->getMaxMemPerProc());

    this->numPages = dis(gen) / ConsoleManager::getInstance()->getMemPerFrame();
}

int ConsoleManager::getNumPages() {
    return this->numPages;
}

void ConsoleManager::printHeader() {
    cout << PASTEL_PINK << "________________________________________________________________________________\n";
    cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n";
    cout << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .-'\\  `.'  /  \n";
    cout << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n";
    cout << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n";
    cout << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'     \n";
    cout << "________________________________________________________________________________\n" << RESET;
    cout << "\n";
}

void ConsoleManager::printMarquee() {
    std::string text = "Welcome to our Command Line Emulator!!! ";
    int len = static_cast<int>(text.size());

    // Marquee effect: one full cycle
    for (int offset = 0; offset < len; ++offset) {
        // Move cursor to beginning of line
        cout << "\r";

        // Print substring starting at offset
        cout << PASTEL_PINK << text.substr(offset);

        // Then print the leading part to complete the loop
        cout << text.substr(0, offset) << RESET;

        cout.flush();

        // Sleep using std::this_thread::sleep_for
        this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    cout << CYAN << "\n> List of commands:" << endl
        << "    - initialize            (initializes processor configuration and scheduler based on config.txt)" << endl
        << "    - screen -s <name>      (start a new process)" << endl
        << "    - screen -r <name>      (reattaches to an existing process)" << endl
        << "    - screen -ls            (list all processes)" << endl
        << "    - process-smi           (prints process info, only applicable when attached to a process)" << endl
        << "    - scheduler-start       (starts the creation of dummy processes at configured intervals)" << endl
        << "    - scheduler-stop        (stops the creation of dummy processes initiated by scheduler-test)" << endl
        << "    - report-util           (generates a CPU utilization report and writes it to csopesy-log.txt)" << endl
        << "    - clear                 (clears the screen)" << endl
        << "    - help                  (displays list of commands)" << endl
        << "    - exit                  (exits the emulator)" << RESET << endl;
}