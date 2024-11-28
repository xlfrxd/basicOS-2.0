//
//  main.cpp
//  basicOS
//  
//  CSOPESY S16
//  Group 2
//  
//  Izabella Imperial
//  Marc Daniel Marasigan
//  Nikolai Santiago
//  Alfred Victoria
//

#define _CRT_SECURE_NO_WARNINGS

#include "ConsoleManager.h"
#include "SystemConfig.h"
#include "Instruction.h"
#include "Process.h"
#include <random>
#include <unordered_map>

using namespace std;

// Global configuration object
SystemConfig sysConfig;

atomic<uint64_t> cpu_cycles{ 0 }; // Global counter for CPU cycles
atomic<bool> process_generator_running{ false }; // Flag to control process generator thread
thread process_generator_thread; // Thread to generate test processes

// colors
const int RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
const int GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const int BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const int RESET = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

// MAIN_MENU Commands
vector<string> MAIN_MENU_CMD; // Main Menu command list

// Initialize main menu commands
void initializeMainMenuCmds() {
    MAIN_MENU_CMD.push_back("initialize");
    MAIN_MENU_CMD.push_back("screen");
    MAIN_MENU_CMD.push_back("scheduler-test");
    MAIN_MENU_CMD.push_back("scheduler-stop");
    MAIN_MENU_CMD.push_back("report-util");
    MAIN_MENU_CMD.push_back("clear");
    MAIN_MENU_CMD.push_back("exit");
    MAIN_MENU_CMD.push_back("print");
}
// Helper function for setting text color
void SetConsoleColor(int textColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, textColor);
}

// Global map to store all screens created by the user
map<string, ScreenInfo> screens;
// Screens mutex
std::mutex screensMutex;

string currentScreen = "Main Menu";  // Track the current screen (default to "Main Menu")
// Helper function for getting system current timestamp
string getCurrentTimestamp() {
    time_t now;
    struct tm* datetime;

    time(&now);
    datetime = localtime(&now);

    char output[50];
    // MM/DD/YYYY, HH:MM:SS AM/PM
    strftime(output, sizeof(output), "%m/%d/%Y, %I:%M:%S %p", datetime);
    return output;
}

// Main menu header
void printHeader() {
    // ascii header
    SetConsoleColor(RESET);
    cout << "                              #######  #####  \n";
    cout << "#####    ##    ####  #  ####  #     # #     # \n";
    cout << "#    #  #  #  #      # #    # #     # #       \n";
    cout << "#####  #    #  ####  # #      #     #  #####  \n";
    cout << "#    # ######      # # #      #     #       # \n";
    cout << "#    # #    # #    # # #    # #     # #     # \n";
    cout << "#####  #    #  ####  #  ####  #######  #####  \n\n";
    SetConsoleColor(GREEN);
    cout << "Hello, welcome to the basicOS command line! \n";
    SetConsoleColor(YELLOW);
    cout << "Type 'exit' to quit, 'clear' to clear the screen. \n";
    SetConsoleColor(RESET);
}

// Display cmd frontline
void printInstruc() {
    SetConsoleColor(RESET);
    cout << "------------------------------------------------" << endl;
    cout << "Enter a command: ";
}

// Checks if a command exists in given command list
bool validateCmd(string& cmd, vector<string>& arr) {
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == cmd) {
            return true; // Command exists
        }
    }
    return false; // Command not recognized
}

// Clear a screen (currently used for exit cmd) TODO: update for new implementations/screens
void clearScreen() {
    system("cls");
}

// Display console for screen
void displayScreen(const ScreenInfo& screen) {
    clearScreen();
    cout << "\n----------------- Screen: ";
    SetConsoleColor(GREEN);
    cout << screen.processName;
    SetConsoleColor(RESET);
    cout << " -----------------\n";
    cout << "Process Name: ";
    SetConsoleColor(GREEN);
    cout << screen.processName;
    SetConsoleColor(RESET);
    cout << "\n";
    cout << "Current Instruction: ";
    SetConsoleColor(BLUE);
    cout << screen.currentLine;
    SetConsoleColor(RESET);
    cout << " / ";
    SetConsoleColor(BLUE);
    cout << screen.totalLines;
    SetConsoleColor(RESET);
    cout << "\n";
    cout << "Screen Created: ";
    SetConsoleColor(BLUE);
    cout << screen.creationTimestamp << "\n";
    SetConsoleColor(YELLOW);
    cout << "Type 'exit' to return to the Main Menu.\n";
    SetConsoleColor(RESET);
}

