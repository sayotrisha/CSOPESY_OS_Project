#pragma once
#include "BaseScreen.h"
#include "PrintCommand.h"
#include <vector>
#include <fstream>

using namespace std;

#include <iostream>
class Screen : public BaseScreen
{
public:
	Screen(string processName, int currentLine, string timestamp, size_t memoryRequired);
	~Screen();

	enum ProcessState
	{
		READY,
		RUNNING,
		WAITING,
		FINISHED
	};


	void setProcessName(string processName);
	void setCurrentLine(int currentLine);
	void setTotalLine(int totalLine);
	void setTimestamp(string timestamp);
	void setTimestampFinished(string timestampFinished);
	void setMemoryRequired(size_t memoryRequired);


	void executeCurrentCommand();
	void moveToNextLine();
	bool isFinished() const;
	int getCommandCounter() const;
	int getCPUCoreID() const;
	void setCPUCoreID(int coreID);
	void createFile();
	void viewFile();
	void setRandomIns();
	size_t getMemoryRequired() const;

	ProcessState getState() const;

	string getProcessName() override;
	int getCurrentLine() override;
	int getTotalLine() override;
	string getTimestamp() override;
	string getTimestampFinished();

private:
	string processName;
	int currentLine;
	int totalLine;
	int commandCounter;
	int cpuCoreID = -1;
	string timestamp;
	string timestampFinished;
	std::vector<PrintCommand> printCommands;
	ProcessState currentState;
	size_t memoryRequired;

};
