#pragma once
using namespace std;

#include "ConsoleManager.h"
#include <iostream>
#include <unordered_map>
#include "Screen.h"
#include "Colors.h"


// stores the created instance of console manager
ConsoleManager* ConsoleManager::consoleManager = nullptr;

ConsoleManager::ConsoleManager() {
};

void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
}

void ConsoleManager::initializeConfiguration() {
    FILE* file;
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
            else if (key == "mem-per-proc") {
                ConsoleManager::getInstance()->setMemPerProc(stoi(value));
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
            /* string processName = "cycle" + std::to_string(ConsoleManager::getInstance()->cpuCycles) + "processName" + std::to_string(i);*/
            string processName = "P" + std::to_string(process_counter);
            shared_ptr<BaseScreen> processScreen = make_shared<Screen>(processName, 0, ConsoleManager::getInstance()->getCurrentTimestamp(), ConsoleManager::getInstance()->getMemPerProc());
            shared_ptr<Screen> screenPtr = static_pointer_cast<Screen>(processScreen);
            Scheduler::getInstance()->addProcessToQueue(screenPtr);
            ConsoleManager::getInstance()->registerConsole(processScreen);
            ConsoleManager::getInstance()->cpuCycles++;


        }
        process_counter++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
                cout << "Screen Name: " << this->screenMap[consoleName]->getConsoleName() << endl;
                cout << "Current line of instruction / Total line of instruction: ";
                cout << this->screenMap[consoleName]->getCurrentLine();
                cout << "/" << this->screenMap[consoleName]->getTotalLine() << endl;
                cout << "Timestamp: " << this->screenMap[consoleName]->getTimestamp() << endl;
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
    //system("cls");
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
    float cpuUtilization = (float)coresUsed / (coresUsed + coresAvailable) * 100;

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

    // Write the log data to a file
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
                //where to put createfile?
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
    cout << "Process: " << this->consoleName << endl;
    if (this->screenMap[consoleName]->getCurrentLine() == this->screenMap[consoleName]->getTotalLine()) {
        cout << "Finished!" << endl;
    }
    else {
        cout << "Current Line: " << this->screenMap[consoleName]->getCurrentLine() << endl;
        cout << "Lines of Code: " << this->screenMap[consoleName]->getTotalLine() << endl;
    }

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

void ConsoleManager::setMemPerProc(size_t memPerProc) {
    this->memPerProc = memPerProc;
}

size_t ConsoleManager::getMaxOverallMem() {
    return this->maxOverallMem;
}

size_t ConsoleManager::getMemPerFrame() {
    return this->memPerFrame;
}

size_t ConsoleManager::getMemPerProc() {
    return this->memPerProc;
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
