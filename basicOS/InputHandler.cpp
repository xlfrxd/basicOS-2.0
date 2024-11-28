#include <iostream>

#include "InputCommands.h"
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"
#include "Process.h"

using namespace std;

InputCommands::InputCommands()
{
}

// stores the created instance of console manager
InputCommands* InputCommands::inputCommands = inputCommands;


void InputCommands::initialize()
{
    inputCommands = new InputCommands();
}

void InputCommands::destroy()
{
	delete inputCommands;
}

InputCommands* InputCommands::getInstance()
{
	return inputCommands;
}

void InputCommands::handleMainConsoleInput()
{
    cout << "Enter a command: ";
    string input;
    getline(cin, input); // Capture entire line input

    // Convert input to lowercase
    for (char &c : input) {
        c = tolower(c);
    }

    // Split input by spaces
    istringstream iss(input);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

    if (tokens.empty()) {
        cout << ConsoleColor::RED <<"Please enter a command" << ConsoleColor::RESET << endl;
        return;
    }

    string command = tokens[0];

    // Check initialization state once
    if (!ConsoleManager::getInstance()->getInitialized() && command != "initialize") {
        cout << ConsoleColor::RED << "Error: System not initialized. Please run 'initialize' command first." << ConsoleColor::RESET << endl;
        return;
    }

    // Main Console commands
    if (ConsoleManager::getInstance()->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
        if (command == "initialize") {
            if (!ConsoleManager::getInstance()->getInitialized()) {
                ConsoleManager::getInstance()->setInitialized(true);

                // Start scheduler
                Scheduler::getInstance()->initialize(ConsoleManager::getInstance()->getNumCpu());
                std::thread schedulerThread([&] {
                    Scheduler::getInstance()->start();
                    });
                schedulerThread.detach();

                cout << ConsoleColor::GREEN << "System initialized successfully with:" << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Number of CPUs: " << ConsoleManager::getInstance()->getNumCpu() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Scheduler Configuration: " << ConsoleManager::getInstance()->getSchedulerConfig() << ConsoleColor::RESET << endl;
                if (ConsoleManager::getInstance()->getSchedulerConfig() == "rr")
                {
                    cout << ConsoleColor::GREEN << "Time Slice: " << ConsoleManager::getInstance()->getTimeSlice() << ConsoleColor::RESET << endl;
                }
				cout << ConsoleColor::GREEN << "Batch Process Frequency: " << ConsoleManager::getInstance()->getBatchProcessFrequency() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Min Instructions: " << ConsoleManager::getInstance()->getMinIns() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Max Instructions: " << ConsoleManager::getInstance()->getMaxIns() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Delay Per Execution: " << ConsoleManager::getInstance()->getDelayPerExec() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Max Overall Memory: " << ConsoleManager::getInstance()->getMaxOverallMem() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Memory per Frame: " << ConsoleManager::getInstance()->getMemPerFrame() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN << "Min Memory per Process: " << ConsoleManager::getInstance()->getMinMemPerProc() << ConsoleColor::RESET << endl;
				cout << ConsoleColor::GREEN <<"Max Memory per Process: " << ConsoleManager::getInstance()->getMaxMemPerProc() << ConsoleColor::RESET << endl;
                cout << "------------------------------------------------" << endl;
            }
            else {
				cout << ConsoleColor::YELLOW << "System is already initialized." << ConsoleColor::RESET << endl;

            }
        }
        else if (command == "exit") {
            ConsoleManager::getInstance()->exitApplication();
        }
        else if (command == "scheduler-test") {
            if (!Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << ConsoleColor::GREEN << ConsoleManager::getInstance()->getSchedulerConfig() << " scheduler is now running." << ConsoleColor::RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(true);
                // create batchProcessFrequency number of processes
                std::thread schedulerTestThread([&] {
					ConsoleManager::getInstance()->schedulerTest();
                    });
                schedulerTestThread.detach();
               
            }
            else {
                cout << ConsoleColor::YELLOW << "Scheduler is already running" << ConsoleColor::RESET << endl;
            }
        }
        else if (command == "scheduler-stop") {
            if (Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << ConsoleColor::GREEN << "Scheduler has been stopped." << ConsoleColor::RESET << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(false);
            }
            else {
                cout << ConsoleColor::YELLOW << "Scheduler is not running." << ConsoleColor::RESET << endl;
            }
        }
        else if (command == "report-util") {
            ConsoleManager::getInstance()->reportUtil();
        }
        else if (command == "clear") {
            system("cls");
            ConsoleManager::getInstance()->drawConsole();
        }
        else if (command == "vmstat") {
            if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
                size_t memoryUsage = FlatMemoryAllocator::getInstance()->getTotalMemoryUsage();
            }
            else {
                size_t usedFrames = PagingAllocator::getInstance()->calculateUsedFrames();
                PagingAllocator::getInstance()->setUsedMemory(usedFrames * ConsoleManager::getInstance()->getMemPerFrame());
            }

            ConsoleManager::getInstance()->printVmstat();
        }
        else if (command == "process-smi") {
			ConsoleManager::getInstance()->printProcessSmi();
        }
        else if (command == "screen") {
            if (tokens.size() > 1) {
                string screenCommand = tokens[1];
                string processName = (tokens.size() > 2) ? tokens[2] : "";

                if (screenCommand == "-s" && !processName.empty()) {
                    if (ConsoleManager::getInstance()->getScreenMap().contains(processName)) {
                        cout << ConsoleColor::YELLOW << "Screen already exists." << ConsoleColor::RESET << endl;
                    }
                    else {
                        string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
                        // TODO: should be either 256 or 512 if num per memory frame is 256?
                        auto screenInstance = std::make_shared<Process>(processName, 0, timestamp, ConsoleManager::getInstance()->getMinMemPerProc());
                        ConsoleManager::getInstance()->registerConsole(screenInstance);

                        ConsoleManager::getInstance()->switchConsole(processName);
                        ConsoleManager::getInstance()->drawConsole();
                        Scheduler::getInstance()->addProcessToQueue(screenInstance);
                    }
                }
                else if (screenCommand == "-r" && !processName.empty()) {
                    ConsoleManager::getInstance()->switchConsole(processName);
                    ConsoleManager::getInstance()->drawConsole();
                }
                else if (screenCommand == "-ls") {
                    ConsoleManager::getInstance()->displayProcessList();
                    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
                        FlatMemoryAllocator::getInstance()->visualizeBackingStore();
                    }
                    else {
                        PagingAllocator::getInstance()->visualizeBackingStore();
                    }
                }
                else {
                    cout << ConsoleColor::RED << "Invalid command" << ConsoleColor::RESET << endl;
                }
            }
            else {
                cout << ConsoleColor::RED << "Invalid command" << ConsoleColor::RESET << endl;
            }
        }
        else {
            cout << ConsoleColor::RED << "Invalid command" << ConsoleColor::RESET << endl;
        }
    }
    else {
        // Process-specific commands
        if (command == "exit") {
            ConsoleManager::getInstance()->switchConsole(MAIN_CONSOLE);
        }
        else if (command == "process-smi") {
            ConsoleManager::getInstance()->printProcessSmi();
        }
        else {
            cout << ConsoleColor::RED << "Invalid command" << ConsoleColor::RED << endl;
        }
    }
}
