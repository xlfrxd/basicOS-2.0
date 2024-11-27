#include "Instruction.h"

Instruction::Instruction(int pid, CommandType commandType)
{
	this->pid = pid;
	this->commandType = commandType;
}

Instruction::CommandType Instruction::getCommandType()
{
	return commandType;
}

void Instruction::execute()
{
	// Do nothing
}