// Write to text file
void writeToFile(const std::string& fileName, const std::string& content, const std::string& processName) {
    std::ofstream logFile;
    // Open file in append mode
    logFile.open(fileName.c_str(), std::ios::app);

    if (logFile.is_open()) {
        // Check if the file is empty
        logFile.seekp(0, std::ios::end);
        if (logFile.tellp() == 0) {
            // Add header if the file is empty
            logFile << "Process name: " << processName << "\nLogs:\n\n";
        }

        // Write content
        logFile << content;
        // Close the file
        logFile.close();
    }
}

// Print logs
void logPrintCommand(const std::string& fileName, int coreId, const std::string& processName) {
    // Convert int to str
    std::stringstream str_coreId;
    str_coreId << coreId;

    // Log value
    std::string log = "(" + getCurrentTimestamp() + ") Core:" + str_coreId.str() + ": \"Hello world from " + fileName + "!\"\n";

    // Write to log file
    writeToFile(fileName, log, processName);
}

// Function to run each process in the background
void runProcessInBackground(ScreenInfo& info) {

    // Check if the process exists before starting
    if (screens.find(info.processName) == screens.end()) {
        std::cout << "Process " << info.processName << " not found." << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(screensMutex);


    while (info.currentLine < info.totalLines) {
        // Move to the next instruction
        info.currentLine++;

        // Simulate execution time
        this_thread::sleep_for(chrono::milliseconds(123));
    }
    // Update process finish flag
    info.isFinished = true;
}

// Function to start a process with background execution
void startProcess(const std::string& processName) {
    if (screens.find(processName) != screens.end()) {
        ScreenInfo& info = screens[processName];

        // Create a new thread to execute the process in the background
        thread backgroundThread(runProcessInBackground, ref(info));

        // Detach the thread so it runs independently
        backgroundThread.detach();
    }
    else {
        cout << "Process " << processName << " not found." << endl;
    }
}

// Add these declarations near the top of basicOS.cpp
void createScreenSilent(const string& screenName); // New function for creating screen without display

// Modify createScreen to have an optional display parameter
void createScreen(const string& screenName, bool shouldDisplay = true) {
    if (screens.find(screenName) == screens.end()) {
        ScreenInfo newScreen;
        newScreen.processName = screenName;
        newScreen.currentLine = 1;
        newScreen.totalLines = 10 + (rand() % 100);
        //newScreen.totalLines = sysConfig.minInstructions + (rand() % sysConfig.maxInstructions);
        newScreen.creationTimestamp = getCurrentTimestamp();
        newScreen.isFinished = false;
        //newScreen.logFileName = screenName + "_log.txt";
        newScreen.commandArr.push_back("exit");
        newScreen.commandArr.push_back("process-smi");
        newScreen.commandArr.push_back("print");
        screens[screenName] = newScreen;

        startProcess(screenName); // start background process

        // Only switch to the screen and display it if shouldDisplay is true
        if (shouldDisplay) {
            currentScreen = screenName;
            displayScreen(screens[screenName]);
        }
    }
    else {
        cout << "Screen '" << screenName << "' already exists." << endl;
    }
}

// Silent version of createScreen that never displays
void createScreenSilent(const string& screenName) {
    createScreen(screenName, false);
}

// Display unknown command
void displayError(const string& cmd) {
    SetConsoleColor(RED);
    cout << "ERROR: \"" << cmd << "\" command not recognized.\n";
    SetConsoleColor(RESET);
}

// Display recognized command
void displayRecognized(const string& cmd) {
    SetConsoleColor(BLUE);
    cout << "\"" << cmd << "\"";
    // Recognized command
    SetConsoleColor(GREEN);
    cout << " command recognized. Doing something.\n";
    SetConsoleColor(RESET);
}

// Display created screen
void resumeScreen(const string& screenName) {
    if (screens.find(screenName) != screens.end()) {
        currentScreen = screenName;
        displayScreen(screens[screenName]);  // Display the screen layout
    }
    else {
        SetConsoleColor(RED);
        cout << "Screen '" << screenName << "' not found.\n"; // Screen not existing
        SetConsoleColor(RESET);
    }
}

void printLogToText(const std::string& screenName) {
    string fullFilename = screenName + "_log.txt";  // Append ".txt" to the screen name
    ifstream inFile(fullFilename);  // Input file stream to read the file

    cout << "Program Name: ";
    SetConsoleColor(BLUE);
    cout << screenName << "\n";
    SetConsoleColor(RESET);
    // Check if the file is open
    if (inFile.is_open()) {
        string line = "";
        bool isEmpty = true;
        cout << "Logs: \n";
        // Read the file line by line and print to the console
        SetConsoleColor(GREEN);
        while (getline(inFile, line)) {
            isEmpty = false;
            cout << line << "\n";
        }
        SetConsoleColor(RESET);
        if (isEmpty) {
            SetConsoleColor(RED);
            cout << "There are no logs.\n";
            SetConsoleColor(RESET);
        }
        cout << "=========================================\n";
        inFile.close();
    }
    else {
        SetConsoleColor(RED);
        cerr << "ERROR: Could not open file " << fullFilename << "\n";
        SetConsoleColor(RESET);

    }
}

// IMemoryAllocator interface
class IMemoryAllocator {
public:
    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* ptr) = 0;
    virtual std::string visualizeMemory() = 0;

    // New method to calculate external fragmentation
    virtual size_t calculateExternalFragmentation() = 0;

    virtual ~IMemoryAllocator() = default;  // Ensure a virtual destructor for proper cleanup
};

