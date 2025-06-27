/**
 *  PrintCommand.cpp
 *
 *  Purpose:
 *      This file implements the PrintCommand class, which inherits from ICommand.
 *      It defines behavior for print-type commands that display text output
 *      to the console, along with the process ID executing it.
 *
 *      This command is part of a command execution system used by simulated
 *      processes in the console-based OS.
 */
#include "PrintCommand.h"
#include <iostream>

 /*----------------------------------------------------------------------
  |  Function PrintCommand(int pid, std::string text)
  |
  |  Purpose:  Constructor for the PrintCommand class. Initializes the command
  |      with a process ID and the text to be printed. Also sets the command
  |      type to PRINT by passing it to the base class (ICommand).
  |
  |  Parameters:
  |      pid (IN) -- The ID of the process that owns or executes this command.
  |      text (IN) -- The string message that will be printed when the command is executed.
  |
  |  Returns:  Nothing (constructor)
  *-------------------------------------------------------------------*/
PrintCommand::PrintCommand(int pid, std::string text) : ICommand(pid, CommandType::PRINT)
{
	this->toPrint = text;
}

/*---------------------------------------------------------------------
 |  Function execute()
 |
 |  Purpose:  Executes the print command. This involves calling the base class
 |      execution logic (if any) and printing the associated message
 |      along with the process ID to the standard output stream.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void PrintCommand::execute()
{
	ICommand::execute();

	std::cout << "PID: " << pid << "  Log: " << toPrint;

}
