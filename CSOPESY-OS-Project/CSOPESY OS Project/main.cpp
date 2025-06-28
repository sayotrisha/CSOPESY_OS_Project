/**
*  CSOPESY_OS_Project.cpp : This file contains the 'main' function. Program execution begins and ends there.
*  CSOPESY - S19 (Group 10)
*  Members:
*          - MARTIN, ELAINE RIZ COCAL
*          - ORDINARIO, KARL ANDREI LARITA
*          - SAYO, TRISHA ALISSANDRA
*
*  Submitted to: Mr. DE LA CRUZ, REN TRISTAN A.
*  main.cpp
*
*  Purpose:
*      This file contains the main entry point of the console-based operating
*      system simulation. It initializes key system components such as the
*      ConsoleManager and InputManager, registers the main console screen,
*      and starts the input-handling loop for user interaction.
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
#include "Colors.h"

/*--------------------------------------------------------------------
 |  Function main()
 |
 |  Purpose:  Entry point of the simulation. Sets up the ConsoleManager
 |      and InputManager, registers the main screen, and enters a loop to
 |      continuously handle user input until the application exits.
 |
 |  Parameters: None
 |
 |  Returns:  int -- exit status (0 indicates normal program termination)
 *-------------------------------------------------------------------*/
int main()
{
    ConsoleManager::initialize();
	InputManager::initialize();
    
    // register main screen
    shared_ptr<BaseScreen> mainScreen = make_shared<MainScreen>(MAIN_CONSOLE);

    ConsoleManager::getInstance()->registerConsole(mainScreen);
    ConsoleManager::getInstance()->setCurrentConsole(mainScreen);
    
    bool running = true;
    ConsoleManager::getInstance()->drawConsole();
    ConsoleManager::getInstance()->printMarquee();

    while (running){
        InputManager::getInstance()->handleMainConsoleInput();
        running = ConsoleManager::getInstance()->isRunning();
    }
    
	InputManager::getInstance()->destroy();
    ConsoleManager::getInstance()->destroy();

    return 0;
}