class PagingAllocator : public IMemoryAllocator {
public:
    PagingAllocator(size_t maxMemorySize);
    void* allocate(Process* process) override;
    void deallocate(Process* process) override;
    void visualizeMemory() const override;

private:
    size_t maxMemorySize;
    size_t numFrames;
    std::unordered_map<size_t, size_t> frameMap;
    std::vector<size_t> freeFrameList;

    size_t allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSizes);
    void deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSizes);
};

PagingAllocator::PagingAllocator(size_t maxMemorySize)
    : maxMemorySize(maxMemorySize), numFrames(maxMemorySize) {
    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList.push_back(i);
    }
}

void* PagingAllocator::allocate(Process* process) {
    size_t processID = process->getId();
    size_t numFramesNeeded = process->getNumPages();
    if (numFramesNeeded > freeFrameList.size()) {
        std::cerr << "Memory allocation failed. Not enough free frame. \n";
        return nullptr;
    }
}

void PagingAllocator::deallocate(Process* process) {
    size_t processId = process->getId();

    auto it = std::find_if(frameMap.begin(), frameMap.end(),
        [processId](const auto& entry) { return entry.second == processId; });
}

void PagingAllocator::visualizeMemory() const {
    std:cout << " Memory Visualization:\n";
    for (size_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
        auto it = frameMap.find(frameIndex);
        if (it != frameMap.end()) {
            std::cout << "Frame " << frameIndex << " -> Process " << it->second << "\n";
        }
        else {
            std::cout << " Frame " << frameIndex << " -> Free\n";
        }
    }
    std::cout << "--------------------------\n";
}

size_t PagingAllocator::allocateFrames(size_t numFrames, size_t processId, const std::vector<size_t>& pageSizes) {
    size_t frameIndex = freeFrameList.back();
    freeFrameList.pop_back();
    
    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList[frameIndex + i] = processId;
    }

    return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrames, size_t frameIndex, const std::vector<size_t>& pageSizes) {
    for (size_t i = 0; i < numFrames; ++i) {
        frameMap.erase(frameIndex + i);
    }

    for (size_t i = 0; i < numFrames; ++i) {
        freeFrameList.push_back(frameIndex + i);
    }
}






class FlatMemoryAllocator : public IMemoryAllocator {
public:
    FlatMemoryAllocator(size_t maxOverallMem, size_t memPerFrame)
        : maxOverallMem(maxOverallMem), memPerFrame(memPerFrame) {
        if (memPerFrame > 0)
        {
			freeFrames = maxOverallMem / memPerFrame;
        }
        else
        {
			freeFrames = 0;
        }
        memory.resize(maxOverallMem);
        std::fill(memory.begin(), memory.end(), '.');
        std::fill(allocationMap.begin(), allocationMap.end(), false);
    }

