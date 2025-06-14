#pragma once
#include "Screen.h"
#include <unordered_map>
#include "Scheduler.h"
#include <string>

const string MAIN_CONSOLE = "MAIN_CONSOLE";

class ConsoleManager
{	
public:
	// default constructor
	ConsoleManager();

	// we can run this to create a console manager pointer
	static void initialize();

	// we get the console manager instance after 
	static ConsoleManager* getInstance();

	void printHeader();
	void drawConsole();
	void destroy();
	string getCurrentTimestamp();
	std::unordered_map<string, std::shared_ptr<BaseScreen>> getScreenMap();

	void registerConsole(std::shared_ptr<BaseScreen> screenRef);
	void switchConsole(string consoleName);
	std::shared_ptr<BaseScreen> getCurrentConsole();

	void exitApplication();
	bool isRunning();
	void setCurrentConsole(std::shared_ptr<BaseScreen> screenRef);
	void displayProcessList();
	void printProcess(string enteredProcess);


private:
	// stores the console manager pointer so that we only instantiate once
	static ConsoleManager* consoleManager;
	bool running = true;
	bool switchSuccessful = true;

	// declare consoles 
	std::shared_ptr<BaseScreen> currentConsole;
	std::unordered_map<string, std::shared_ptr<BaseScreen>> screenMap;

	Scheduler scheduler;
};