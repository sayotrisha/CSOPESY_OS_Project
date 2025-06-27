/**
 *  File name: BaseScreen.cpp
 *
 *  Purpose:
 *      This file defines the BaseScreen class, which serves as an abstract
 *      foundation for different types of screen objects in the console-based
 *      operating system simulation. It provides a basic interface and shared
 *      functionality such as storing and retrieving the screen's name.
 *
 *      Derived classes (e.g., Screen, MainScreen) extend BaseScreen to implement
 *      specific behavior for process visualization, logging, and user interaction.
 *
 *      This abstraction enables the ConsoleManager to manage various screen types
 *      in a uniform way, promoting extensibility and modularity in the system design.
 */
#include "BaseScreen.h"

/*--------------------------------------------------------------------
 |  Function: BaseScreen(string name)
 |
 |  Purpose:  Constructor for the BaseScreen class that sets the name
 |      of the screen. This helps identify or label different
 |      screen instances in the console system.
 |
 |  Parameters:
 |      name (IN) -- The string identifier for this screen instance.
 |
 |  Returns:  Nothing (constructor).
 *-------------------------------------------------------------------*/
BaseScreen::BaseScreen(string name)
{
	this->name = name;
}

/*---------------------------------------------------------------------
 |  Function: BaseScreen()
 |
 |  Purpose:  Default constructor for the BaseScreen class. Allows for
 |      creation of a screen object without initializing its name.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing (constructor).
 *-------------------------------------------------------------------*/
BaseScreen::BaseScreen() {

}

/*---------------------------------------------------------------------
 |  Function getConsoleName()
 |
 |  Purpose:  Returns the name of the console/screen.
 |
 |  Parameters: None
 |
 |  Returns:  string -- the name assigned to the screen.
 |
 |  Comments: draw main console here
 *-------------------------------------------------------------------*/
string BaseScreen::getConsoleName(){
	return this->name;
}

/*----------------------------------------------------------------------
 |  Function getProcessName()
 |
 |  Purpose:  Virtual method intended to be overridden by derived classes
 |      like Screen. In the base class, it returns an empty string.
 |
 |  Parameters: None
 |
 |  Returns:  string -- the process name (empty by default).
 *-------------------------------------------------------------------*/
string BaseScreen::getProcessName()
{
	return string();
}

/*---------------------------------------------------------------------
 |  Function getCurrentLine()
 |
 |  Purpose:  Virtual method to retrieve the current line of execution
 |      in the screen or process. Returns 0 by default.
 |
 |  Parameters: None
 |
 |  Returns:  int -- current line number (default is 0).
 *-------------------------------------------------------------------*/
int BaseScreen::getCurrentLine()
{
	return 0;
}

/*----------------------------------------------------------------------
 |  Function getTotalLine()
 |
 |  Purpose:  Virtual method to get the total number of lines/instructions
 |      that the process or screen should execute. Returns 0 by default.
 |
 |  Parameters: None
 |
 |  Returns:  int -- total line count (default is 0).
 *-------------------------------------------------------------------*/
int BaseScreen::getTotalLine()
{
	return 0;
}

/*------------------------------------------------- getTimestamp -----
 |  Function getTimestamp()
 |
 |  Purpose:  Virtual method to return the timestamp when the screen or
 |      process was created or initialized. Returns an empty string by default.
 |
 |  Parameters: None
 |
 |  Returns:  string -- timestamp (empty by default).
 *-------------------------------------------------------------------*/
string BaseScreen::getTimestamp()
{
	return string();
}
