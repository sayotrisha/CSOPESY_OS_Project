/**
 *  InputManager.cpp
 *
 *  Purpose:
 *      This file implements the InputManager class, which handles all
 *      user input for the console-based OS simulation. It processes commands,
 *      delegates actions to ConsoleManager, and controls simulation features
 *      like initialization, screen creation, and scheduler interaction.
 *
 *      InputManager ensures command parsing, validation, and execution,
 *      supporting both main screen and process-specific command scopes.
 */
using namespace std;

#include "InputManager.h"
#include <iostream>
#include "ConsoleManager.h"
#include "Screen.h"
#include "Colors.h"

/*-------------------------------------------------------------------
 |
 |  Purpose:  Default constructor for the InputManager class.
 |      Initializes an InputManager instance. This constructor is typically
 |      called through initialize().
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
InputManager::InputManager()
{
}

/*------------------------------------------------- Static Instance -----
 |  Declaration: InputManager* InputManager::inputManager = inputManager;
 |
 |  Purpose:  Static member to hold the singleton instance of InputManager.
 |      Ensures only one global input handler exists throughout the program.
 |
 |  Comments: stores the created instance of console manager
 *-------------------------------------------------------------------*/
InputManager* InputManager::inputManager = inputManager;

/*--------------------------------------------------------------------
 |  Function initialize()
 |
 |  Purpose:  Instantiates the singleton InputManager. Must be called before
 |      handling user input.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void InputManager::initialize()
{
	inputManager = new InputManager();
}

/*------------------------------------------------------------------
 |  Function destroy()
 |
 |  Purpose:  Deallocates the InputManager singleton instance.
 |      Called when the application exits to clean up resources.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void InputManager::destroy()
{
	delete inputManager;
}

/*---------------------------------------------------------------------
 |  Function getInstance()
 |
 |  Purpose:  Returns the singleton instance of the InputManager.
 |
 |  Parameters: None
 |
 |  Returns:  InputManager* -- pointer to the global InputManager instance.
 *-------------------------------------------------------------------*/
InputManager* InputManager::getInstance()
{
	return inputManager;
}

/*---------------------------------------------------------------------
 |  Function handleMainConsoleInput()
 |
 |  Purpose:  Reads user input from the command line and parses it into tokens.
 |      Interprets and executes commands depending on whether the current
 |      screen is the main console or a process screen.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void InputManager::handleMainConsoleInput()
{
    cout << "root:\\> ";
    string input;
    getline(cin, input); // Capture entire line input

    // Convert input to lowercase
    for (char &c : input) {
        c = tolower(c);
    }

    // Split input by spaces
    istringstream iss(input);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

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

            cout << GREEN << "'Processor Configuration Initialized'" << RESET << endl;
        }
        else if (command == "exit") {
            ConsoleManager::getInstance()->exitApplication();
        }
        else if (command == "scheduler-start") {
            if (!Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << GREEN << "Scheduler Test now running" << RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(true);
                // create batchProcessFrequency number of processes
                std::thread schedulerTestThread([&] {
					ConsoleManager::getInstance()->schedulerTest();
                    });
                schedulerTestThread.detach();
               
            }
            else {
                cout << YELLOW << "Scheduler Test already running" << RESET << endl;
            }
        }
        else if (command == "scheduler-stop") {
            if (Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << RED << "Scheduler Test stopped" << RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(false);
            }
            else {
                cout << RED << "Scheduler Test not running" << RESET << endl;
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
        else if (command == "screen") {
            if (tokens.size() > 1) {
                string screenCommand = tokens[1];
                string processName = (tokens.size() > 2) ? tokens[2] : "";

                if (screenCommand == "-s" && !processName.empty()) {
                    if (ConsoleManager::getInstance()->getScreenMap().contains(processName)) {
                        cout << RED << "Screen already exists." << RESET << endl;
                    }
                    else {
                        string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
                        auto screenInstance = std::make_shared<Screen>(processName, 0, timestamp);
                        ConsoleManager::getInstance()->registerConsole(screenInstance);

                        ConsoleManager::getInstance()->switchConsole(processName);
                        ConsoleManager::getInstance()->drawConsole();
                        Scheduler::getInstance()->addProcessToQueue(screenInstance);
                    }
                }
                else if (screenCommand == "-r" && !processName.empty()) {
                    //cin >> processName;

                    //// Check if screen exists before switching
                    auto screenMap = ConsoleManager::getInstance()->getScreenMap();
                    if (screenMap.find(processName) != screenMap.end()) {
                        ConsoleManager::getInstance()->switchConsole(processName);
                        ConsoleManager::getInstance()->drawConsole();
                    }
                    else {
                        cout << RED << "Error: Screen with name '" << processName << "' does not exist or was not initialized." << RESET << endl;
                    }
                }
                else if (screenCommand == "-ls") {
                    ConsoleManager::getInstance()->displayProcessList();
                }
                else {
                    cout << RED << "Command not recognized." << RESET << endl;
                }
            }
            else {
                cout << RED << "Command not recognized." << RESET << endl;
            }
        }
        else {
            cout << RED << "Command not recognized." << RESET << endl;
        }
    }
    else {
        // Process-specific commands
        if (command == "exit") {
            ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
        }
        else if (command == "process-smi") {
            ConsoleManager::getInstance()->printProcessSmi();
        }
        else {
            cout << RED << "Command not recognized." << RESET << endl;
        }
    }
}

    
