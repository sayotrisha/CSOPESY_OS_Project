using namespace std;

#include "InputManager.h"
#include <iostream>
#include "ConsoleManager.h"
#include "Screen.h"

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
	cout << "Enter a command: ";
    string input;
	cin >> input;

	for (int i = 0; i < input.length(); i++) {
		input[i] = tolower(input[i]);
	}

    if (ConsoleManager::getInstance()->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
        if (input == "initialize") {
            cout << "'initialize' command recognized. Doing something." << endl;
        }
        else if (input == "scheduler-test") {
            cout << "'scheduler-test' command recognized. Doing something." << endl;
        }
        else if (input == "scheduler-stop") {
            cout << "'scheduler-stop' command recognized. Doing something." << endl;
        }
        else if (input == "report-util") {
            cout << "'report-util' command recognized. Doing something." << endl;
        }
        else if (input == "clear") {
            system("cls");
        }
        else if (input == "screen") {
            string screenCommand;
            string processName;
            cin >> screenCommand;

            if (screenCommand == "-s") {

                cin >> processName;

				if (ConsoleManager::getInstance()->getScreenMap().contains(processName)) {
					cout << "Screen already exists." << endl;
				}
                else {
                    string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
                    std::shared_ptr<Screen> screenInstance = std::make_shared<Screen>(processName, 1, 10, timestamp);
                    ConsoleManager::getInstance()->registerConsole(screenInstance);

                    ConsoleManager::getInstance()->switchConsole(processName);
                    ConsoleManager::getInstance()->drawConsole();
                }
            }
            else if (screenCommand == "-r") {
                cin >> processName;

                // Check if screen exists before switching
                auto screenMap = ConsoleManager::getInstance()->getScreenMap();
                if (screenMap.find(processName) != screenMap.end()) {
                    ConsoleManager::getInstance()->switchConsole(processName);
                    ConsoleManager::getInstance()->drawConsole();
                }
                else {
                    cout << "Error: Screen with name '" << processName << "' does not exist or was not initialized." << endl;
                }
            }
            else if (screenCommand == "-ls") {
				ConsoleManager::getInstance()->displayProcessList();
			}
			else {
				cout << "Command not recognized." << endl;
			}
        }
        else if (input == "print") {
            cout << "Process name: ";
            string enteredProcess;
            cin >> enteredProcess;
            if (enteredProcess.empty()) { // if the process name is empty
                std::cout << "Command not recognized! Please provide a process name." << std::endl;
            }
            ConsoleManager::getInstance()->printProcess(enteredProcess);
        }


        else if (input == "exit") {
            ConsoleManager::getInstance()->exitApplication();
		}
		else {
			cout << "Command not recognized." << endl;
		}
    } 

    // screen is at process
    else {
        if (input == "exit") {
            ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
        }
    }

    
}
    
