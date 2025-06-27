/**
 *  Screen.cpp
 *
 *  Purpose:
 *      Implements the Screen class, which represents a process in the simulated
 *      console operating system. Inherits from BaseScreen and adds support for
 *      tracking instruction execution, command queues, timestamps, CPU core
 *      assignment, and file logging.
 */
#include "Screen.h"
#include <fstream> 
#include <filesystem> // for folder creation
#include "PrintCommand.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
#include "ConsoleManager.h"

using namespace std;

//namespace fs = std::filesystem; // alias for convenience

/*---------------------------------------------------------------------
 |  Function Screen(string processName, int currentLine, string timestamp)
 |
 |  Purpose:  Initializes a new Screen (process) with a name, starting instruction line,
 |      and creation timestamp. Randomly generates a set number of print commands.
 |
 |  Parameters:
 |      processName (IN) -- the name/identifier of the process.
 |      currentLine (IN) -- the starting line of instruction.
 |      timestamp (IN) -- the time the process was created.
 |
 |  Returns:  Nothing (constructor)
 *-------------------------------------------------------------------*/
Screen::Screen(string processName, int currentLine, string timestamp)
    : BaseScreen(processName), cpuCoreID(-1), commandCounter(0), currentState(ProcessState::READY)
{
    this->processName = processName;
    this->currentLine = currentLine;
	this->setRandomIns();
    this->timestamp = timestamp;


	// create 100 print commands
	for(int i=0; i<totalLine; i++){
		this->printCommands.push_back(PrintCommand(i, "Printing from " + processName + " " + std::to_string(i)));

	}
	
}

/*---------------------------------------------------------------------
 |  Function setRandomIns()
 |
 |  Purpose:  Generates a random number of instructions (totalLine) for this process
 |      based on min/max values from ConsoleManager configuration.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setRandomIns() {
	std::random_device rd;
	std::mt19937 gen(rd()); 
	std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinIns(), ConsoleManager::getInstance()->getMaxIns()); 
	this->totalLine = dis(gen);
}

/*---------------------------------------------------------------------
 |  Function ~Screen()
 |
 |  Purpose:  Destructor for the Screen class. Currently empty since
 |      smart pointers and RAII handle memory management.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/ 
Screen::~Screen()
{
}

/*---------------------------------------------------------------------
 |  Function setProcessName(string processName)
 |
 |  Purpose:  Sets the name of the process.
 |
 |  Parameters:
 |      processName (IN) -- name to assign to the process.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setProcessName(string processName)
{
	this->processName = processName;
}

/*---------------------------------------------------------------------
 |  Function setCurrentLine(int currentLine)
 |
 |  Purpose:  Sets the current instruction line for the process.
 |
 |  Parameters:
 |      currentLine (IN) -- line number currently being executed.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setCurrentLine(int currentLine)
{
	this->currentLine = currentLine;
}

/*-----------------------------------------------------------------------
 |  Function setTotalLine(int totalLine)
 |
 |  Purpose:  Sets the total number of lines/instructions for the process.
 |
 |  Parameters:
 |      totalLine (IN) -- total instruction count.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setTotalLine(int totalLine)
{
	this->totalLine = totalLine;
}

/*---------------------------------------------------------------------
 |  Function setTimestamp(string timestamp)
 |
 |  Purpose:  Sets the creation timestamp for the process.
 |
 |  Parameters:
 |      timestamp (IN) -- formatted date/time string.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

/*---------------------------------------------------------------------
 |  Function getProcessName()
 |
 |  Purpose:  Retrieves the name of the process.
 |
 |  Parameters: None
 |
 |  Returns:  string -- name of the process.
 *-------------------------------------------------------------------*/
string Screen::getProcessName()
{
	return this->processName;
}

/*---------------------------------------------------------------------
 |  Function getCurrentLine()
 |
 |  Purpose:  Gets the current instruction line being executed.
 |
 |  Parameters: None
 |
 |  Returns:  int -- current instruction line.
 *-------------------------------------------------------------------*/
int Screen::getCurrentLine()
{
	return this->currentLine;
}

/*---------------------------------------------------------------------
 |  Function getTotalLine()
 |
 |  Purpose:  Gets the total number of instructions in this process.
 |
 |  Parameters: None
 |
 |  Returns:  int -- total instruction count.
 *-------------------------------------------------------------------*/
