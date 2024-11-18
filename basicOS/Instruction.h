#pragma once
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

enum class InstructionType {
    PRINT,
    SLEEP
};

struct Instruction {
    InstructionType type;
    int cycles;  // For SLEEP instructions, number of cycles to sleep

    Instruction(InstructionType type, int cycles = 0)
        : type(type), cycles(cycles) {}
};

#endif
