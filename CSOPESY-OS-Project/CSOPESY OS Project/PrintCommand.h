#pragma once

#include "ICommand.h"
#include <string>
class PrintCommand : public ICommand
{
public:
	PrintCommand(int pid, std::string text);
	void execute() override;

private:
	std::string toPrint;
};