    void* allocate(size_t size) override {
        // Find the first available block that can accommodate the process
        for (size_t i = 0; i < freeFrames; i++) {
            if (!allocationMap[i] && canAllocateAt(i, size / memPerFrame)) {
                allocateAt(i, size / memPerFrame);
                return &memory[i * memPerFrame];
            }
        }

        // No available block found, return nullptr
        return nullptr;
    }

    void deallocate(void* ptr) override {
        // Find the index of the memory block to deallocate
        size_t index = static_cast<char*>(ptr) - &memory[0];
        size_t frameIndex = index / memPerFrame;
        if (allocationMap[frameIndex]) {
            deallocateAt(frameIndex);
        }
    }

    std::string visualizeMemory() override {
        std::stringstream ss;
        for (size_t i = 0; i < memory.size(); i += memPerFrame) {
            if (allocationMap[i / memPerFrame]) {
                ss << "P" << i / memPerFrame << std::endl;
            }
            else {
                ss << "." << std::endl;
            }
        }
        return ss.str();
    }

    size_t calculateExternalFragmentation() {
        size_t fragmentation = 0;
        size_t currentFreeBlock = 0;

        for (size_t i = 0; i < freeFrames; ++i) {
            if (!allocationMap[i]) {
                currentFreeBlock += memPerFrame;
            }
            else if (currentFreeBlock > 0) {
                fragmentation += currentFreeBlock;
                currentFreeBlock = 0;
            }
        }

        // Add the last free block if we ended on a free frame
        fragmentation += currentFreeBlock;
        return fragmentation;
    }


private:
    size_t maxOverallMem;
    size_t memPerFrame;
    size_t freeFrames;
    std::vector<char> memory;
    std::vector<bool> allocationMap;

    bool canAllocateAt(size_t frameIndex, size_t size) const {
        // Check if the memory block is large enough
        return (frameIndex + size <= freeFrames);
    }

    void allocateAt(size_t frameIndex, size_t size) {
        // Mark the memory block as allocated
        std::fill(allocationMap.begin() + frameIndex, allocationMap.begin() + frameIndex + size, true);
        freeFrames -= size;
    }

    void deallocateAt(size_t frameIndex) {
        // Mark the memory block as deallocated
        std::fill(allocationMap.begin() + frameIndex, allocationMap.begin() + frameIndex + 1, false);
        freeFrames++;
    }
};

// Modified Scheduler class to handle quantum only for RR
class Scheduler {
private:
    queue<Process> ready_queue;
    unordered_map<string, Process> running_processes;
    vector<Process> finished_processes;
    mutex queue_mutex;
    condition_variable cv;
    bool scheduler_done;
    atomic<int> active_cores{ 0 };
    map<int, uint32_t> core_quantum_remaining;
    unique_ptr<IMemoryAllocator> memoryAllocator;

public:
    Scheduler() : scheduler_done(false) {
        // Initialize quantum counters for all cores
        for (int i = 0; i < sysConfig.numCPU; i++) {
            core_quantum_remaining[i] = sysConfig.quantumCycles;
        }

        // Initialize memory allocator
        memoryAllocator = std::make_unique<FlatMemoryAllocator>(sysConfig.maxOverallMem, sysConfig.memPerFrame);
    }

    void addProcess(const Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        // Allocate memory for the process
        void* memoryPtr = memoryAllocator->allocate(process.getMemoryAllocated());
        if (memoryPtr) {
            Process updated_process = process;
            updated_process.setMemoryPtr(memoryPtr);
            ready_queue.push(updated_process);
        }
        else {
            // No available memory, put the process back at the end of the queue
            ready_queue.push(process);
        }
        cv.notify_one();
    }

    Process getNextProcess(int core_id) {
        unique_lock<mutex> lock(queue_mutex);

        cv.wait(lock, [&] {
            return !ready_queue.empty() || scheduler_done;
            });

        if (ready_queue.empty()) {
            return Process("", 0);
        }

        Process p = ready_queue.front();
        ready_queue.pop();
        p.setCoreId(core_id);
        active_cores++;

        // Reset quantum for this core
        core_quantum_remaining[core_id] = sysConfig.quantumCycles;

        // Add to running processes map
        running_processes.emplace(p.getId(), p);

        return p;
    }

