#include "ProcessScreen.h"


ProcessScreen::ProcessScreen(string name)
{
	this->name = name;
}

ProcessScreen::ProcessScreen() {

}

string ProcessScreen::getConsoleName(){
	return this->name;
}

string ProcessScreen::getProcessName()
{
	return string();
}

int ProcessScreen::getCurrentLine()
{
	return 0;
}

int ProcessScreen::getTotalLine()
{
	return 0;
}

string ProcessScreen::getTimestamp()
{
	return string();
}
