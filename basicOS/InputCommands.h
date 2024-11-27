#pragma once
class InputCommands
{
public:
	InputCommands();
	static void initialize();
	static InputCommands* getInstance();
	void handleMainConsoleInput();
	void destroy();

private:
	static InputCommands* inputCommands;
};

