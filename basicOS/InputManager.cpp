using namespace std;

#include "InputCommands.h"
#include <iostream>
#include "ConsoleManager.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"
#include "Process.h"

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
        cout << "No command entered." << endl;
        return;
    }

    string command = tokens[0];

    // Check initialization state once
    if (!ConsoleManager::getInstance()->getInitialized() && command != "initialize") {
        cout << "Please initialize the processor configuration first." << endl;
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

                cout << "'Processor Configuration Initialized'" << endl;
            }
            else {
				cout << "Processor Configuration already initialized." << endl;

            }
        }
        else if (command == "exit") {
            ConsoleManager::getInstance()->exitApplication();
        }
        else if (command == "scheduler-test") {
            if (!Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << "Scheduler Test now running" << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(true);
                // create batchProcessFrequency number of processes
                std::thread schedulerTestThread([&] {
					ConsoleManager::getInstance()->schedulerTest();
                    });
                schedulerTestThread.detach();
               
            }
            else {
                cout << "Scheduler Test already running" << endl;
            }
        }
        else if (command == "scheduler-stop") {
            if (Scheduler::getInstance()->getSchedulerTestRunning()) {
                cout << "Scheduler Test stopped" << endl;
                Scheduler::getInstance()->setSchedulerTestRunning(false);
            }
            else {
                cout << "Scheduler Test not running" << endl;
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
                        cout << "Screen already exists." << endl;
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
                    FlatMemoryAllocator::getInstance()->visualizeBackingStore();
                }
                else {
                    cout << "Command not recognized." << endl;
                }
            }
            else {
                cout << "Command not recognized." << endl;
            }
        }
        else {
            cout << "Command not recognized." << endl;
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
            cout << "Command not recognized." << endl;
        }
    }
}

    
