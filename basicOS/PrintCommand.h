#pragma once

#include <string>

#include "Instruction.h"

class PrintCommand : public Instruction
{
public:
	PrintCommand(int pid, std::string text);
	void execute() override;

private:
	std::string toPrint;
};

