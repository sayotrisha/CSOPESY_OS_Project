#include "Screen.h"
#include <fstream> 
#include <filesystem> // for folder creation
#include "PrintCommand.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

using namespace std;

//namespace fs = std::filesystem; // alias for convenience

Screen::Screen(string processName, int currentLine, int totalLine, string timestamp)
    : BaseScreen(processName), cpuCoreID(-1), commandCounter(0), currentState(ProcessState::READY)
{
    this->processName = processName;
    this->currentLine = currentLine;
    this->totalLine = totalLine;
    this->timestamp = timestamp;


	// create 100 print commands
	for(int i=0; i<totalLine; i++){
		this->printCommands.push_back(PrintCommand(i, "Printing from " + processName + " " + std::to_string(i)));

	}

	/*
	for(int i = 0; i < this->printCommands.size(); i++){
		this->printCommands[i].execute();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}*/
	
}


Screen::~Screen()
{
}

void Screen::setProcessName(string processName)
{
	this->processName = processName;
}

void Screen::setCurrentLine(int currentLine)
{
	this->currentLine = currentLine;
}

void Screen::setTotalLine(int totalLine)
{
	this->totalLine = totalLine;
}

void Screen::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

string Screen::getProcessName()
{
	return this->processName;
}

int Screen::getCurrentLine()
{
	return this->currentLine;
}

int Screen::getTotalLine()
{
	return this->totalLine;
}

string Screen::getTimestamp()
{
	return this->timestamp;
}

string Screen::getTimestampFinished() {
	return timestampFinished;
}

void Screen::setTimestampFinished(string timestampFinished) {
	this->timestampFinished = timestampFinished;
}


void Screen::executeCurrentCommand() 
{
	this->printCommands[this->currentLine].execute();
}

void Screen::moveToNextLine()
{
	this->currentLine++;
}

bool Screen::isFinished() const
{
	return this->currentLine >= this->totalLine;
}

int Screen::getCommandCounter() const
{
	return this->commandCounter;
}

int Screen::getCPUCoreID() const
{
	return this->cpuCoreID;
}

Screen::ProcessState Screen::getState() const
{
	return this->currentState;
}

void Screen::setCPUCoreID(int coreID)
{
	this->cpuCoreID = coreID;
}

void Screen::createFile()
{
	string fileName = this->processName + ".txt";
	fstream file;
	//create file if it doesnt exit
	file.open(fileName, std::ios::out | std::ios::trunc);
	file << "Hello world from "<< this->processName << "!" << std::endl;
	file.close();
}

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

