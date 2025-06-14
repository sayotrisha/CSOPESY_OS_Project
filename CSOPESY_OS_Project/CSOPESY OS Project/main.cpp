/**
*  CSOPESY_OS_Project.cpp : This file contains the 'main' function. Program execution begins and ends there.
*  CSOPESY - S19 (Group 10)
*  Members:
*          - MARTIN, ELAINE RIZ COCAL
*          - ORDINARIO, KARL ANDREI LARITA
*          - SAYO, TRISHA ALISSANDRA
*
*  Submitted to: Mr. DE LA CRUZ, REN TRISTAN A.
**/

using namespace std;

#include <iostream>
#include <cctype>
#include <vector>
#include <ctime>
#include "ConsoleManager.h"
#include "InputManager.h"
#include "BaseScreen.h"
#include "MainScreen.h"
#include "Scheduler.h"
#include <fstream>
#include <random>



int main()
{
    ConsoleManager::initialize();
	InputManager::initialize();

    Scheduler::initialize(4);

    // Creating 10 processes with randomized total line of instructions
    for (int i = 0; i < 10; i++) {
        // Create a random number generator
        std::random_device rd;  // Seed
        std::mt19937 gen(rd()); // Mersenne Twister random number generator
        std::uniform_int_distribution<> dis(50, 100); // Random distribution between 50 and 100 lines

        // Generate random number of lines between 50 and 100
        int randLine = dis(gen);

        string processName = "Process" + to_string(i);
        shared_ptr<BaseScreen> processScreen = make_shared<Screen>(processName, 0, randLine, ConsoleManager::getInstance()->getCurrentTimestamp());

        ConsoleManager::getInstance()->registerConsole(processScreen);

        // Cast processScreen to shared_ptr<Screen>
        shared_ptr<Screen> screenPtr = static_pointer_cast<Screen>(processScreen);

        // Add the process to the scheduler queue
        Scheduler::getInstance()->addProcessToQueue(screenPtr);
    }


  //  // create 10 processes each with 100 commands
  //  for (int i = 0; i < 10; i++) {
  //      string processName = "Process" + to_string(i);
  //      shared_ptr<BaseScreen> processScreen = make_shared<Screen>(processName, 0, 100, ConsoleManager::getInstance()->getCurrentTimestamp());
  //      ConsoleManager::getInstance()->registerConsole(processScreen);
  //      // Cast processScreen to shared_ptr<Screen>
  //      shared_ptr<Screen> screenPtr = static_pointer_cast<Screen>(processScreen);
		//Scheduler::getInstance()->addProcessToQueue(screenPtr);
  //  }

    std::thread schedulerThread([&] {
        Scheduler::getInstance()->start();
        });
    schedulerThread.detach();
    
    // register main screen
    shared_ptr<BaseScreen> mainScreen = make_shared<MainScreen>(MAIN_CONSOLE);

    ConsoleManager::getInstance()->registerConsole(mainScreen);
    ConsoleManager::getInstance()->setCurrentConsole(mainScreen);
    
    bool running = true;
    ConsoleManager::getInstance()->drawConsole();

    while (running){
        InputManager::getInstance()->handleMainConsoleInput();
        running = ConsoleManager::getInstance()->isRunning();
    }
    
	InputManager::getInstance()->destroy();
    ConsoleManager::getInstance()->destroy();

    return 0;
}

