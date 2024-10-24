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
#include <random>

using namespace std;

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
    cout << "\n========== Screen: ";
    SetConsoleColor(GREEN);
    cout << screen.processName;
    SetConsoleColor(RESET);
    cout << " ==========\n";
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
    cout << "=========================================\n";
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

// Add these declarations near the top of basicOS.cpp
void createScreenSilent(const string& screenName); // New function for creating screen without display

// Modify createScreen to have an optional display parameter
void createScreen(const string& screenName, bool shouldDisplay = true) {
    if (screens.find(screenName) == screens.end()) {
        ScreenInfo newScreen;
        newScreen.processName = screenName;
        newScreen.currentLine = 1;
        newScreen.totalLines = 100;
        newScreen.creationTimestamp = getCurrentTimestamp();
        newScreen.logFileName = screenName + "_log.txt";
        newScreen.commandArr.push_back("exit");
        newScreen.commandArr.push_back("print");
        screens[screenName] = newScreen;

        // Only switch to the screen and display it if shouldDisplay is true
        if (shouldDisplay) {
            currentScreen = screenName;
            displayScreen(screens[screenName]);
        }

        // Add default log file header
        string content = "";
        writeToFile(newScreen.logFileName, content, newScreen.processName);
    }
    else {
        cout << "Screen '" << screenName << "' already exists.\n";
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

// Configuration structure to hold system parameters
struct SystemConfig {
    int numCPU;                  // Number of CPUs [1-128]
    string schedulerType;        // "fcfs" or "rr"
    uint32_t quantumCycles;      // Time slice for RR
    uint32_t batchProcessFreq;   // Process generation frequency
    uint32_t minInstructions;    // Minimum instructions per process
    uint32_t maxInstructions;    // Maximum instructions per process
    uint32_t delaysPerExec;      // Delay cycles between instructions
    bool isInitialized;          // Track if system is initialized

    SystemConfig() : isInitialized(false) {}

    // Validate configuration parameters
    bool validate() const {
        if (numCPU < 1 || numCPU > 128) {
            cerr << "Error: num-cpu must be between 1 and 128" << endl;
            return false;
        }

        if (schedulerType != "fcfs" && schedulerType != "rr") {
            cerr << "Error: scheduler must be 'fcfs' or 'rr'" << endl;
            return false;
        }

        if (schedulerType == "rr" && quantumCycles < 1) {
            cerr << "Error: quantum-cycles must be at least 1 for RR scheduler" << endl;
            return false;
        }

        if (batchProcessFreq < 1) {
            cerr << "Error: batch-process-freq must be at least 1" << endl;
            return false;
        }

        if (minInstructions < 1 || maxInstructions < minInstructions) {
            cerr << "Error: Invalid instruction range. min-ins must be >= 1 and <= max-ins" << endl;
            return false;
        }

        return true;
    }
};

// Global configuration object
SystemConfig sysConfig;

// Function to read and parse config file
bool readConfigFile(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        SetConsoleColor(RED);
        cerr << "Error: Could not open config file: " << filename << endl;
        SetConsoleColor(RESET);
        return false;
    }

    string line;
    map<string, string> configValues;

    // Read config file line by line
    while (getline(configFile, line)) {
        stringstream ss(line);
        string key, value;

        if (ss >> key >> value) {
            configValues[key] = value;
        }
    }

    // Parse and validate all required parameters
    try {
        if (configValues.find("num-cpu") == configValues.end()) throw runtime_error("num-cpu not found");
        if (configValues.find("scheduler") == configValues.end()) throw runtime_error("scheduler not found");
        if (configValues.find("quantum-cycles") == configValues.end()) throw runtime_error("quantum-cycles not found");
        if (configValues.find("batch-process-freq") == configValues.end()) throw runtime_error("batch-process-freq not found");
        if (configValues.find("min-ins") == configValues.end()) throw runtime_error("min-ins not found");
        if (configValues.find("max-ins") == configValues.end()) throw runtime_error("max-ins not found");
        if (configValues.find("delays-per-exec") == configValues.end()) throw runtime_error("delays-per-exec not found");

        sysConfig.numCPU = stoi(configValues["num-cpu"]);
        sysConfig.schedulerType = configValues["scheduler"];
        sysConfig.quantumCycles = stoul(configValues["quantum-cycles"]);
        sysConfig.batchProcessFreq = stoul(configValues["batch-process-freq"]);
        sysConfig.minInstructions = stoul(configValues["min-ins"]);
        sysConfig.maxInstructions = stoul(configValues["max-ins"]);
        sysConfig.delaysPerExec = stoul(configValues["delays-per-exec"]);

    }
    catch (const exception& e) {
        SetConsoleColor(RED);
        cerr << "Error parsing config file: " << e.what() << endl;
        SetConsoleColor(RESET);
        return false;
    }

    return sysConfig.validate();
}

// enums before the Process class definition
enum class InstructionType {
    PRINT,
    SLEEP
};

struct Instruction {
    InstructionType type;
    int cycles;       // For SLEEP instructions, number of cycles to sleep
};

// Class to represent a process with randomized execution time per instruction
class Process {
public:
    string id;
    int burst_time;
    int progress;
    int core_id;
    chrono::system_clock::time_point arrival_time;
    mt19937 rng;
    vector<Instruction> instructions;
    int current_instruction;
    int cycles_until_next_instruction;
    bool instruction_completed;

    Process(const string& pid, int burst)
        : id(pid),
        burst_time(burst),
        progress(0),
        core_id(-1),
        arrival_time(chrono::system_clock::now()),
        rng(random_device{}()),
        current_instruction(0),
        cycles_until_next_instruction(0),
        instruction_completed(false) {
        generateInstructions();
    }

    void generateInstructions() {
        uniform_int_distribution<> sleep_dist(1, 5); // Random sleep cycles between 1-5

        for (int i = 0; i < burst_time; i++) {
            // Alternate between PRINT and SLEEP instructions
            if (i % 2 == 0) {
                instructions.push_back({ InstructionType::PRINT, 1 });
            }
            else {
                instructions.push_back({ InstructionType::SLEEP, sleep_dist(rng) });
            }
        }
    }

    // Returns true if an instruction was completed this cycle
    bool executeInstruction() {
        if (current_instruction >= instructions.size()) {
            return false;
        }

        instruction_completed = false;

        if (cycles_until_next_instruction > 0) {
            cycles_until_next_instruction--;
            return false;
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
            // cycles_until_next_instruction = instruction.cycles;
            break;
        }

        // Move to next instruction if current one is complete
        if (instruction_completed) {
            current_instruction++;
            progress++;
            return true;
        }

        return false;
    }

    // Get random execution time for each instruction
    chrono::milliseconds getInstructionTime() {
        // Random time between 100ms and 1000ms
        uniform_int_distribution<> dist(100, 1000);
        return chrono::milliseconds(dist(rng));
    }

    string getTimeStamp() const {
        time_t now = chrono::system_clock::to_time_t(arrival_time);
        tm time_info;
        localtime_s(&time_info, &now);
        ostringstream oss;
        oss << put_time(&time_info, "%m/%d/%Y %I:%M:%S%p");
        return oss.str();
    }
};

// Modified Scheduler class to handle quantum only for RR
class Scheduler {
private:
    queue<Process> ready_queue;
    vector<Process> running_processes;
    vector<Process> finished_processes;
    mutex queue_mutex;
    condition_variable cv;
    bool scheduler_done;
    atomic<int> active_cores{ 0 };
    map<int, uint32_t> core_quantum_remaining;

public:
    Scheduler() : scheduler_done(false) {
        // Initialize quantum counters for all cores
        for (int i = 0; i < sysConfig.numCPU; i++) {
            core_quantum_remaining[i] = sysConfig.quantumCycles;
        }
    }

    void addProcess(const Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        // Save current progress before adding back to queue
        Process updated_process = process;
        ready_queue.push(updated_process);
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
        p.core_id = core_id;
        active_cores++;

        // Reset quantum for this core
        core_quantum_remaining[core_id] = sysConfig.quantumCycles;

        // Add to running processes
        running_processes.push_back(p);
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
        auto it = find_if(running_processes.begin(), running_processes.end(),
            [&process](const Process& p) { return p.id == process.id; });
        if (it != running_processes.end()) {
            it->progress = process.progress;
            it->current_instruction = process.current_instruction;
            it->cycles_until_next_instruction = process.cycles_until_next_instruction;
        }
    }

    void preemptProcess(Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        // Remove from running processes
        auto it = find_if(running_processes.begin(), running_processes.end(),
            [&process](const Process& p) { return p.id == process.id; });
        if (it != running_processes.end()) {
            running_processes.erase(it);
            active_cores--;
        }
        // Add back to ready queue if not finished
        if (process.progress < process.burst_time) {
            ready_queue.push(process);
        }
        else {
            finished_processes.push_back(process);
        }
    }

    void markAsFinished(const Process& process) {
        lock_guard<mutex> lock(queue_mutex);
        auto it = find_if(running_processes.begin(), running_processes.end(),
            [&process](const Process& p) { return p.id == process.id; });
        if (it != running_processes.end()) {
            finished_processes.push_back(*it);
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

        cout << "\n--------------------------------------" << endl;
        cout << "Active Cores: " << active_cores << endl;
        cout << "Processes in queue: " << ready_queue.size() << endl;

        cout << "\nRunning processes:" << endl;
        for (const auto& p : running_processes) {
            cout << p.id << "\t(" << p.getTimeStamp() << ")\tCore: " << p.core_id
                << "\tProgress: " << p.progress << "/" << p.burst_time << endl;
        }

        cout << "\nFinished processes:" << endl;
        for (const auto& p : finished_processes) {
            cout << p.id << "\t(" << p.getTimeStamp() << ")\tFinished\t"
                << p.burst_time << "/" << p.burst_time << endl;
        }

        cout << "--------------------------------------\n" << endl;
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

        this_thread::sleep_for(chrono::milliseconds(sysConfig.batchProcessFreq * 100));
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
    while (!scheduler.isSchedulerDone()) {
        Process p = scheduler.getNextProcess(core_id);
        if (p.id.empty()) continue;

        bool was_preempted = false;
        while (p.progress < p.burst_time && !was_preempted) {
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

            // Update quantum counter
            scheduler.updateQuantum(core_id);

            // Simulate execution delay
            for (uint32_t i = 0; i < sysConfig.delaysPerExec; i++) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }

        if (!was_preempted && p.progress >= p.burst_time) {
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

    if (!readConfigFile("config.txt")) {
        SetConsoleColor(RED);
        cerr << "Failed to initialize system. Check config.txt file." << endl;
        SetConsoleColor(RESET);
        return false;
    }

    sysConfig.isInitialized = true;
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
                resumeScreen(cmd[2]);
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
        SetConsoleColor(YELLOW);
        cout << "Report utility not implemented." << endl;
        SetConsoleColor(RESET);
    }
    else {
        displayError(cmd[0]);
    }
}

// Modified main function to enforce initialization
int main(int argc, const char* argv[]) {
    initializeMainMenuCmds();
    string input = "";
    vector<string> commandArgs;
    bool shouldExit = false;

    // Initialize scheduler but don't start threads yet
    static Scheduler scheduler;
    static vector<thread> worker_threads;

    thread cpu_cycle_thread;

    while (!shouldExit) {
        commandArgs.clear();

        if (currentScreen == "Main Menu") {
            printHeader();
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
        }

        /*
        // Non-Main Menu Commands
        //TODO: Append this to a function
        // Check if exit
        if (command == "exit") {
            // Check if current screen is Main Menu
            if (currentScreen == "Main Menu") break; // Exit program
            // Exit from current screen
            currentScreen = "Main Menu"; // Set current screen to Main Menu
            clearScreen();
            continue;
        }
        else if (command == "print") {
            if (currentScreen != "Main Menu") {
                displayRecognized(command);
                int CORES = 4;
                for (int i = 0; i < 100; ++i) { // Simulate 100 print commands
                    int coreId = i % CORES;
                    logPrintCommand(screens.at(currentScreen).logFileName, coreId, screens.at(currentScreen).processName);
                }
            }
            else {
                input = "";
                clearScreen();
                if (screens.empty()) {
                    SetConsoleColor(RED);
                    cout << "ERROR: No process have been saved\n";
                    SetConsoleColor(RESET);
                }
                else {
                    // Loop through each screen in the map and print the contents of its associated text file
                    for (const auto& screen : screens) {
                        printLogToText(screen.first);  // screen.first contains the screen name (e.g., "p1")
                    }
                }
                SetConsoleColor(YELLOW);
                cout << "Enter 'enter key' to return to the Main Menu.\n";
                SetConsoleColor(RESET);
                cout << "=========================================\n";
                printInstruc();
                getline(cin, input); // Reads entire line
                clearScreen();
            }
        }
        */

        // For screen-specific commands
        else {
            if (validateCmd(command, screens.at(currentScreen).commandArr)) {
                // Handle screen-specific commands
                // ... (existing screen command handling)
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