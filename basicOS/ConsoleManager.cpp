#pragma once

#include <iostream>
#include <unordered_map>
#include <random>

#include "Process.h"
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"

using namespace std;

// stores the created instance of console manager
ConsoleManager* ConsoleManager::consoleManager = nullptr;

ConsoleManager::ConsoleManager() {
};

void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
    ConsoleManager::getInstance()->initializeConfiguration();
}

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

            if (space_pos == string::npos) continue; // Skip malformed lines

            string key = str.substr(0, space_pos);
            string value = str.substr(space_pos + 1);
            value.erase(remove(value.begin(), value.end(), '\n'), value.end()); // Remove newline

            if (key == "num-cpu") {
                ConsoleManager::getInstance()->setNumCpu(stoi(value));
            }
            else if (key == "scheduler") {
                value.erase(remove(value.begin(), value.end(), '\"'), value.end()); // Remove quotes
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


void ConsoleManager::schedulerTest() {
    static int process_counter = 0;
    process_counter++;

    while (Scheduler::getInstance()->getSchedulerTestRunning()) {
        for (int i = 0; i < ConsoleManager::getInstance()->getBatchProcessFrequency(); i++) {
           /* string processName = "cycle" + std::to_string(ConsoleManager::getInstance()->cpuCycles) + "processName" + std::to_string(i);*/
            string processName = "P" + std::to_string(process_counter);
            shared_ptr<ProcessScreen> processScreen = make_shared<Process>(processName, 0, ConsoleManager::getInstance()->getCurrentTimestamp(), ConsoleManager::getInstance()->getMinMemPerProc());
            shared_ptr<Process> screenPtr = static_pointer_cast<Process>(processScreen);
            Scheduler::getInstance()->addProcessToQueue(screenPtr);
            ConsoleManager::getInstance()->registerConsole(processScreen);
            ConsoleManager::getInstance()->cpuCycles++;
            

        }
        process_counter++;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int ConsoleManager::getCpuCycles() {
	return this->cpuCycles;
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

void ConsoleManager::destroy() {
    consoleManager->scheduler.stop(); // Stop the scheduler
    delete consoleManager;
}

string ConsoleManager::getCurrentTimestamp() {
    // Get current time as time_t object
    time_t currentTime = time(nullptr);
    // Create tm structure to store local time
    tm localTime;
    // Convert time_t to tm structure 
    localtime_s(&localTime, &currentTime);
    // Create a buffer to store the formatted time
    char timeBuffer[100];
    // Format the time (MM/DD/YYYY, HH:MM:SS AM/PM)
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", &localTime);
    return timeBuffer;
}

void ConsoleManager::registerConsole(shared_ptr<ProcessScreen> screenRef) {
    this->screenMap[screenRef->getConsoleName()] = screenRef; //it should accept MainScreen and ProcessScreen
    //system("cls");
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
        shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(pair.second);

        if (screenPtr && screenPtr->getIsRunning() && screenPtr->getMemoryUsage() != 0) {

            auto coreID = screenPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            cout << screenPtr->getProcessName() << "\t(" << screenPtr->getTimestamp() << ")\tCore: " << coreIDstr << "\tProgress: "
        	<< screenPtr->getCurrentLine() << "/" << screenPtr->getTotalLine() << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(pair.second);


        if (screenPtr && screenPtr->isFinished()) {
            cout << screenPtr->getProcessName() << "\t(" << screenPtr->getTimestamp() << ")\tCore: " << "\tFinished"
                << screenPtr->getCurrentLine() << "/" << screenPtr->getTotalLine() << endl;
        }
    }
    cout << "-----------------------------------" << endl;
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

    // Log details of running processes (pre-memory)
    /*
	for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Process>(pair.second);
        if (screenPtr && !screenPtr->isFinished()) {
            auto coreID = screenPtr->getCPUCoreID();
            std::string coreIDstr = (coreID == -1) ? "N/A" : std::to_string(coreID);

            logStream << screenPtr->getProcessName() << "\t(" << screenPtr->getTimestamp() << ")\tCore: " << coreIDstr
        	<< "\tProgress: " << screenPtr->getCurrentLine() << "/" << screenPtr->getTotalLine() << endl;
        }
    } */

    for (const auto& pair : screenMap) {
        shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(pair.second);

        if (screenPtr && screenPtr->getIsRunning() && screenPtr->getMemoryUsage() != 0) {

            auto coreID = screenPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            logStream << screenPtr->getProcessName() << "\t(" << screenPtr->getTimestamp() << ")\tCore: " << coreIDstr << "\tProgress: "
                << screenPtr->getCurrentLine() << "/" << screenPtr->getTotalLine() << endl;
        }
    }

    logStream << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(pair.second);

        if (screenPtr && screenPtr->isFinished()) {
            logStream << screenPtr->getProcessName() << "\t(" << screenPtr->getTimestamp() << ")\tCore: " << "\tFinished"
                << screenPtr->getCurrentLine() << "/" << screenPtr->getTotalLine() << endl;
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

void ConsoleManager::initializeAllocators() {
    FlatMemoryAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
    PagingAllocator::initialize(ConsoleManager::getInstance()->getMaxOverallMem());
}

std::shared_ptr<Process> ConsoleManager::getScreenByProcessName(const std::string& processName) {
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(processName);
    if (it != screenMap.end()) {
        std::shared_ptr<Process> process = std::dynamic_pointer_cast<Process>(it->second);
        if(process) {
            return process;  // Successfully casted to Screen
        }
    }
    return nullptr;  // Process not found
}

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


void ConsoleManager::printProcess(string enteredProcess){
    unordered_map<string, shared_ptr<ProcessScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(enteredProcess);
    for (const auto& pair : screenMap) {
        shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(pair.second);

        //check if process name exits
        if (screenPtr->getProcessName() == enteredProcess) {

            //check if process is finished
            if (screenPtr && screenPtr->isFinished()){
                shared_ptr<Process> screenPtr = dynamic_pointer_cast<Process>(screenMap.find(enteredProcess)->second);

                auto coreID = screenPtr->getCPUCoreID();
                string coreIDstr;
                if (coreID == -1) {
                    coreIDstr = "N/A";
                }
                else {
                    coreIDstr = to_string(coreID);
                }

                cout << "Process Name: " << enteredProcess << endl;
                cout << "Logs:" << endl;
                cout << "(" << screenPtr->getTimestamp() << ")  "
                    << "Core: " << coreIDstr << "  ";

                screenPtr->createFile();
                screenPtr->viewFile();
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
        auto screenPtr = std::dynamic_pointer_cast<Process>(pair.second);
        if (screenPtr && !screenPtr->isFinished() && screenPtr->getIsRunning() && screenPtr->getMemoryUsage() != 0) {  
            size_t memoryUsage = screenPtr->getMemoryUsage();
            cout << "Process: " << screenPtr->getProcessName() << " | Memory: " << memoryUsage << " KB" << endl;
        }
    }
	cout << "===================================================" << endl << endl;
}

void ConsoleManager::getMemoryUsage() {
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << "Memory Usage: " << FlatMemoryAllocator::getInstance()->visualizeMemory() << endl;
    }
    else {
        PagingAllocator::getInstance()->visualizeMemory();
    }
}

void ConsoleManager::printVmstat() {
    cout << "Total Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() << " KB" << endl;
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        cout << "Used Memory: " << FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << " KB" << endl;
        cout << "Free Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() - FlatMemoryAllocator::getInstance()->getTotalMemoryUsage() << " KB" << endl;
    }
    else {
        cout << "Used Memory: " << PagingAllocator::getInstance()->getUsedMemory() << " KB" << endl;
        cout << "Free Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() - PagingAllocator::getInstance()->getUsedMemory() << " KB" << endl;
    }
    
    
    cout << "Idle CPU Ticks: " << Scheduler::getInstance()->getIdleCpuTicks() << endl;
    cout << "Active CPU Ticks: " << Scheduler::getInstance()->getCpuCycles() << endl;
    cout << "Total CPU Ticks: " << Scheduler::getInstance()->getCpuCycles() + Scheduler::getInstance()->getIdleCpuTicks() << endl;
    cout << "Num paged in: " << PagingAllocator::getInstance()->getNumPagedIn() << endl;
    cout << "Num paged out: " << PagingAllocator::getInstance()->getNumPagedOut() << endl << endl;
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
    cout << ConsoleColor::YELLOW << "Type 'exit' to quit, 'clear' to clear the screen. \n" << ConsoleColor::RESET;
}

shared_ptr<ProcessScreen> ConsoleManager::getCurrentConsole()
{
    return this->currentConsole;
}

void ConsoleManager::setCurrentConsole(shared_ptr<ProcessScreen> screenRef)
{
    this->currentConsole = screenRef;
}

ConsoleManager* ConsoleManager::getInstance()
{
    return consoleManager;
}

void ConsoleManager::exitApplication() {
    this->running = false;
	Scheduler::getInstance()->stop();
}

bool ConsoleManager::isRunning() {
    return this->running;
}

unordered_map<string, shared_ptr<ProcessScreen>> ConsoleManager::getScreenMap() {
    return this->screenMap;
}

void ConsoleManager::setInitialized(bool initialized) {
	this->initialized = initialized;
}

bool ConsoleManager::getInitialized() {
	return this->initialized;
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

void ConsoleManager::setNumPages() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinMemPerProc(), ConsoleManager::getInstance()->getMaxMemPerProc());

    this->numPages = dis(gen) / ConsoleManager::getInstance()->getMemPerFrame();
}

int ConsoleManager::getNumPages() {
	return this->numPages;
}
