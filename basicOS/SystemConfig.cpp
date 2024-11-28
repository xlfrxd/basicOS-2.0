#include "SystemConfig.h"
#include <sstream>
#include <random>

SystemConfig::SystemConfig() :
    isInitialized(false) {}

bool SystemConfig::validate() const {
    if (numCPU < 1 || numCPU > 128) {
        std::cerr << "Error: num-cpu must be between 1 and 128" << std::endl;
        return false;
    }

    if (schedulerType != "fcfs" && schedulerType != "rr") {
        std::cerr << "Error: scheduler must be 'fcfs' or 'rr'" << std::endl;
        return false;
    }

    if (schedulerType == "rr" && quantumCycles < 1) {
        std::cerr << "Error: quantum-cycles must be at least 1 for RR scheduler" << std::endl;
        return false;
    }

    if (batchProcessFreq < 1) {
        std::cerr << "Error: batch-process-freq must be at least 1" << std::endl;
        return false;
    }

    if (minInstructions < 1 || maxInstructions < minInstructions) {
        std::cerr << "Error: Invalid instruction range. min-ins must be >= 1 and <= max-ins" << std::endl;
        return false;
    }

    if (maxOverallMem < memPerFrame || memPerFrame < 1) {
        std::cerr << "Error: Invalid memory configuration. max-overall-mem must be >= mem-per-frame, and mem-per-frame must be >= 1" << std::endl;
        return false;
    }

    return true;
}

bool readConfigFile(const std::string& filename, SystemConfig& config) {
    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::map<std::string, std::string> configValues;

    // Read config file line by line
    while (std::getline(configFile, line)) {
        std::stringstream ss(line);
        std::string key, value;

        if (ss >> key >> value) {
            configValues[key] = value;
        }
    }

    // Parse and validate all required parameters
    try {
        if (configValues.find("num-cpu") == configValues.end()) throw std::runtime_error("num-cpu not found");
        if (configValues.find("scheduler") == configValues.end()) throw std::runtime_error("scheduler not found");
        if (configValues.find("quantum-cycles") == configValues.end()) throw std::runtime_error("quantum-cycles not found");
        if (configValues.find("batch-process-freq") == configValues.end()) throw std::runtime_error("batch-process-freq not found");
        if (configValues.find("min-ins") == configValues.end()) throw std::runtime_error("min-ins not found");
        if (configValues.find("max-ins") == configValues.end()) throw std::runtime_error("max-ins not found");
        if (configValues.find("delays-per-exec") == configValues.end()) throw std::runtime_error("delays-per-exec not found");
        if (configValues.find("max-overall-mem") == configValues.end()) throw std::runtime_error("max-overall-mem not found");
        if (configValues.find("mem-per-frame") == configValues.end()) throw std::runtime_error("mem-per-frame not found");
        if (configValues.find("min-mem-per-proc") == configValues.end()) throw std::runtime_error("min-mem-per-proc not found");
        if (configValues.find("max-mem-per-proc") == configValues.end()) throw std::runtime_error("max-mem-per-proc not found");

        config.numCPU = std::stoi(configValues["num-cpu"]);
        config.schedulerType = configValues["scheduler"];
        config.quantumCycles = std::stoul(configValues["quantum-cycles"]);
        config.batchProcessFreq = std::stoul(configValues["batch-process-freq"]);
        config.minInstructions = std::stoul(configValues["min-ins"]);
        config.maxInstructions = std::stoul(configValues["max-ins"]);
        config.delaysPerExec = std::stoul(configValues["delays-per-exec"]);
        config.maxOverallMem = std::stoul(configValues["max-overall-mem"]);
        config.memPerFrame = std::stoul(configValues["mem-per-frame"]);
        std::random_device rd;  // Random device to seed the generator
        std::mt19937 gen(rd()); // Mersenne Twister engine
        std::uniform_int_distribution<> dist(std::stoi(configValues["min-mem-per-proc"]), std::stoi(configValues["max-mem-per-proc"]));
        config.memPerProc = dist(gen); // Input process mem
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        return false;
    }

    return config.validate();
}
