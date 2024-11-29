#pragma once

#include <iostream>
#include <unordered_map>
#include <random>

#include "Process.h"
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"

using namespace std;

ConsoleManager* ConsoleManager::consoleManager = nullptr;

ConsoleManager::ConsoleManager() {
};

ConsoleManager* ConsoleManager::getInstance()
{
    return consoleManager;
}

void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
    ConsoleManager::getInstance()->initializeConfiguration();
}

void ConsoleManager::drawConsole() {
    if (this->switchSuccessful) {
        system("cls");
        string consoleName = this->getCurrentConsole()->getConsoleName();

        if (this->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
            this->printHeader();
        }
        else {
            if (this->screenMap.contains(consoleName)) {
                cout << "Screen Name: " << this->screenMap[consoleName]->getConsoleName() << endl;
                cout << "Current line of instruction / Total line of instruction: ";
                cout << this->screenMap[consoleName]->getCurrentLine();
                cout << "/" << this->screenMap[consoleName]->getTotalLine() << endl;
                cout << "Timestamp: " << this->screenMap[consoleName]->getTimestamp() << endl;
            }
        }
    }
}

void ConsoleManager::printHeader() {
    cout << ConsoleColor::BLUE << "                              #######  #####  \n";
    cout << ConsoleColor::BLUE << "#####    ##    ####  #  ####  #     # #     # \n";
    cout << ConsoleColor::BLUE << "#    #  #  #  #      # #    # #     # #       \n";
    cout << ConsoleColor::BLUE << "#####  #    #  ####  # #      #     #  #####  \n";
    cout << ConsoleColor::BLUE << "#    # ######      # # #      #     #       # \n";
    cout << ConsoleColor::BLUE << "#    # #    # #    # # #    # #     # #     # \n";
    cout << ConsoleColor::BLUE << "#####  #    #  ####  #  ####  #######  #####  \n\n";
    cout << "Hello, welcome to the basicOS command line! \n";
    cout << ConsoleColor::YELLOW << "Type 'initialize' to start, 'exit' to quit, 'clear' to clear the screen. \n" << ConsoleColor::RESET;
}

void ConsoleManager::destroy() {
    consoleManager->scheduler.stop();
    delete consoleManager;
}

void ConsoleManager::exitApplication() {
    this->running = false;
    Scheduler::getInstance()->stop();
}

// Configuration getters and setters
void ConsoleManager::initializeConfiguration() {
    FILE* file;
    errno_t err = fopen_s(&file, "config.txt", "r");
    if (err != 0) {
        cout << ConsoleColor::RED << "Cannot open file" << ConsoleColor::RESET << endl;
    }

    else {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            string str = line;
            size_t space_pos = str.find(" ");

            if (space_pos == string::npos) continue;

            string key = str.substr(0, space_pos);
            string value = str.substr(space_pos + 1);
            value.erase(remove(value.begin(), value.end(), '\n'), value.end());

            if (key == "num-cpu") {
                ConsoleManager::getInstance()->setNumCpu(stoi(value));
            }
            else if (key == "scheduler") {
                value.erase(remove(value.begin(), value.end(), '\"'), value.end()); 
                ConsoleManager::getInstance()->setSchedulerConfig(value);
            }
            else if (key == "quantum-cycles") {
                ConsoleManager::getInstance()->setTimeSlice(stoi(value));
            }
            else if (key == "min-ins") {
                ConsoleManager::getInstance()->setMinIns(stoi(value));
            }
            else if (key == "max-ins") {
                ConsoleManager::getInstance()->setMaxIns(stoi(value));
            }
            else if (key == "delay-per-exec") {
                ConsoleManager::getInstance()->setDelayPerExec(stoi(value));
            }
            else if (key == "batch-process-freq") {
                ConsoleManager::getInstance()->setBatchProcessFrequency(stoi(value));
            } 
            else if (key == "max-overall-mem") {
				ConsoleManager::getInstance()->setMaxOverallMem(stoi(value));
            }
            else if (key == "mem-per-frame") {
                ConsoleManager::getInstance()->setMemPerFrame(stoi(value));
            }
            else if (key == "min-mem-per-proc") {
				ConsoleManager::getInstance()->setMinMemPerProc(stoi(value));
			}
			else if (key == "max-mem-per-proc") {
				ConsoleManager::getInstance()->setMaxMemPerProc(stoi(value));
            }
           
        }
        fclose(file);
    }


	Scheduler* scheduler = Scheduler::getInstance();

    setNumPages();
}

