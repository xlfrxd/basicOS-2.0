#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "Process.h"

class Scheduler {
public:

    enum ProcessState
    {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    Scheduler(int numCores);
    Scheduler();
    ~Scheduler();
    void start();
    void stop();
    void addProcessToQueue(std::shared_ptr<Process> process);
    void workerFunction(int core, std::shared_ptr<Process> process, void* ptr);
    static Scheduler* getInstance();
    static void initialize(int numCores);
    bool getSchedulerTestRunning() const;
    void setSchedulerTestRunning(int schedulerTestRunning);
    void addToFrontOfProcessQueue(std::shared_ptr<Process> process);

    int getCoresUsed() const;
    int getCoresAvailable() const;
    int getIdleCpuTicks();
    int coresUsed = 0;
    int coresAvailable;
    int getCpuCycles() const;
    void setCpuCycles(int cpuCycles);

private:
    int numCores;
    int cpuCycles = 0;
    int idleCpuTicks = 0;
    bool schedulerRunning;
    int activeThreads;
    bool schedulerTestRunning = false;
    std::vector<std::thread> workerThreads;
    std::queue<std::shared_ptr<Process>> processQueue;
    std::mutex processQueueMutex;
    std::condition_variable processQueueCondition;
    static Scheduler* scheduler;
    string algorithm = "";
};

#endif // SCHEDULER_H
