#pragma once
using namespace std;

#include "ConsoleManager.h"
#include <iostream>
#include <unordered_map>
#include "Screen.h"


// stores the created instance of console manager
ConsoleManager* ConsoleManager::consoleManager = nullptr;

ConsoleManager::ConsoleManager() {
};

void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
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
    system("cls");
}

void ConsoleManager::switchConsole(string consoleName)
{
    if (this->screenMap.contains(consoleName)) {
        this->currentConsole = this->screenMap[consoleName];

        if (consoleName == MAIN_CONSOLE) {
            this->drawConsole();
        }
    
        this->switchSuccessful = true;
    }
    else {
        cout << "Console name " << consoleName << " not found. Was it initialized?" << endl;
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
	cout << "-----------------------------------" << endl;
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

            cout << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestamp() << " | "
                << "Core: " << coreIDstr << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);


        if (screenPtr && screenPtr->isFinished()) {
            cout << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestampFinished() << " | "
                << "Finished" << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << endl;
        }
    }
    cout << "-----------------------------------" << endl;
}

void ConsoleManager::printProcess(string enteredProcess){
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(enteredProcess);
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);

        //check if process name exits
        if (screenPtr->getProcessName() == enteredProcess) {

            //check if process is finished
            if (screenPtr && screenPtr->isFinished()){
                shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(screenMap.find(enteredProcess)->second);

                auto coreID = screenPtr->getCPUCoreID();
                string coreIDstr;
                if (coreID == -1) {
                    coreIDstr = "N/A";
                }
                else {
                    coreIDstr = to_string(coreID);
                }

                cout << "Process Name: " << enteredProcess << endl;
                cout << "Logs:" << endl;
                cout << "(" << screenPtr->getTimestamp() << ")  "
                    << "Core: " << coreIDstr << "  ";
                //where to put createfile?
                screenPtr->createFile();
                screenPtr->viewFile();
            }
            else {
                cout << "Process is not yet finished" << endl;
            }
            
        }

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

void ConsoleManager::printHeader() {
    cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n";
    cout << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .-'\\  `.'  /  \n";
    cout << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n";
    cout << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n";
    cout << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'     \n";
    cout << "                                                            \n";
}