void ConsoleManager::initializeAllocators() {
    FlatMemoryAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
    PagingAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
}

// getters
int ConsoleManager::getNumCpu() {
    return this->num_cpu;
}

string ConsoleManager::getSchedulerConfig() {
    return this->schedulerConfig;
}

int ConsoleManager::getTimeSlice() {
    return this->timeSlice;
}

int ConsoleManager::getBatchProcessFrequency() {
    return this->batchProcessFrequency;
}

int ConsoleManager::getMinIns() {
    return this->minIns;
}

int ConsoleManager::getMaxIns() {
    return this->maxIns;
}

int ConsoleManager::getDelayPerExec() {
    return this->delayPerExec;
}

size_t ConsoleManager::getMaxOverallMem() {
    return this->maxOverallMem;
}

size_t ConsoleManager::getMemPerFrame() {
    return this->memPerFrame;
}

size_t ConsoleManager::getMinMemPerProc() {
    return this->minMemPerProc;
}

size_t ConsoleManager::getMaxMemPerProc() {
    return this->maxMemPerProc;
}

//setters
void ConsoleManager::setNumCpu(int num_cpu) {
    this->num_cpu = num_cpu;
}

void ConsoleManager::setSchedulerConfig(string scheduler) {
    this->schedulerConfig = scheduler;
}

void ConsoleManager::setTimeSlice(int timeSlice) {
    this->timeSlice = timeSlice;
}

void ConsoleManager::setBatchProcessFrequency(int batchProcessFrequency) {
    this->batchProcessFrequency = batchProcessFrequency;
}

void ConsoleManager::setMinIns(int minIns) {
    this->minIns = minIns;
}

void ConsoleManager::setMaxIns(int maxIns) {
    this->maxIns = maxIns;
}

void ConsoleManager::setDelayPerExec(int delayPerExec) {
    this->delayPerExec = delayPerExec;
}

void ConsoleManager::setMaxOverallMem(size_t maxOverallMem) {
    this->maxOverallMem = maxOverallMem;
}

void ConsoleManager::setMemPerFrame(size_t memPerFrame) {
    this->memPerFrame = memPerFrame;
}

void ConsoleManager::setMinMemPerProc(size_t minMemPerProc) {
    this->minMemPerProc = minMemPerProc;
}

void ConsoleManager::setMaxMemPerProc(size_t maxMemPerProc) {
    this->maxMemPerProc = maxMemPerProc;
}

//Process Screen
std::shared_ptr<Process> ConsoleManager::getScreenByProcessName(const std::string& processName) {
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(processName);
    if (it != screenMap.end()) {
        std::shared_ptr<Process> process = std::dynamic_pointer_cast<Process>(it->second);
        if (process) {
            return process;
        }
    }
    return nullptr;
}

unordered_map<string, shared_ptr<ProcessScreen>> ConsoleManager::getScreenMap() {
    return this->screenMap;
}

shared_ptr<ProcessScreen> ConsoleManager::getCurrentConsole()
{
    return this->currentConsole;
}

void ConsoleManager::registerConsole(shared_ptr<ProcessScreen> screenRef) {
    this->screenMap[screenRef->getConsoleName()] = screenRef;
}

void ConsoleManager::setCurrentConsole(shared_ptr<ProcessScreen> screenRef)
{
    this->currentConsole = screenRef;
}

void ConsoleManager::switchConsole(string consoleName)
{
    if (this->screenMap.contains(consoleName)) {
        this->currentConsole = this->screenMap[consoleName];
        this->consoleName = consoleName;

        if (consoleName == MAIN_CONSOLE) {
            this->drawConsole();
        }

        this->switchSuccessful = true;
    }
    else {
        cout << ConsoleColor::YELLOW << "Console name " << consoleName << " not found." << ConsoleColor::RESET << endl;
        this->switchSuccessful = false;
    }
}

bool ConsoleManager::getInitialized() {
    return this->initialized;
}

void ConsoleManager::setInitialized(bool initialized) {
    this->initialized = initialized;
}

// Process information
string ConsoleManager::getCurrentTimestamp() {
    time_t currentTime = time(nullptr);
    tm localTime;

	localtime_s(&localTime, &currentTime); // localtime_s for thread safety
    
	char timeBuffer[100]; // Buffer to store formatted time
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", &localTime);

    return timeBuffer;
}

