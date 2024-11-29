#pragma once
class Instruction
{
public:
	enum CommandType
	{
		IO,
		PRINT
	};

	Instruction(int pid, CommandType commandType);
	CommandType getCommandType();
	virtual void execute();

protected:
	int pid;
	CommandType commandType;
};


