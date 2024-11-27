#include "Scheduler.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "FlatMemoryAllocator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "PagingAllocator.h"


Scheduler::Scheduler(int numCores)
    : numCores(numCores), schedulerRunning(false),
    coresUsed(0), coresAvailable(numCores), 
    activeThreads(0),
    processQueueMutex(), processQueueCondition() {}

Scheduler* Scheduler::scheduler = nullptr;

void Scheduler::initialize(int numCores) {
    scheduler = new Scheduler(numCores);
}

Scheduler::~Scheduler() {
    stop();
}

Scheduler::Scheduler() {
    this->numCores = -1;
    this->schedulerRunning = false;
    this->coresAvailable = 0;
}
void Scheduler::start() {
    schedulerRunning = true;
    algorithm = ConsoleManager::getInstance()->getSchedulerConfig();
    for (int i = 0; i < numCores; i++) {
        // Launch each core on a separate detached thread
        
        std::thread([this, i]() {
            while (schedulerRunning) {
                std::shared_ptr<Process> process;

                {
                    std::unique_lock<std::mutex> lock(processQueueMutex);
                    processQueueCondition.wait(lock, [this]() { return !processQueue.empty() || !schedulerRunning; });

                    if (!schedulerRunning) return;

                    process = processQueue.front();
                    processQueue.pop();
                    ++activeThreads; // Increment active thread count
                }

				void* memoryPtr = nullptr;
			
				if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
                    memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);
				}
                else {
					memoryPtr = PagingAllocator::getInstance()->allocate(process);
                }

                if (memoryPtr) {
                    coresAvailable--;
                    coresUsed++;
                    process->setCPUCoreID(i);
                    workerFunction(i, process, memoryPtr);
                }
                else {
                    addProcessToQueue(process);
                }

                // Update core tracking after process completion
                {
                    std::lock_guard<std::mutex> lock(processQueueMutex);


                    if (memoryPtr) {
                        coresAvailable++;
                        coresUsed--;
                    }

                    --activeThreads; // Decrement active thread count
                    if (processQueue.empty() && activeThreads == 0) {
                        schedulerRunning = false;
                        processQueueCondition.notify_all();
                        coresUsed = 0;
                        coresAvailable = ConsoleManager::getInstance()->getNumCpu();
                    }
                }
            }
            }).detach(); // Detach thread for independent execution
    }
}

int Scheduler::getCoresUsed() const {
    return coresUsed;
}

int Scheduler::getCoresAvailable() const {
    return coresAvailable;
}

int Scheduler::getIdleCpuTicks()
{
    return idleCpuTicks;
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        schedulerRunning = false;
    }
    processQueueCondition.notify_all();
}

void Scheduler::workerFunction(int core, std::shared_ptr<Process> process, void* memoryPtr) {
    string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();


    // Ensure the process keeps its original core for FCFS and RR
    if (process->getCPUCoreID() == -1) {
        // If core is not yet assigned, set the current core as the affinity core
        process->setCPUCoreID(core);
    }
    else {
        // Otherwise, ensure the process stays on its assigned core
        core = process->getCPUCoreID();
    }

    if (algorithm == "fcfs") {
        // First-Come, First-Served logic
        for (int i = 0; i < process->getTotalLine(); i++) {
            if (ConsoleManager::getInstance()->getDelayPerExec() != 0) {
                for (int i = 0; i < ConsoleManager::getInstance()->getDelayPerExec(); i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            process->setCurrentLine(process->getCurrentLine() + 1);
            // Increment active cpu tick
            cpuCycles++;

            if (coresAvailable > 0) {
                idleCpuTicks += coresAvailable;
            }
        }
    }
	
    else if (algorithm == "rr") {
       // Round-Robin logic
       int quantum = ConsoleManager::getInstance()->getTimeSlice();  // Get RR time slice

       // Process for the duration of the quantum or until the process is finished
       for (int i = 0; i < quantum && process->getCurrentLine() < process->getTotalLine(); i++) {
           if (ConsoleManager::getInstance()->getDelayPerExec() != 0) {
               for (int i = 0; i < ConsoleManager::getInstance()->getDelayPerExec(); i++) {
                   std::this_thread::sleep_for(std::chrono::milliseconds(100));
               }
           }
		   else {
			   std::this_thread::sleep_for(std::chrono::milliseconds(100));
           }
           process->setCurrentLine(process->getCurrentLine() + 1);

           // Increment active cpu tick
           cpuCycles++;

           if (coresAvailable > 0) {
               idleCpuTicks += coresAvailable;
           }
       }

        //if process is not finished, re-queue it but retain its core affinity
       if (process->getCurrentLine() < process->getTotalLine()) {
           std::lock_guard<std::mutex> lock(processQueueMutex);
           processQueue.push(process);  // Re-queue the unfinished process
           processQueueCondition.notify_one();
       }
    }

    // Deallocate frames after execution 
    if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
        FlatMemoryAllocator::getInstance()->deallocate(memoryPtr, process);
    }
    else {
        PagingAllocator::getInstance()->deallocate(process);
    }
    

    string timestampFinished = ConsoleManager::getInstance()->getCurrentTimestamp();
    process->setTimestampFinished(timestampFinished);  // Log completion time
}


void Scheduler::addProcessToQueue(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        processQueue.push(process);
    }
    processQueueCondition.notify_one();  // Notify one waiting thread
}

Scheduler* Scheduler::getInstance() {
    if (scheduler == nullptr) {
        scheduler = new Scheduler();
    }
    return scheduler;
}

int Scheduler::getCpuCycles() const {
    return cpuCycles;
}

void Scheduler::setCpuCycles(int cpuCycles) {
    this->cpuCycles = cpuCycles;
}

bool Scheduler::getSchedulerTestRunning() const {
    return schedulerTestRunning;
}

void Scheduler::setSchedulerTestRunning(int schedulerTestRunning) {
    this->schedulerTestRunning = schedulerTestRunning;
}
