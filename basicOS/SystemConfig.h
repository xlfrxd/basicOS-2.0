#pragma once
#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include <string>
#include <iostream>
#include <fstream>
#include <map>

struct SystemConfig {
    int numCPU;                  // Number of CPUs [1-128]
    std::string schedulerType;   // "fcfs" or "rr"
    uint32_t quantumCycles;      // Time slice for RR
    uint32_t batchProcessFreq;   // Process generation frequency
    uint32_t minInstructions;    // Minimum instructions per process
    uint32_t maxInstructions;    // Maximum instructions per process
    uint32_t delaysPerExec;      // Delay cycles between instructions
    bool isInitialized;          // Track if system is initialized
    size_t maxOverallMem;        // Maximum memory available in KB
    size_t memPerFrame;          // The size of memory in KB per frame
    size_t memPerProc;           // Fixed amount of memory for each process

    SystemConfig();

    bool validate() const;  // Function to validate configuration parameters
};

// Function prototype to read configuration file
bool readConfigFile(const std::string& filename, SystemConfig& config);

#endif
