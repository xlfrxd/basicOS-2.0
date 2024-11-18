#pragma once
#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>
#include <random>
#include <chrono>
#include "Instruction.h"

class Process {
public:
    Process(const std::string& pid, int burst);

    void generateInstructions();
    bool executeInstruction();
    std::chrono::milliseconds getInstructionTime();
    std::string getTimeStamp() const;

    std::string id;                    // Process ID
    int burst_time;                    // Total burst time
    int progress;                      // Current progress in burst time
    int core_id;                       // Core assigned to the process
    std::chrono::system_clock::time_point arrival_time;
    std::mt19937 rng;                  // Random number generator
    std::vector<Instruction> instructions; // List of instructions for the process
    int current_instruction;           // Current instruction index
    int cycles_until_next_instruction; // Remaining cycles for current instruction
    bool instruction_completed;        // Whether the current instruction was completed
    size_t memoryAllocated;            // Memory allocated to the process
    void* memoryPtr;                   // Pointer to allocated memory for the process
};

#endif