    bool shouldPreempt(int core_id) {
        if (sysConfig.schedulerType != "rr") {
            return false;
        }

        lock_guard<mutex> lock(queue_mutex);
        return core_quantum_remaining[core_id] == 0;
    }

    void updateQuantum(int core_id) {
        if (sysConfig.schedulerType == "rr") {
            lock_guard<mutex> lock(queue_mutex);
            if (core_quantum_remaining[core_id] > 0) {
                core_quantum_remaining[core_id]--;
            }
        }
    }

    void updateProcessProgress(const Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        auto it = running_processes.find(process.getId());
        if (it != running_processes.end()) {
            it->second.setProgress(process.getProgress());
            it->second.setCurrentInstruction(process.getCurrentInstruction());
            it->second.setCyclesUntilNextInstruction(process.getCyclesUntilNextInstruction());
        }
    }

    void preemptProcess(Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        // Remove from running processes map
        auto it = running_processes.find(process.getId());
        if (it != running_processes.end()) {
            running_processes.erase(it);
            active_cores--;
        }
        // Add back to ready queue if not finished
        if (process.getProgress() < process.getBurstTime()) {
            ready_queue.push(process);
        }
        else {
            finished_processes.push_back(process);
        }
    }

    void markAsFinished(const Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        auto it = running_processes.find(process.getId());
        if (it != running_processes.end()) {
            finished_processes.push_back(it->second);
            running_processes.erase(it);
            active_cores--;
        }
        cv.notify_all();
    }

    bool allProcessesFinished() {
        lock_guard<mutex> lock(queue_mutex);
        return ready_queue.empty() && running_processes.empty();
    }
    
    void displayStatus() {
        lock_guard<mutex> lock(queue_mutex);

        cout << "------------------------------------------------" << endl;

        int utilization = (active_cores * 100.0) / sysConfig.numCPU;
        cout << "CPU Utilization: " << fixed << setprecision(2) << utilization << "%" << endl;
        cout << "Active Cores: " << active_cores << endl;
        cout << "Processes in queue: " << ready_queue.size() << endl;

        cout << "\nRunning processes:" << endl;
        for (const auto& entry : running_processes) {
            const auto& id = entry.first;
            const auto& process = entry.second;
            cout << process.getId() << "\t(" << process.getTimeStamp() << ")\tCore: " << process.getCoreId()
                << "\tProgress: " << process.getProgress() << "/" << process.getBurstTime() << endl;
        }

        cout << "\nFinished processes:" << endl;
        for (const auto& process : finished_processes) {
            cout << process.getId() << "\t(" << process.getTimeStamp() << ")\tFinished\t"
                << process.getBurstTime() << "/" << process.getBurstTime() << endl;
        }
    }

    void printStatus() {
        string filename = "csopesy-log.txt";
        ofstream outputFile(filename, ios::app);
        if (!outputFile.is_open()) {
            cerr << "Error: Could not open or create the file: " << filename << endl;
            return;
        }

        int utilization = (active_cores * 100.0) / sysConfig.numCPU;
        outputFile << "CPU Utilization: " << fixed << setprecision(2) << utilization << "%" << endl;
        outputFile << "Active Cores: " << active_cores << endl;
        outputFile << "Processes in queue: " << ready_queue.size() << endl;

        cout << "\nRunning processes:" << endl;
        for (const auto& entry : running_processes) {
            const auto& id = entry.first;
            const auto& process = entry.second;
            cout << process.getId() << "\t(" << process.getTimeStamp() << ")\tCore: " << process.getCoreId()
                << "\tProgress: " << process.getProgress() << "/" << process.getBurstTime() << endl;
        }

        outputFile << "\nFinished processes:" << endl;
        for (const auto& process : finished_processes) {
            outputFile << process.getId() << "\t(" << process.getTimeStamp() << ")\tFinished\t"
                << process.getBurstTime() << "/" << process.getBurstTime() << endl;
        }

        outputFile << "--------------------------------------\n" << endl;

        SetConsoleColor(GREEN);
        cout << "Report generated at " << filename << "!" << endl;
        SetConsoleColor(RESET);
        outputFile.close();
    }