int ConsoleManager::getCpuCycles() {
    return this->cpuCycles;
}

void ConsoleManager::getMemoryUsage() {
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << "Memory Usage: " << FlatMemoryAllocator::getInstance()->visualizeMemory() << endl;
    }
    else {
        PagingAllocator::getInstance()->visualizeMemory();
    }
}

int ConsoleManager::getNumPages() {
    return this->numPages;
}

void ConsoleManager::setNumPages() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinMemPerProc(), ConsoleManager::getInstance()->getMaxMemPerProc());

    this->numPages = dis(gen) / ConsoleManager::getInstance()->getMemPerFrame();
}

// Scheduler
void ConsoleManager::schedulerTest() {
    static int process_counter = 0;
    process_counter++;

    while (Scheduler::getInstance()->getSchedulerTestRunning()) {
        for (int i = 0; i < ConsoleManager::getInstance()->getBatchProcessFrequency(); i++) {
           /* string processName = "cycle" + std::to_string(ConsoleManager::getInstance()->cpuCycles) + "processName" + std::to_string(i);*/
            string processName = "P" + std::to_string(process_counter);
            shared_ptr<ProcessScreen> processScreen = make_shared<Process>(processName, 0, ConsoleManager::getInstance()->getCurrentTimestamp(), ConsoleManager::getInstance()->getMinMemPerProc());
            shared_ptr<Process> processPtr = static_pointer_cast<Process>(processScreen);
            Scheduler::getInstance()->addProcessToQueue(processPtr);
            ConsoleManager::getInstance()->registerConsole(processScreen);
            ConsoleManager::getInstance()->cpuCycles++;
            

        }
        process_counter++;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

bool ConsoleManager::isRunning() {
    return this->running;
}

//Printing
void ConsoleManager::displayProcessList() {
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = (float)coresUsed / (coresUsed + coresAvailable) * 100;

    cout << "\nCPU Utilization: " << cpuUtilization << "%" << endl;
    cout << "Cores used: " << coresUsed << endl;
    cout << "Cores available: " << coresAvailable << endl;
    cout << "-----------------------------------" << endl;
    cout << "Running processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(pair.second);

        if (processPtr && processPtr->getIsRunning() && processPtr->getMemoryUsage() != 0) {

            auto coreID = processPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            cout << processPtr->getProcessName() << "\t(" << processPtr->getTimestamp() << ")\tCore: " << coreIDstr << "\tProgress: "
        	<< processPtr->getCurrentLine() << "/" << processPtr->getTotalLine() << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(pair.second);


        if (processPtr && processPtr->isFinished()) {
            cout << processPtr->getProcessName() << "\t(" << processPtr->getTimestamp() << ")\tCore: " << "\tFinished"
                << processPtr->getCurrentLine() << "/" << processPtr->getTotalLine() << endl;
        }
    }
    cout << "-----------------------------------" << endl;
}

void ConsoleManager::printProcess(string enteredProcess){
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(enteredProcess);
    for (const auto& pair : screenMap) {
        shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(pair.second);

        //check if process name exits
        if (processPtr->getProcessName() == enteredProcess) {

            //check if process is finished
            if (processPtr && processPtr->isFinished()){
                shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(screenMap.find(enteredProcess)->second);

                auto coreID = processPtr->getCPUCoreID();
                string coreIDstr;
                if (coreID == -1) {
                    coreIDstr = "N/A";
                }
                else {
                    coreIDstr = to_string(coreID);
                }

                cout << "Process Name: " << enteredProcess << endl;
                cout << "Logs:" << endl;
                cout << "(" << processPtr->getTimestamp() << ")  "
                    << "Core: " << coreIDstr << "  ";

                processPtr->createFile();
                processPtr->viewFile();
            }
            else {
                cout << ConsoleColor::YELLOW << "Process is not yet finished" << ConsoleColor::RESET << endl;
            }
            
        }

    }
}

void ConsoleManager::printProcessSmi() {
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    cout << "--------------------------------------------------" << endl;
    cout << "|    PROCESS-SMI V01.00 Driver Version 01.00      |" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "CPU Utilization: " << cpuUtilization << "%" << endl;
    getMemoryUsage();

    cout << "===================================================" << endl;
    cout << "Running processes and memory usage:" << endl;
    cout << "---------------------------------------------------" << endl;

    // Iterate through screenMap to get running processes and their memory usage
    for (const auto& pair : screenMap) {
        auto processPtr = std::dynamic_pointer_cast<Process>(pair.second);
        if (processPtr && !processPtr->isFinished() && processPtr->getIsRunning() && processPtr->getMemoryUsage() != 0) {  
            size_t memoryUsage = processPtr->getMemoryUsage();
            cout << "Process: " << processPtr->getProcessName() << " | Memory: " << memoryUsage << " KB" << endl;
        }
    }
	cout << "===================================================" << endl << endl;
}

void ConsoleManager::printVmstat() {
    cout << ConsoleColor::BLUE << ConsoleManager::getInstance()->getMaxOverallMem() << ConsoleColor::RESET << " KB" << " total memory" << endl;
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << ConsoleColor::BLUE << FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << ConsoleColor::RESET << " KB" << " used memory" << endl;
        cout << ConsoleColor::BLUE << ConsoleManager::getInstance()->getMaxOverallMem() - FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << ConsoleColor::RESET << " KB" << " free memory" << endl;
    }
    else {
        cout << ConsoleColor::BLUE << PagingAllocator::getInstance()->getUsedMemory() << ConsoleColor::RESET << " KB" << " used memory" << endl;
        cout << ConsoleColor::BLUE << ConsoleManager::getInstance()->getMaxOverallMem() - PagingAllocator::getInstance()->getUsedMemory() << ConsoleColor::RESET << " KB" << " free memory" << endl;
    }
    cout << ConsoleColor::BLUE << Scheduler::getInstance()->getIdleCpuTicks() << ConsoleColor::RESET << " idle cpu ticks" << endl;
    cout << ConsoleColor::BLUE << Scheduler::getInstance()->getCpuCycles() << ConsoleColor::RESET << " active cpu ticks" << endl;
    cout << ConsoleColor::BLUE << Scheduler::getInstance()->getCpuCycles() + Scheduler::getInstance()->getIdleCpuTicks() << ConsoleColor::RESET << " total cpu ticks" << endl;
    cout << ConsoleColor::BLUE << PagingAllocator::getInstance()->getNumPagedIn() << ConsoleColor::RESET << " num paged in" << endl;
    cout << ConsoleColor::BLUE << PagingAllocator::getInstance()->getNumPagedOut() << ConsoleColor::RESET << " num paged out"<< endl << endl;
}

void ConsoleManager::reportUtil() {
    std::ostringstream logStream;
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    // Log CPU utilization and core details
    logStream << "\nCPU Utilization: " << cpuUtilization << "%" << std::endl;
    logStream << "Cores used: " << coresUsed << std::endl;
    logStream << "Cores available: " << coresAvailable << std::endl;
    logStream << "-----------------------------------" << std::endl;
    logStream << "Running processes:" << std::endl;

    for (const auto& pair : screenMap) {
        shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(pair.second);

        if (processPtr && processPtr->getIsRunning() && processPtr->getMemoryUsage() != 0) {

            auto coreID = processPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            logStream << processPtr->getProcessName() << "\t(" << processPtr->getTimestamp() << ")\tCore: " << coreIDstr << "\tProgress: "
                << processPtr->getCurrentLine() << "/" << processPtr->getTotalLine() << endl;
        }
    }

    logStream << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Process> processPtr = dynamic_pointer_cast<Process>(pair.second);

        if (processPtr && processPtr->isFinished()) {
            logStream << processPtr->getProcessName() << "\t(" << processPtr->getTimestamp() << ")\tCore: " << "\tFinished"
                << processPtr->getCurrentLine() << "/" << processPtr->getTotalLine() << endl;
        }
    }

    logStream << "-----------------------------------" << std::endl;

    // Write the log data to a file
    std::ofstream file("csopesy-log.txt", std::ios::out);
    if (file.is_open()) {
        file << logStream.str(); // Write log contents to file
        file.close();
        std::cout << ConsoleColor::GREEN << "Report generated with the filename csopesy-log.txt" << ConsoleColor::RESET << std::endl;
    }
    else {
        std::cerr << ConsoleColor::RED << "Error: Could not open file for writing." << ConsoleColor::RESET << std::endl;
    }
}
