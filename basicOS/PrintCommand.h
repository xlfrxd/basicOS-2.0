#pragma once

#include "Instruction.h"
#include <string>
class PrintCommand : public Instruction
{
public:
	PrintCommand(int pid, std::string text);
	void execute() override;

private:
	std::string toPrint;
};

