#pragma once

#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>

#include "Process.h"
#include "ConsoleColor.h"
#include "Scheduler.h"

const string MAIN_CONSOLE = "MAIN_CONSOLE";

class ConsoleManager
{	
public:
	enum ProcessState
	{
		READY,
		RUNNING,
		WAITING,
		FINISHED
	};

	ConsoleManager();

	static ConsoleManager* getInstance();

	static void initialize();

	void drawConsole();
	void printHeader();
	void destroy();
	void exitApplication();

	// Configuration getters and setters
	void initializeConfiguration();
	void initializeAllocators();

	// getters
	int getNumCpu();
	string getSchedulerConfig();
	int getTimeSlice();
	int getBatchProcessFrequency();
	int getMinIns();
	int getMaxIns();
	int getDelayPerExec();
	size_t getMaxOverallMem();
	size_t getMemPerFrame();
	size_t getMinMemPerProc();
	size_t getMaxMemPerProc();

	//setters
	void setNumCpu(int num_cpu);
	void setSchedulerConfig(string schedulerConfig);
	void setTimeSlice(int timeSlice);
	void setBatchProcessFrequency(int batchProcessFrequency);
	void setMinIns(int minIns);
	void setMaxIns(int maxIns);
	void setDelayPerExec(int delayPerExec);
	void setMaxOverallMem(size_t maxOverallMem);
	void setMemPerFrame(size_t memPerFrame);
	void setMinMemPerProc(size_t minMemPerProc);
	void setMaxMemPerProc(size_t maxMemPerProc);

	//Process Screen
	std::shared_ptr<Process> getScreenByProcessName(const std::string& processName);
	std::unordered_map<string, std::shared_ptr<ProcessScreen>> getScreenMap();
	std::shared_ptr<ProcessScreen> getCurrentConsole();
	void registerConsole(std::shared_ptr<ProcessScreen> screenRef);
	void setCurrentConsole(std::shared_ptr<ProcessScreen> screenRef);
	void switchConsole(string consoleName);
	bool getInitialized();
	void setInitialized(bool initialized);

	// Process information
	string getCurrentTimestamp();
	int getCpuCycles();
	void getMemoryUsage();
	int getNumPages();
	void setNumPages();

	// Scheduler
	void schedulerTest();
	bool isRunning();
	
	//Printing
	void displayProcessList();
	void printProcess(string enteredProcess);\
	void printProcessSmi();
	void reportUtil();
	void printVmstat();

private:
	static ConsoleManager* consoleManager;
	string consoleName = "";
	bool running = true;
	bool switchSuccessful = true;
	bool initialized = false;
	string schedulerConfig = "";
	int num_cpu = 0;
	int timeSlice = 0;
	int minIns = 0;
	int maxIns = 0;
	int delayPerExec = 0;
	int cpuCycles = 0;
	int batchProcessFrequency = 0;
	size_t maxOverallMem = 0;
	size_t memPerFrame = 0;
	size_t minMemPerProc = 0;
	size_t maxMemPerProc = 0;
	size_t numPages = 0;
 
	Scheduler scheduler;

	std::shared_ptr<ProcessScreen> currentConsole;
	std::stringstream logStream;
	std::unordered_map<string, std::shared_ptr<ProcessScreen>> screenMap;
};