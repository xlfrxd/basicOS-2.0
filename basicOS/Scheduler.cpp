#include <iostream>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "Scheduler.h"
#include "ConsoleManager.h"
#include "Process.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"

using namespace std;

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

        std::thread([this, i]() {
            while (schedulerRunning) {
                std::shared_ptr<Process> process;

                {
                    std::unique_lock<std::mutex> lock(processQueueMutex);
                    processQueueCondition.wait(lock, [this]() { return !processQueue.empty() || !schedulerRunning; });

                    if (!schedulerRunning) return;

                    process = processQueue.front();
                    processQueue.pop();
                    ++activeThreads;
                }

                void* memoryPtr = nullptr;
                bool isFlatMemory = ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc();
                bool processInMemory = false;


                if (isFlatMemory) {
                    void* tempPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(process->getMemoryRequired(), process->getProcessName(), process);

                    if (tempPtr) {
                        memoryPtr = tempPtr;
                    }

                    // allocate the memory
                    else {
                        memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);
                    }
                }



                // paging
                else {
                    // check if process is in memory
                    processInMemory = PagingAllocator::getInstance()->isProcessInMemory(process->getProcessName());

                    if (processInMemory) {
						// Does nothing, process is already in memory
                    }
                    // allocate the memory
                    else {
                        processInMemory = PagingAllocator::getInstance()->allocate(process);


                    }
                }


                if (memoryPtr || processInMemory) {
                    coresAvailable--;
                    coresUsed++;
                    process->setCPUCoreID(i);
                    process->setIsRunning(true);
                    workerFunction(i, process, memoryPtr);
                }
                else {
                    if (algorithm == "fcfs") {
                        addToFrontOfProcessQueue(process);
                    }
                    else {
                        coresAvailable--;
                        coresUsed++;
                        if (isFlatMemory) {
                            std::shared_ptr<Process> oldestProcess = FlatMemoryAllocator::getInstance()->findOldestProcess();
                            void* oldestMemoryPtr = FlatMemoryAllocator::getInstance()->getMemoryPtr(oldestProcess->getMemoryRequired(), oldestProcess->getProcessName(), oldestProcess);

                            FlatMemoryAllocator::getInstance()->deallocate(oldestMemoryPtr, oldestProcess);

                            // oldest process back to backing store
                            FlatMemoryAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                            // if the new process is in backing store, remove it from the backing store
                            FlatMemoryAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);

                            // allocate the new process
                            void* memoryPtr = FlatMemoryAllocator::getInstance()->allocate(process->getMemoryRequired(), process->getProcessName(), process);

                            if (memoryPtr) {
                                process->setCPUCoreID(i);
                                process->setIsRunning(true);
                                workerFunction(i, process, memoryPtr);
                            }
                        }
                        else {
                            string oldestProcessStr = PagingAllocator::getInstance()->findOldestProcess();
                            std::shared_ptr<Process> oldestProcess = ConsoleManager::getInstance()->getScreenByProcessName(oldestProcessStr);

                            PagingAllocator::getInstance()->deallocate(oldestProcess);

                            PagingAllocator::getInstance()->allocateFromBackingStore(oldestProcess);

                            PagingAllocator::getInstance()->findAndRemoveProcessFromBackingStore(process);

                            bool processInMemory = PagingAllocator::getInstance()->allocate(process);

                            if (processInMemory) {
                                process->setCPUCoreID(i);
                                workerFunction(i, process, memoryPtr);
                            }
                        }

                    }

                }

                // Update core tracking after process completion
                {
                    std::lock_guard<std::mutex> lock(processQueueMutex);
                    --activeThreads;

                    if (processQueue.empty() && activeThreads == 0) {
                        schedulerRunning = false;
                        processQueueCondition.notify_all();
                        coresUsed = 0;
                        coresAvailable = ConsoleManager::getInstance()->getNumCpu();
                    }
                }
            }
            }).detach();
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
        process->setCPUCoreID(core);
    }
    else {
        core = process->getCPUCoreID();
    }

    if (algorithm == "fcfs") {
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
            cpuCycles++;

            if (coresAvailable > 0) {
                idleCpuTicks += coresAvailable;
            }
        }

        // subtract cores utilization
        {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            coresAvailable++;
            coresUsed--;
        }

        // deallocate memory
        if (ConsoleManager::getInstance()->getMinMemPerProc() == ConsoleManager::getInstance()->getMaxMemPerProc()) {
            FlatMemoryAllocator::getInstance()->deallocate(memoryPtr, process);
        }
        else {
            PagingAllocator::getInstance()->deallocate(process);
        }


    }

    else if (algorithm == "rr") {
        int quantum = ConsoleManager::getInstance()->getTimeSlice();

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

            cpuCycles++;

            if (coresAvailable > 0) {
                idleCpuTicks += coresAvailable;
            }
        }

        if (process->getCurrentLine() < process->getTotalLine()) {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            processQueue.push(process);  // Re-queue the unfinished process
            processQueueCondition.notify_one();
        }


        process->setIsRunning(false);
        {
            std::lock_guard<std::mutex> lock(processQueueMutex);
            coresAvailable++;
            coresUsed--;
        }

    }


    string timestampFinished = ConsoleManager::getInstance()->getCurrentTimestamp();
    process->setTimestampFinished(timestampFinished);
}


void Scheduler::addProcessToQueue(std::shared_ptr<Process> process) {
    {
        std::lock_guard<std::mutex> lock(processQueueMutex);
        processQueue.push(process);
    }
    processQueueCondition.notify_one();
}

void Scheduler::addToFrontOfProcessQueue(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(processQueueMutex);

    // Create a temporary queue and add the new process at the front
    std::queue<std::shared_ptr<Process>> tempQueue;
    tempQueue.push(process);

    // Add the remaining processes to the temporary queue
    while (!processQueue.empty()) {
        tempQueue.push(processQueue.front());
        processQueue.pop();
    }

    // Replace the original queue with the temporary queue
    processQueue = std::move(tempQueue);

    processQueueCondition.notify_all();
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