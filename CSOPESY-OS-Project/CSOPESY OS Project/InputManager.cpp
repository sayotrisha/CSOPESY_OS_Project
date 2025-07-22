using namespace std;

#include "InputManager.h"
#include <iostream>
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "Screen.h"
#include "Colors.h"

InputManager::InputManager()
{
}

// stores the created instance of console manager
InputManager* InputManager::inputManager = inputManager;


void InputManager::initialize()
{
    inputManager = new InputManager();
}

void InputManager::destroy()
{
    delete inputManager;
}

InputManager* InputManager::getInstance()
{
    return inputManager;
}

void InputManager::handleMainConsoleInput()
{
    cout << "root:\\> ";
    string input;
    getline(cin, input); // Capture entire line input

    // Convert input to lowercase
    for (char& c : input) {
        c = tolower(c);
    }

    // Split input by spaces
    istringstream iss(input);
    vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };

    if (tokens.empty()) {
        cout << "No command entered." << endl;
        return;
    }

    string command = tokens[0];

    // Check initialization state once
    if (!ConsoleManager::getInstance()->getInitialized() && command != "initialize") {
        cout << "Please initialize the processor configuration first." << endl;
        return;
    }

    // Main Console commands
    if (ConsoleManager::getInstance()->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
        if (command == "initialize") {
            ConsoleManager::getInstance()->setInitialized(true);
            ConsoleManager::getInstance()->initializeConfiguration();

            // Start scheduler
            Scheduler::getInstance()->initialize(ConsoleManager::getInstance()->getNumCpu());
            std::thread schedulerThread([&] {
                Scheduler::getInstance()->start();
                });
            schedulerThread.detach();

            cout << GREEN << "> Processor Configuration Initialized" << RESET << endl;
        }
        else if (command == "exit") {
            cout << RED << "> Exiting emulator..." << RESET << endl;
            ConsoleManager::getInstance()->exitApplication();
        }
        else if (command == "scheduler-start") {
            if (!Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << GREEN << "> Scheduler now running" << RESET << endl;
                cout << GREEN << "> Creating dummy processes..." << RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(true);
                // create batchProcessFrequency number of processes
                std::thread schedulerTestThread([&] {
                    ConsoleManager::getInstance()->schedulerTest();
                    });
                schedulerTestThread.detach();

            }
            else {
                cout << YELLOW << "> Scheduler Test already running" << RESET << endl;
            }
        }
        else if (command == "scheduler-stop") {
            if (Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << RED << "> Scheduler stopped" << RESET << endl;
                cout << GREEN << "> Stopping creation of dummy processes..." << RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(false);
            }
            else {
                cout << RED << "> Scheduler not running" << RESET << endl;
            }
        }
        else if (command == "report-util") {
            ConsoleManager::getInstance()->reportUtil();
        }
        else if (command == "clear") {
            system("cls");
            ConsoleManager::getInstance()->drawConsole();
        }
        else if (command == "help") {
            // Display the list of available commands
            cout << LIGHT_YELLOW << "> List of commands:" << endl
                << "    - initialize            (initializes processor configuration and scheduler based on config.txt)" << endl
                << "    - screen -s <name>      (start a new process)" << endl
                << "    - screen -r <name>      (reattaches to an existing process)" << endl
                << "    - screen -ls            (list all processes)" << endl
                << "    - process-smi           (prints process info, only applicable when attached to a process)" << endl
                << "    - scheduler-start        (starts the creation of dummy processes at configured intervals)" << endl
                << "    - scheduler-stop        (stops the creation of dummy processes initiated by scheduler-test)" << endl
                << "    - report-util           (generates a CPU utilization report and writes it to csopesy-log.txt)" << endl
                << "    - clear                 (clears the screen)" << endl
                << "    - help                  (displays list of commands)" << endl
                << "    - exit                  (exits the emulator)" << RESET << endl;
        }
        else if (command == "memory") {
            FlatMemoryAllocator::getInstance()->printMemoryInfo(ConsoleManager::getInstance()->getTimeSlice());
        }
        else if (command == "screen") {
            if (tokens.size() > 1) {
                string screenCommand = tokens[1];
                string processName = (tokens.size() > 2) ? tokens[2] : "";

                if (screenCommand == "-s" && !processName.empty()) {
                    if (ConsoleManager::getInstance()->getScreenMap().contains(processName)) {
                        cout << RED << "> Error: Process already exists." << RESET << endl;
                    }
                    else {
                        string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
                        auto screenInstance = std::make_shared<Screen>(processName, 0, timestamp, ConsoleManager::getInstance()->getMemPerProc());
                        ConsoleManager::getInstance()->registerConsole(screenInstance);

                        ConsoleManager::getInstance()->switchConsole(processName);
                        ConsoleManager::getInstance()->drawConsole();
                        Scheduler::getInstance()->addProcessToQueue(screenInstance);
                    }
                }
                else if (screenCommand == "-r" && !processName.empty()) {
                    // Check if screen exists before switching
                    auto screenMap = ConsoleManager::getInstance()->getScreenMap();
                    if (screenMap.find(processName) != screenMap.end()) {
                        ConsoleManager::getInstance()->switchConsole(processName);
                        ConsoleManager::getInstance()->drawConsole();
                    }
                    else {
                        cout << RED << "> Error: Screen with name '" << processName << "' does not exist or was not initialized." << RESET << endl;
                    }
                }
                else if (screenCommand == "-ls") {
                    system("cls");
                    ConsoleManager::getInstance()->drawConsole();
                    cout << "root:\\> screen -ls" << endl;
                    ConsoleManager::getInstance()->displayProcessList();
                }
                else {
                    cout << RED << "> Error! Unrecognized command : " << command << RESET << endl;
                }
            }
            else {
                cout << RED << "> Error! Unrecognized command : " << command << RESET << endl;
            }
        }
        else {
            cout << RED << "> Error! Unrecognized command : " << command << RESET << endl;
        }
    }
    else {
        // Process-specific commands
        if (command == "exit") {
            cout << "> Exiting process..." << endl;
            ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
        }
        else if (command == "process-smi") {
            string processName = ConsoleManager::getInstance()->getCurrentConsole()->getConsoleName();
            ConsoleManager::getInstance()->printProcessSmi();
            ConsoleManager::getInstance()->printProcess(processName);
        }
        else {
            cout << RED << "> Error! Unrecognized command : " << command << RESET << endl;
        }
    }
}


