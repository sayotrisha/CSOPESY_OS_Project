#include "ICommand.h"

ICommand::ICommand(int pid, CommandType commandType)
{
	this->pid = pid;
	this->commandType = commandType;
}

ICommand::CommandType ICommand::getCommandType()
{
	return commandType;
}

void ICommand::execute()
{
	// Do nothing
}
