#pragma once
#include "Process.h"
#include <unordered_map>
#include "Scheduler.h"
#include <string>
#include <sstream>
#include <fstream>
#include "ConsoleColor.h"

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

	// default constructor
	ConsoleManager();

	// we can run this to create a console manager pointer
	static void initialize();

	// we get the console manager instance after 
	static ConsoleManager* getInstance();

	void printHeader();
	void drawConsole();
	void destroy();
	string getCurrentTimestamp();
	std::unordered_map<string, std::shared_ptr<ProcessScreen>> getScreenMap();

	void registerConsole(std::shared_ptr<ProcessScreen> screenRef);
	void switchConsole(string consoleName);
	std::shared_ptr<ProcessScreen> getCurrentConsole();
	void setInitialized(bool initialized);
	bool getInitialized();

	void setNumCpu(int num_cpu);
	void setSchedulerConfig(string schedulerConfig);
	void setTimeSlice(int timeSlice);
	void setBatchProcessFrequency(int batchProcessFrequency);
	void setMinIns(int minIns);
	void setMaxIns(int maxIns);
	void setDelayPerExec(int delayPerExec);
	void initializeConfiguration();
	void initializeAllocators();

	std::shared_ptr<Process> getScreenByProcessName(const std::string& processName);
	std::unordered_map<std::string, std::shared_ptr<Process>> processNameToScreenMap;

	void setMaxOverallMem(size_t maxOverallMem);
	void setMemPerFrame(size_t memPerFrame);
	void setMinMemPerProc(size_t minMemPerProc);
	void setMaxMemPerProc(size_t maxMemPerProc);

	int getNumCpu();
	string getSchedulerConfig();
	int getTimeSlice();
	int getBatchProcessFrequency();
	int getMinIns();
	int getMaxIns();
	int getDelayPerExec();
	void printProcessSmi(); 
	void printVmstat();
	size_t getUsedMemory();
	int getCpuCycles();
	size_t getMaxOverallMem();
	size_t getMemPerFrame();
	size_t getMinMemPerProc();
	size_t getMaxMemPerProc();

	void setNumPages();
	int getNumPages();

	void getMemoryUsage();
	void exitApplication();
	bool isRunning();
	void setCurrentConsole(std::shared_ptr<ProcessScreen> screenRef);
	void displayProcessList();
	void reportUtil();
	void printProcess(string enteredProcess);
	void schedulerTest();


private:
	// stores the console manager pointer so that we only instantiate once
	static ConsoleManager* consoleManager;
	string consoleName = "";
	bool running = true;
	bool switchSuccessful = true;
	bool initialized = false;
	int num_cpu = 0;
	string schedulerConfig = "";
	int timeSlice = 0;
	int batchProcessFrequency = 0;
	int minIns = 0;
	int maxIns = 0;
	int delayPerExec = 0;
	int cpuCycles = 0;
	size_t maxOverallMem = 0;
	size_t memPerFrame = 0;
	size_t minMemPerProc = 0;
	size_t maxMemPerProc = 0;
	int numPages = 0;


	// declare consoles 
	std::shared_ptr<ProcessScreen> currentConsole;
	std::stringstream logStream;
	std::unordered_map<string, std::shared_ptr<ProcessScreen>> screenMap;

	Scheduler scheduler;
};