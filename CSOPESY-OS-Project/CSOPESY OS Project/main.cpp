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

