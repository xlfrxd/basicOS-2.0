#pragma once
#include <iostream>

class InputHandler
{
public:
	InputHandler();
	static void initialize();
	static InputHandler* getInstance();
	void handleMainConsoleInput();
	void destroy();

private:
	static InputHandler* inputHandler;
};