int Screen::getTotalLine()
{
	return this->totalLine;
}

/*---------------------------------------------------------------------
 |  Function getTimestamp()
 |
 |  Purpose:  Retrieves the creation timestamp of the process.
 |
 |  Parameters: None
 |
 |  Returns:  string -- process creation time.
 *-------------------------------------------------------------------*/
string Screen::getTimestamp()
{
	return this->timestamp;
}

/*---------------------------------------------------------------------
 |  Function getTimestampFinished()
 |
 |  Purpose:  Retrieves the timestamp when the process finished execution.
 |
 |  Parameters: None
 |
 |  Returns:  string -- finish time of the process.
 *-------------------------------------------------------------------*/
string Screen::getTimestampFinished() {
	return timestampFinished;
}

/*----------------------------------------------------------------------
 |  Function setTimestampFinished(string timestampFinished)
 |
 |  Purpose:  Sets the time at which the process finished execution.
 |
 |  Parameters:
 |      timestampFinished (IN) -- formatted finish time.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setTimestampFinished(string timestampFinished) {
	this->timestampFinished = timestampFinished;
}

/*----------------------------------------------------------------------
 |  Function executeCurrentCommand()
 |
 |  Purpose:  Executes the current print command associated with this process.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::executeCurrentCommand() 
{
	this->printCommands[this->currentLine].execute();
}

/*---------------------------------------------------------------------
 |  Function moveToNextLine()
 |
 |  Purpose:  Moves to the next instruction line.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::moveToNextLine()
{
	this->currentLine++;
}

/*-------------------------------------------------------------------
 |  Function isFinished()
 |
 |  Purpose:  Determines whether the process has finished executing all instructions.
 |
 |  Parameters: None
 |
 |  Returns:  bool -- true if current line >= total lines; false otherwise.
 *-------------------------------------------------------------------*/
bool Screen::isFinished() const
{
	return this->currentLine >= this->totalLine;
}

/*---------------------------------------------------------------------
 |  Function getCommandCounter()
 |
 |  Purpose:  Returns how many commands have been executed. (Currently unused counter.)
 |
 |  Parameters: None
 |
 |  Returns:  int -- the number of commands executed.
 *-------------------------------------------------------------------*/
int Screen::getCommandCounter() const
{
	return this->commandCounter;
}

/*--------------------------------------------------------------------
 |  Function getCPUCoreID()
 |
 |  Purpose:  Returns the ID of the CPU core assigned to this process.
 |
 |  Parameters: None
 |
 |  Returns:  int -- assigned core ID; -1 if unassigned.
 *-------------------------------------------------------------------*/
int Screen::getCPUCoreID() const
{
	return this->cpuCoreID;
}

/*------------------------------------------------------------------
 |  Function getState()
 |
 |  Purpose:  Returns the current state of the process (e.g., READY, RUNNING).
 |
 |  Parameters: None
 |
 |  Returns:  ProcessState -- the current process state.
 *-------------------------------------------------------------------*/
Screen::ProcessState Screen::getState() const
{
	return this->currentState;
}

/*----------------------------------------------------------------
 |  Function setCPUCoreID(int coreID)
 |
 |  Purpose:  Assigns a specific CPU core ID to this process.
 |
 |  Parameters:
 |      coreID (IN) -- ID of the core that will execute this process.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::setCPUCoreID(int coreID)
{
	this->cpuCoreID = coreID;
}

/*--------------------------------------------------------------------
 |  Function createFile()
 |
 |  Purpose:  Creates a file named after the process and writes a greeting message
 |      to simulate logging or output generation.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::createFile()
{
	string fileName = this->processName + ".txt";
	fstream file;
	//create file if it doesnt exit
	file.open(fileName, std::ios::out | std::ios::trunc);
	file << "Hello world from "<< this->processName << "!" << std::endl;
	file.close();
}

/*--------------------------------------------------------------------
 |  Function viewFile()
 |
 |  Purpose:  Opens and displays the content of the file created by this process.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void Screen::viewFile()
{
	string fileName = this->processName + ".txt";
	fstream file;
	file.open(fileName, std::ios::in);
	std::string line;
	while (std::getline(file, line)) {
		cout << "\"";
		cout << line;
		cout << "\"" << endl;
	}
	file.close();
}