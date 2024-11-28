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
                bool isFlatMemory = ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc();
                bool isPagingRunning = false;

                if (process->getIsRunning() == false) {
                    if (isFlatMemory) {
                        memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);
                    }
                    else {
                        memoryPtr = PagingAllocator::getInstance()->allocate(process);
                    }
                }
                else {
                    if (isFlatMemory) {
                        memoryPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(process->getMemoryRequired(), process->getProcessName(), process);
                    }
                    else {
                        memoryPtr = PagingAllocator::getInstance()->isProcessAllocated(process->getProcessName());
                    }
                }


                if (isFlatMemory) {
                    if (memoryPtr) {
                        coresAvailable--;
                        coresUsed++;
                        process->setCPUCoreID(i);
                        workerFunction(i, process, memoryPtr);
                    }

                    else {

                        // add to backing store
                        if (isFlatMemory) {
                            // get oldest process
                            std::shared_ptr<Process> oldestProcess = FlatMemoryAllocator::getInstance()->findOldestProcess();
                            //cout << "Oldest process: " << oldestProcess->getProcessName() << endl;
                            // get memory ptr of the oldest process
                            void* oldestMemoryPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(oldestProcess->getMemoryRequired(), oldestProcess->getProcessName(), oldestProcess);


                            // deallocate the oldest process
                            FlatMemoryAllocator::getInstance()->deallocate(oldestMemoryPtr, oldestProcess);

                            // put the oldest process back to backing store
                            FlatMemoryAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                            // if the process is in backing store, remove it from the backing store
                            FlatMemoryAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);
                            // allocate the new process
                            void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);

                            if (memoryPtr) {
                                process->setCPUCoreID(i);
                                workerFunction(i, process, memoryPtr);
                            }
                        }
                    }
                }
                else {
                    /*    cout << "PAGING: " << isPagingRunning << endl;*/
                    cout << "MEMORY: " << memoryPtr << endl;

                    if (memoryPtr) {
                        coresAvailable--;
                        coresUsed++;
                        process->setCPUCoreID(i);
                        workerFunction(i, process, memoryPtr);
                    }
                    else {
                        // get oldest process


                        string oldestProcessStr = PagingAllocator::getInstance()->findOldestProcess();
                        std::shared_ptr<Process> oldestProcess = ConsoleManager::getInstance()->getScreenByProcessName(oldestProcessStr);

                        /*if (oldestProcess) {
                            cout << "Existing" << endl;
                        }
                        else {
                            cout << "NOT EXISTING" << endl;
                        }*/

                        /*cout << "OLDEST: " << oldestProcessStr << endl;*/

                        PagingAllocator::getInstance()->deallocate(oldestProcess);

                        PagingAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                        PagingAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);

                        void* memoryPtr = PagingAllocator::getInstance()->allocate(process);

                        if (memoryPtr) {
                            process->setCPUCoreID(i);
                            workerFunction(i, process, memoryPtr);
                        }
                    }



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
        else {
            if (ConsoleManager::getInstance()->getMinMemPerProc() != ConsoleManager::getInstance()->getMaxMemPerProc()) {
                PagingAllocator::getInstance()->deallocate(process);
            }
            else {
                FlatMemoryAllocator::getInstance()->deallocate(memoryPtr, process);  // Deallocate memory
            }
        }

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