    void takeMemorySnapshot(uint32_t quantumCycle) {
        lock_guard<mutex> lock(queue_mutex);

        string filename = "memory_stamp_" + to_string(quantumCycle) + ".txt";
        ofstream outputFile(filename, ios::trunc);
        if (!outputFile.is_open()) {
            cerr << "Error: Could not open or create the file: " << filename << endl;
            return;
        }

        outputFile << "Timestamp: (" << getCurrentTimestamp() << ")" << endl;
        outputFile << "Number of processes in memory: " << running_processes.size() << endl;

        size_t externalFragmentation = memoryAllocator->calculateExternalFragmentation();
        outputFile << "Total external fragmentation in KB: " << externalFragmentation << endl;

        outputFile << "----end---- = " << sysConfig.maxOverallMem << endl;

        for (const auto& entry : running_processes) {
            const auto& id = entry.first;
            const auto& process = entry.second;

            size_t start = reinterpret_cast<size_t>(process.getMemoryPtr()) / sysConfig.memPerFrame;
            size_t end = (reinterpret_cast<size_t>(process.getMemoryPtr()) + process.getMemoryAllocated() - 1) / sysConfig.memPerFrame;

            outputFile << "P" << process.getId() << endl;
            outputFile << end * sysConfig.memPerFrame << endl;
            outputFile << (start * sysConfig.memPerFrame) << endl;
        }


        outputFile << "----start---- = 0" << endl;
        outputFile.close();

        SetConsoleColor(GREEN);
        // cout << "Memory snapshot taken at " << filename << endl;
        SetConsoleColor(RESET);
    }


    bool isSchedulerDone() const {
        return scheduler_done;
    }

    void setSchedulerDone(bool value) {
        scheduler_done = value;
        cv.notify_all();
    }
};

// scheduler test functionality
void generateTestProcesses(Scheduler& scheduler) {
    mt19937 rng(random_device{}());
    int process_counter = 0;

    while (process_generator_running) {
        if (!sysConfig.isInitialized) {
            break;
        }

        uniform_int_distribution<> inst_dist(sysConfig.minInstructions, sysConfig.maxInstructions);
        int num_instructions = inst_dist(rng);

        string process_id = "p" + to_string(process_counter++);
        Process new_process(process_id, num_instructions);

        scheduler.addProcess(new_process);

        this_thread::sleep_for(chrono::milliseconds(sysConfig.batchProcessFreq * 700));
    }
}

// New function to handle CPU cycle updates in main loop
void updateCPUCycles(const Scheduler& scheduler) {
    while (!scheduler.isSchedulerDone()) {
        cpu_cycles++;
        this_thread::sleep_for(chrono::milliseconds(100)); // Adjust timing as needed
    }
}

// Core worker function to simulate CPU execution
void coreWorker(Scheduler& scheduler, int core_id) {
    uint32_t quantumCounter = 0;
    while (!scheduler.isSchedulerDone()) {
        Process p = scheduler.getNextProcess(core_id);
        if (p.getId().empty()) continue;

        bool was_preempted = false;
        while (p.getProgress() < p.getBurstTime() && !was_preempted) {
            // Check for preemption
            if (scheduler.shouldPreempt(core_id)) {
                scheduler.preemptProcess(p);
                was_preempted = true;
                break;
            }

            // Execute instruction
            bool instruction_completed = p.executeInstruction();

            if (instruction_completed) {
                scheduler.updateProcessProgress(p);
            }

            // Update quantum counter and take memory snapshot if needed
            scheduler.updateQuantum(core_id);
            quantumCounter++;
            if (quantumCounter % sysConfig.quantumCycles == 0) {
                scheduler.takeMemorySnapshot(quantumCounter / sysConfig.quantumCycles);
            }

            // Simulate execution delay
            if (sysConfig.delaysPerExec > 0) {
                for (uint32_t i = 0; i < sysConfig.delaysPerExec; i++) {
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(200));
            }
        }

        if (!was_preempted && p.getProgress() >= p.getBurstTime()) {
            scheduler.markAsFinished(p);
        }
    }
}

