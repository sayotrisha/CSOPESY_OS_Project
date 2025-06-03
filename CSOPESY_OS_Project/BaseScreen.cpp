#include "BaseScreen.h"


BaseScreen::BaseScreen(string name)
{
	this->name = name;
}

BaseScreen::BaseScreen() {

}

//draw main console here
string BaseScreen::getConsoleName() {
	return this->name;
}

string BaseScreen::getProcessName()
{
	return string();
}

int BaseScreen::getCurrentLine()
{
	return 0;
}

int BaseScreen::getTotalLine()
{
	return 0;
}

string BaseScreen::getTimestamp()
{
	return string();
}