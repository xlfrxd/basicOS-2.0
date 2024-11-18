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

    // Getters
    const std::string& getId() const;
    int getBurstTime() const;
    int getProgress() const;
    int getCoreId() const;
    const std::vector<Instruction>& getInstructions() const;
    int getCurrentInstruction() const;
    int getCyclesUntilNextInstruction() const;
    bool isInstructionCompleted() const;
    size_t getMemoryAllocated() const;
    void* getMemoryPtr() const;
    const std::chrono::system_clock::time_point& getArrivalTime() const;

    // Setters
    void setId(const std::string& pid);
    void setBurstTime(int burst);
    void setProgress(int progress);
    void setCoreId(int coreId);
    void setInstructions(const std::vector<Instruction>& instructions);
    void setCurrentInstruction(int currentInstruction);
    void setCyclesUntilNextInstruction(int cycles);
    void setInstructionCompleted(bool completed);
    void setMemoryAllocated(size_t memory);
    void setMemoryPtr(void* ptr);
    void setArrivalTime(const std::chrono::system_clock::time_point& arrival);

private:
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
