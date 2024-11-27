#include "Process.h"
#include <fstream> 
#include <filesystem> // for folder creation
#include "PrintCommand.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <random>
#include "ConsoleManager.h"

using namespace std;

namespace fs = std::filesystem; // alias for convenience

Process::Process(string processName, int currentLine, string timestamp, size_t memoryRequired)
    : ProcessScreen(processName), cpuCoreID(-1), commandCounter(0), currentState(ProcessState::READY)
{
    this->processName = processName;
    this->currentLine = currentLine;
	this->setRandomIns();
	this->memoryRequired = memoryRequired;
    this->timestamp = timestamp;

	this->numPages = ConsoleManager::getInstance()->getNumPages();

	// create 100 print commands
	for(int i=0; i<totalLine; i++){
		this->printCommands.push_back(PrintCommand(i, "Printing from " + processName + " " + std::to_string(i)));

	}

	
}

void Process::setMemoryUsage(size_t memoryUsage)
{
	this->memoryUsage = memoryUsage;
}

size_t Process::getMemoryUsage() const
{
	return this->memoryUsage;
}

void Process::setIsRunning(bool isRunning)
{
	this->isRunning = isRunning;
}

bool Process::getIsRunning() const
{
	return this->isRunning;
}

void Process::setRandomIns() {
	std::random_device rd;
	std::mt19937 gen(rd()); 
	std::uniform_int_distribution<> dis(ConsoleManager::getInstance()->getMinIns(), ConsoleManager::getInstance()->getMaxIns()); 
	this->totalLine = dis(gen);
}

Process::~Process()
{
}

void Process::setProcessName(string processName)
{
	this->processName = processName;
}

void Process::setCurrentLine(int currentLine)
{
	this->currentLine = currentLine;
}

void Process::setTotalLine(int totalLine)
{
	this->totalLine = totalLine;
}

void Process::setTimestamp(string timestamp)
{
	this->timestamp = timestamp;
}

string Process::getProcessName()
{
	return this->processName;
}

int Process::getCurrentLine()
{
	return this->currentLine;
}

int Process::getTotalLine()
{
	return this->totalLine;
}

string Process::getTimestamp()
{
	return this->timestamp;
}

string Process::getTimestampFinished() {
	return timestampFinished;
}

size_t Process::getNumPages()
{
	return this->numPages;
}

void Process::setNumPages(size_t numPages)
{
	this->numPages = numPages;
}

void Process::setTimestampFinished(string timestampFinished) {
	this->timestampFinished = timestampFinished;
}


void Process::executeCurrentCommand()
{
	this->printCommands[this->currentLine].execute();
}

void Process::moveToNextLine()
{
	this->currentLine++;
}

bool Process::isFinished() const
{
	return this->currentLine >= this->totalLine;
}

int Process::getCommandCounter() const
{
	return this->commandCounter;
}

int Process::getCPUCoreID() const
{
	return this->cpuCoreID;
}

Process::ProcessState Process::getState() const
{
	return this->currentState;
}

void Process::setCPUCoreID(int coreID)
{
	this->cpuCoreID = coreID;
}

void Process::createFile()
{
	string fileName = this->processName + ".txt";
	fstream file;
	//create file if it doesnt exit
	file.open(fileName, std::ios::out | std::ios::trunc);
	file << "Hello world from "<< this->processName << "!" << std::endl;
	file.close();
}

void Process::viewFile()
{
	string fileName = this->processName + ".txt";
	fstream file;
	file.open(fileName, std::ios::in);
	std::string line;
	while (std::getline(file, line)) {
		cout << "\"";
		cout << line;
		cout << "\"" << endl;
	}
	file.close();
}

size_t Process::getMemoryRequired() const
{
	return this->memoryRequired;
}

void Process::setMemoryRequired(size_t memoryRequired)
{
	this->memoryRequired = memoryRequired;
}