#pragma once
#include <string>

using namespace std;

class BaseScreen {
public:
	BaseScreen(string name);
	BaseScreen();

	virtual string getConsoleName();
	virtual string getProcessName();
	virtual int getCurrentLine();
	virtual int getTotalLine();
	virtual string getTimestamp();


private:
	string name;
};