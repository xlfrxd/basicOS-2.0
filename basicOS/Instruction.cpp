#include "Instruction.h"
#include <iostream>
#include <chrono>
#include <thread>

void executeInstruction(const Instruction& instruction) {
    switch (instruction.type) {
    case InstructionType::PRINT:
        std::cout << "Executing PRINT instruction.\n";
        break;
    case InstructionType::SLEEP:
        std::this_thread::sleep_for(std::chrono::milliseconds(instruction.cycles));
        std::cout << "SLEEP for " << instruction.cycles << " cycles completed.\n";
        break;
    default:
        std::cerr << "Unknown instruction type.\n";
        break;
    }
}
