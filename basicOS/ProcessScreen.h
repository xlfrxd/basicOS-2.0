#pragma once

#include <string>

using namespace std;

class ProcessScreen {
public:

	ProcessScreen(string name);
	ProcessScreen();

	virtual string getConsoleName(); 
	virtual string getProcessName();   
	virtual int getCurrentLine();       
	virtual int getTotalLine();     
	virtual string getTimestamp(); 


private:
	string name;
};
