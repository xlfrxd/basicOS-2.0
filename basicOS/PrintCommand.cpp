#include <iostream>

#include "PrintCommand.h"

PrintCommand::PrintCommand(int pid, std::string text) : Instruction(pid, CommandType::PRINT)
{
	this->toPrint = text;
}

void PrintCommand::execute()
{
	Instruction::execute();

	std::cout << "PID: " << pid << "  Log: " << toPrint;

}
