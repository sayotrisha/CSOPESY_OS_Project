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

namespace fs = std::filesystem; // alias for convenience

Screen::Screen(string processName, int currentLine, string timestamp, size_t memoryRequired)
	: BaseScreen(processName), cpuCoreID(-1), commandCounter(0), currentState(ProcessState::READY)
{
	this->processName = processName;
	this->currentLine = currentLine;
	this->setRandomIns();
	this->memoryRequired = memoryRequired;
	this->timestamp = timestamp;

	this->numPages = ConsoleManager::getInstance()->getNumPages();

	// create 100 print commands
	for (int i = 0; i < totalLine; i++) {
		this->printCommands.push_back(PrintCommand(i, "Printing from " + processName + " " + std::to_string(i)));

	}


}

void Screen::setMemoryUsage(size_t memoryUsage)
{
	this->memoryUsage = memoryUsage;
}

size_t Screen::getMemoryUsage() const
{
	return this->memoryUsage;
}

void Screen::setIsRunning(bool isRunning)
{
	this->isRunning = isRunning;
}

bool Screen::getIsRunning() const
{
	return this->isRunning;
}

void Screen::setRandomIns() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinIns(), ConsoleManager::getInstance()->getMaxIns());
	this->totalLine = dis(gen);
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

size_t Screen::getNumPages()
{
	return this->numPages;
}

void Screen::setNumPages(size_t numPages)
{
	this->numPages = numPages;
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
	file << "Hello world from " << this->processName << "!" << std::endl;
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

size_t Screen::getMemoryRequired() const
{
	return this->memoryRequired;
}

void Screen::setMemoryRequired(size_t memoryRequired)
{
	this->memoryRequired = memoryRequired;
}