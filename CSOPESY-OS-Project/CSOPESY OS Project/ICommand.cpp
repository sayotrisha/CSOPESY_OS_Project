/**
 *  ICommand.cpp
 *
 *  Purpose:
 *      This file implements the ICommand class, which serves as an abstract base
 *      class for defining commands that can be executed by a process or screen.
 *      It provides a common interface and basic functionality such as storing
 *      a process ID and command type.
 *
 *      Derived classes (e.g., PrintCommand) extend this class to implement
 *      specific command behaviors.
 */
#include "ICommand.h"

 /*---------------------------------------------------------------------
  |  Function ICommand(int pid, CommandType commandType)
  |
  |  Purpose:  Constructor for the ICommand class. Initializes a command with
  |      a given process ID and command type (e.g., PRINT).
  |
  |  Parameters:
  |      pid (IN) -- The ID of the process that will execute this command.
  |      commandType (IN) -- The type of command (enum), used for classification.
  |
  |  Returns:  Nothing (constructor)
  *-------------------------------------------------------------------*/
ICommand::ICommand(int pid, CommandType commandType)
{
	this->pid = pid;
	this->commandType = commandType;
}

/*---------------------------------------------------------------------
 |  Function getCommandType()
 |
 |  Purpose:  Retrieves the type of this command (e.g., PRINT).
 |      Useful for identifying what kind of operation the command performs.
 |
 |  Parameters: None
 |
 |  Returns:  CommandType -- enumeration value indicating the command type.
 *-------------------------------------------------------------------*/
ICommand::CommandType ICommand::getCommandType()
{
	return commandType;
}

/*-------------------------------------------------------------------
 |  Function execute()
 |
 |  Purpose:  Base implementation of the execute method for ICommand.
 |      Intended to be overridden by derived classes to perform
 |      specific command behavior (e.g., printing to screen).
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ICommand::execute()
{
	// Do nothing
}