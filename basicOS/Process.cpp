#include "Process.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

Process::Process(const std::string& pid, int burst)
    : id(pid),
    burst_time(burst),
    progress(0),
    core_id(-1),
    arrival_time(std::chrono::system_clock::now()),
    rng(std::random_device{}()),
    current_instruction(0),
    cycles_until_next_instruction(0),
    instruction_completed(false),
    memoryAllocated(0),
    memoryPtr(nullptr),
    pagesReq(0) {
    generateInstructions();
}

void Process::generateInstructions() {
    std::uniform_int_distribution<> sleep_dist(1, 5); // Random sleep cycles between 1-5

    for (int i = 0; i < burst_time; ++i) {
        // Alternate between PRINT and SLEEP instructions
        if (i % 2 == 0) {
            instructions.push_back({ InstructionType::PRINT, 1 });
        }
        else {
            instructions.push_back({ InstructionType::SLEEP, sleep_dist(rng) });
        }
    }
}

bool Process::executeInstruction() {
    if (current_instruction >= instructions.size()) {
        return false; // No more instructions to execute
    }

    instruction_completed = false;

    if (cycles_until_next_instruction > 0) {
        cycles_until_next_instruction--;
        return false; // Currently sleeping, not completed yet
    }

    auto& instruction = instructions[current_instruction];
    switch (instruction.type) {
    case InstructionType::PRINT:
        // PRINT instructions complete immediately
        instruction_completed = true;
        break;
    case InstructionType::SLEEP:
        // Set cycles needed for SLEEP instruction
        instruction_completed = true;
        cycles_until_next_instruction = instruction.cycles;
        break;
    }

    if (instruction_completed) {
        current_instruction++;
        progress++;
        return true; // Successfully completed an instruction
    }

    return false; // Instruction not yet completed
}

std::chrono::milliseconds Process::getInstructionTime() {
    // Random time between 500ms and 2000ms
    std::uniform_int_distribution<> dist(500, 2000);
    return std::chrono::milliseconds(dist(rng));
}

std::string Process::getTimeStamp() const {
    std::time_t now = std::chrono::system_clock::to_time_t(arrival_time);
    std::tm time_info;
#ifdef _WIN32
    localtime_s(&time_info, &now);
#else
    localtime_r(&now, &time_info);
#endif
    std::ostringstream oss;
    oss << std::put_time(&time_info, "%m/%d/%Y %I:%M:%S%p");
    return oss.str();
}

// Getters
const std::string& Process::getId() const { return id; }
int Process::getBurstTime() const { return burst_time; }
int Process::getProgress() const { return progress; }
int Process::getCoreId() const { return core_id; }
const std::vector<Instruction>& Process::getInstructions() const { return instructions; }
int Process::getCurrentInstruction() const { return current_instruction; }
int Process::getCyclesUntilNextInstruction() const { return cycles_until_next_instruction; }
bool Process::isInstructionCompleted() const { return instruction_completed; }
size_t Process::getMemoryAllocated() const { return memoryAllocated; }
void* Process::getMemoryPtr() const { return memoryPtr; }
const std::chrono::system_clock::time_point& Process::getArrivalTime() const { return arrival_time; }
int Process::getNumPages() const { return pagesReq; }
// Setters
void Process::setId(const std::string& pid) { id = pid; }
void Process::setBurstTime(int burst) { burst_time = burst; }
void Process::setProgress(int progress) { this->progress = progress; }
void Process::setCoreId(int coreId) { core_id = coreId; }
void Process::setInstructions(const std::vector<Instruction>& instructions) { this->instructions = instructions; }
void Process::setCurrentInstruction(int currentInstruction) { current_instruction = currentInstruction; }
void Process::setCyclesUntilNextInstruction(int cycles) { cycles_until_next_instruction = cycles; }
void Process::setInstructionCompleted(bool completed) { instruction_completed = completed; }
void Process::setMemoryAllocated(size_t memory) { memoryAllocated = memory; }
void Process::setMemoryPtr(void* ptr) { memoryPtr = ptr; }
void Process::setArrivalTime(const std::chrono::system_clock::time_point& arrival) { arrival_time = arrival; }
void Process::setNumPages(int pages) { this->pagesReq = pages; }
