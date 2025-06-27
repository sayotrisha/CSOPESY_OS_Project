/**
 *  MainScreen.cpp
 *
 *  Purpose:
 *      This file implements the MainScreen class, a subclass of BaseScreen.
 *      It defines the constructors for creating a screen object that
 *      represents the main console. This screen acts as the central hub
 *      for user interaction and system control.
 */
#include "MainScreen.h"
#include "BaseScreen.h"

 /*--------------------------------------------------------------------
  |  Function MainScreen(string name)
  |
  |  Purpose:  Constructor that creates a MainScreen instance with a specified name.
  |      It passes the name to the BaseScreen constructor, allowing the main screen
  |      to be uniquely identified.
  |
  |  Parameters:
  |      name (IN) -- A string representing the name assigned to this screen instance.
  |
  |  Returns:  Nothing (constructor)
  *-------------------------------------------------------------------*/
MainScreen::MainScreen(string name) : BaseScreen(name)
{
	
}

/*--------------------------------------------------------------------
 |  Function MainScreen()
 |
 |  Purpose:  Default constructor for MainScreen. Allows the creation of a main screen
 |      without assigning a name during instantiation.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing (constructor)
 *-------------------------------------------------------------------*/
MainScreen::MainScreen()
{
}