// Initialize command implementation
bool initializeSystem() {
    if (sysConfig.isInitialized) {
        SetConsoleColor(YELLOW);
        cout << "System is already initialized." << endl;
        SetConsoleColor(RESET);
        return true;
    }

    if (!readConfigFile("config.txt", sysConfig)) {
        SetConsoleColor(RED);
        cerr << "Failed to initialize system. Check config.txt file." << endl;
        SetConsoleColor(RESET);
        return false;
    }

    sysConfig.isInitialized = true;
    cout << "------------------------------------------------" << endl;
    SetConsoleColor(GREEN);
    cout << "System initialized successfully with:" << endl;
    cout << "CPUs: " << sysConfig.numCPU << endl;
    cout << "Scheduler: " << sysConfig.schedulerType << endl;
    if (sysConfig.schedulerType == "rr") {
        cout << "Quantum cycles: " << sysConfig.quantumCycles << endl;
    }
    cout << "Process generation frequency: " << sysConfig.batchProcessFreq << " cycles" << endl;
    cout << "Instructions per process: " << sysConfig.minInstructions << "-" << sysConfig.maxInstructions << endl;
    cout << "Delay per execution: " << sysConfig.delaysPerExec << " cycles" << endl;
    cout << "Max overall memory: " << sysConfig.maxOverallMem << " bytes" << endl;
    cout << "Memory per frame: " << sysConfig.memPerFrame << " bytes" << endl;
    cout << "Memory per process: " << sysConfig.memPerProc << " bytes" << endl;
    SetConsoleColor(RESET);

    return true;
}

// Modified execute function to enforce initialization requirement
void execute(Scheduler& scheduler, const vector<string>& cmd) {
    // Special handling for exit command - always allowed
    if (cmd[0] == "exit") {
        return;
    }


    // Check initialization status except for initialize command
    if (cmd[0] != "initialize" && !sysConfig.isInitialized) {
        SetConsoleColor(RED);
        cout << "Error: System not initialized. Please run 'initialize' command first." << endl;
        SetConsoleColor(RESET);
        return;
    }

    // Execute commands
    if (cmd[0] == "initialize") {
        initializeSystem();
    }
    else if (cmd[0] == "clear") {
        clearScreen();
    }
    else if (cmd[0] == "screen") {
        // Check if command has enough arguments
        if (cmd.size() < 2) {
            SetConsoleColor(RED);
            cout << "Error: Invalid screen command format." << endl;
            SetConsoleColor(RESET);
            return;
        }

        if (cmd[1] == "-ls") {
            scheduler.displayStatus();
        }
        else if (cmd[1] == "-s" || cmd[1] == "-r") {
            // These commands need a third argument (screen name)
            if (cmd.size() < 3) {
                SetConsoleColor(RED);
                cout << "Error: Missing screen name for " << cmd[1] << " command." << endl;
                SetConsoleColor(RESET);
                return;
            }

            if (cmd[1] == "-s") {
                createScreen(cmd[2]);
            }
            else { // cmd[1] == "-r"

                // Screen exists and process has finished
                if (screens.find(cmd[2]) != screens.end() && screens[cmd[2]].isFinished) {
                    cout << "Process " << cmd[2] << " not found." << endl;
                    //screens.erase(cmd[2]); // remove finished screen
                }
                else if (screens.find(cmd[2]) != screens.end() && !screens[cmd[2]].isFinished) {
                    resumeScreen(cmd[2]);
                }
                else {
                    cout << "Process " << cmd[2] << " not found." << endl;

                }
            }
        }
        else {
            displayError(cmd[1]);
        }
    }
    else if (cmd[0] == "scheduler-test") {
        if (process_generator_running) {
            SetConsoleColor(YELLOW);
            cout << "Process generator is already running." << endl;
            SetConsoleColor(RESET);
            return;
        }

        SetConsoleColor(GREEN);
        cout << "Starting process generator..." << endl;
        cout << "Generating processes every " << sysConfig.batchProcessFreq << " cycles" << endl;
        cout << "Instructions per process: " << sysConfig.minInstructions << "-" << sysConfig.maxInstructions << endl;
        cout << "Use 'screen -ls' to view processes and 'scheduler-stop' to stop generation" << endl;
        SetConsoleColor(RESET);

        process_generator_running = true;
        process_generator_thread = thread(generateTestProcesses, ref(scheduler));
    }
    else if (cmd[0] == "scheduler-stop") {
        if (!process_generator_running) {
            SetConsoleColor(YELLOW);
            cout << "Process generator is not running." << endl;
            SetConsoleColor(RESET);
            return;
        }

        SetConsoleColor(GREEN);
        cout << "Stopping process generator..." << endl;
        SetConsoleColor(RESET);

        process_generator_running = false;
        if (process_generator_thread.joinable()) {
            process_generator_thread.join();
        }
    }
    else if (cmd[0] == "report-util") {
        scheduler.printStatus();
    }
    else {
        displayError(cmd[0]);
    }
}

