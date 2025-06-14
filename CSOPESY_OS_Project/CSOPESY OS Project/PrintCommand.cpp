#include "PrintCommand.h"
#include <iostream>

PrintCommand::PrintCommand(int pid, std::string text) : ICommand(pid, CommandType::PRINT)
{
	this->toPrint = text;
}

void PrintCommand::execute()
{
	ICommand::execute();

	std::cout << "PID: " << pid << "  Log: " << toPrint;

}