// Modified main function to enforce initialization
int main(int argc, const char* argv[]) {
    initializeMainMenuCmds();
    ofstream outputFile("csopesy-log.txt", ios::trunc);
    string input = "";
    vector<string> commandArgs;
    bool shouldExit = false;
    bool leftMainScreen = true;

    // Initialize scheduler but don't start threads yet
    static Scheduler scheduler;
    static vector<thread> worker_threads;

    thread cpu_cycle_thread;

    while (!shouldExit) {
        commandArgs.clear();

        if (currentScreen == "Main Menu" && leftMainScreen) {
            printHeader();
            leftMainScreen = false;
        }

        printInstruc();
        getline(cin, input);

        stringstream ss(input);
        string token;
        while (ss >> token) {
            commandArgs.push_back(token);
        }

        // Store commandArgs
        // Validate Commands
        string command = commandArgs[0];

        // Special handling for exit command
        if (command == "exit") {
            if (currentScreen == "Main Menu") {
                shouldExit = true;
            }
            else {
                currentScreen = "Main Menu";
                clearScreen();
                leftMainScreen = true;
            }
            continue;
        }

        // For Main Menu commands
        if (currentScreen == "Main Menu") {
            if (command == "initialize" || sysConfig.isInitialized) {
                if (validateCmd(command, MAIN_MENU_CMD)) {
                    displayRecognized(command);
                    execute(scheduler, commandArgs);

                    // Start worker threads after initialization
                    if (command == "initialize" && sysConfig.isInitialized) {
                        if (worker_threads.empty()) {
                            for (int i = 0; i < sysConfig.numCPU; ++i) {
                                worker_threads.emplace_back(coreWorker, ref(scheduler), i);
                            }
                            // Start CPU cycle update thread
                            cpu_cycle_thread = thread(updateCPUCycles, ref(scheduler));
                        }
                    }
                }
                else {
                    displayError(command);

                }
            }
            else {
                SetConsoleColor(RED);
                cout << "Error: System not initialized. Please run 'initialize' command first." << endl;
                SetConsoleColor(RESET);
            }
            leftMainScreen = false;
        }

        // For screen-specific commands
        else {
            if (validateCmd(command, screens.at(currentScreen).commandArr)) {
                //displayRecognized(command);
                // Update process information (lines of code being executed)
                if (command == "process-smi") {
                    // Check if the process exists in the screens map
                    if (screens.find(currentScreen) != screens.end()) {
                        ScreenInfo& info = screens[currentScreen];

                        // Display process details
                        if (info.currentLine != info.totalLines && !info.isFinished) {
                            cout << "Process: " << info.processName << endl;
                            cout << "ID: " << screens.size() << endl << endl;
                            cout << "Current instruction line: " << info.currentLine << endl;
                            cout << "Lines of code: " << info.totalLines << endl;
                        }
                        else {
                            cout << "Finished!" << endl;

                        }
                    }
                    else {
                        cout << "Process " << currentScreen << " not found." << endl;
                    }
                }
            }
            else {
                displayError(command);
            }
        }
    }

    // Cleanup when exiting
    scheduler.setSchedulerDone(true);

    // Stop and join CPU cycle thread
    if (cpu_cycle_thread.joinable()) {
        cpu_cycle_thread.join();
    }

    // Stop process generator if running
    if (process_generator_running) {
        process_generator_running = false;
        if (process_generator_thread.joinable()) {
            process_generator_thread.join();
        }
    }

    // Join worker threads
    for (auto& t : worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    SetConsoleColor(RESET);
    return 0;
}