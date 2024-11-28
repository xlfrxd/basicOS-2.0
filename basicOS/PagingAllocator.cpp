#include "PagingAllocator.h"
#include "ConsoleManager.h"
#include "ProcessScreen.h"

#include <map>
#include <vector> 
#include <string>
#include <memory>  
#include <algorithm>
#include <iostream> 
#include <set>

using namespace std;

PagingAllocator::PagingAllocator(size_t maxMemorySize) : maxMemorySize(maxMemorySize), numFrames(maxMemorySize)
{
	this->maxMemorySize = maxMemorySize;

	// Initialize free frame list
	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(i);
	}
}

PagingAllocator* PagingAllocator::pagingAllocator = nullptr;

void PagingAllocator::initialize(size_t maxMemorySize) {
	pagingAllocator = new PagingAllocator(maxMemorySize);
}

PagingAllocator* PagingAllocator::getInstance() {
	return pagingAllocator;
}

void* PagingAllocator::allocate(std::shared_ptr<Process> process) {
	string processId = process->getProcessName();
	size_t numFramesNeeded = process->getNumPages();
	/*cout << "FRAMES: " << numFramesNeeded << endl;
	cout << "SIZE: " << freeFrameList.size() << endl;*/

	for (auto it = frameMap.begin(); it != frameMap.end(); ++it) {
		std::cout << "Frame Index: " << it->first << " -> Process: " << it->second << "\n";
	}

	if (numFramesNeeded > freeFrameList.size()) {
		std::cerr << "Memory allocation failed. Not enough free frames.\n";
		return nullptr;
	}

	size_t frameIndex = allocateFrames(numFramesNeeded, processId);
	process->setMemoryUsage(PagingAllocator::getInstance()->getProcessMemoryUsage(process->getProcessName()));
	process->setIsRunning(true);
	processMemoryMap[process->getProcessName()] += process->getMemoryRequired();

	allocationMap.push(process);

	return reinterpret_cast<void*>(frameIndex);
}

void PagingAllocator::deallocate(std::shared_ptr<Process> process) {
	string processName = process->getProcessName();

	auto it = std::find_if(frameMap.begin(), frameMap.end(),
		[&processName](const std::pair<const size_t, std::string>& entry) {
			return entry.second == processName;
		});

	while (it != frameMap.end()) {
		size_t frameIndex = it->first;
		deallocateFrames(1, frameIndex);
		it = std::find_if(frameMap.begin(), frameMap.end(),
			[processName](const auto& entry) { return entry.second == processName; });
		// Deduct from process memory usage
		if (processMemoryMap.find(process->getProcessName()) != processMemoryMap.end()) {
			processMemoryMap[process->getProcessName()] -= process->getMemoryRequired();
			if (processMemoryMap[process->getProcessName()] == 0) {
				processMemoryMap.erase(process->getProcessName());  // Clean up zero usage
			}
		}


	}

	process->setIsRunning(false);

	/*cout << "PROCESS: " << process->getIsRunning() << endl;*/

	allocationMap.pop();
}

void* PagingAllocator::isProcessAllocated(const std::string& processName) {
	for (const auto& entry : frameMap) {
		if (entry.second == processName) {
			return reinterpret_cast<void*>(entry.first); // Found the process
		}
	}
	return nullptr; // Process not found
}



void PagingAllocator::visualizeMemory()
{
	size_t usedFrames = calculateUsedFrames();
	size_t totalMemory = maxMemorySize; // Maximum overall memory

	this->setUsedMemory(usedFrames * ConsoleManager::getInstance()->getMemPerFrame());


	// Display memory usage
	std::cout << "Memory Usage: " << this->usedMemory << " / " << totalMemory << " bytes" << std::endl;
}

size_t PagingAllocator::calculateUsedFrames() {
	size_t usedFrames = 0; // Counter for allocated frames

	// Iterate through frameMap to count allocated frames
	for (const auto& frame : frameMap)
	{
		if (!frame.second.empty()) // Check if the frame is in use
		{
			++usedFrames;
		}
	}

	return usedFrames;
}

size_t PagingAllocator::allocateFrames(size_t numFrames, string processName) {
	size_t frameIndex = freeFrameList.back();
	freeFrameList.pop_back();

	for (size_t i = 0; i < numFrames; ++i) {
		frameMap[frameIndex + i] = processName;
	}
	/*numPagedIn += numFrames;*/
	return frameIndex;
}

void PagingAllocator::deallocateFrames(size_t numFrames, size_t frameIndex) {
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap.erase(frameIndex + i);
	}

	for (size_t i = 0; i < numFrames; ++i) {
		freeFrameList.push_back(frameIndex + i);
	}
	/*numPagedOut += numFrames;*/
}

size_t PagingAllocator::getProcessMemoryUsage(const std::string& processName) {
	if (processMemoryMap.find(processName) != processMemoryMap.end()) {
		return processMemoryMap.at(processName);
	}
	return 0;  // Process not found
}

size_t PagingAllocator::getUsedMemory()
{
	return this->usedMemory;
}

void PagingAllocator::setUsedMemory(size_t usedMemory) {
	this->usedMemory = usedMemory;
}

std::string PagingAllocator::findOldestProcess() {
	//std::set<std::string> visitedProcesses; // To avoid duplicates
	//for (const auto& entry : frameMap) {
	//	const std::string& processName = entry.second;
	//	if (!processName.empty() && visitedProcesses.find(processName) == visitedProcesses.end()) {
	//		// If process name is not empty and hasn't been visited yet, return it
	//		visitedProcesses.insert(processName);
	//		return processName; // The first valid entry is the oldest
	//	}
	//}
	//return ""; // If no process is found
	std::shared_ptr<Process> oldest = allocationMap.front();

	//allocationMap.pop();

	return oldest->getProcessName();
}

void PagingAllocator::findAndRemoveProcessFromBackingStore(std::shared_ptr<Process> process) {
	bool found = false;

	for (int i = 0; i < backingStore.size(); i++) {
		//cout << "backing store process:" << backingStore[i]->getProcessName() << endl;
		//cout << "process:" << process->getProcessName() << endl;
		if (backingStore[i]->getProcessName() == process->getProcessName()) {
			// Remove the process from the backing store
			//cout << "Removing process " << process->getProcessName() << " from backing store." << endl;
			backingStore.erase(backingStore.begin() + i);
			break;
		}
	}

	//cout << backingStore.size() << endl;
}

void PagingAllocator::allocateFromBackingStore(std::shared_ptr<Process> process) {
	backingStore.push_back(process);
}

//size_t PagingAllocator::getNumPagedIn() const {
//	return numPagedIn;
//}
//
//size_t PagingAllocator::getNumPagedOut() const {
//	return numPagedOut;
//}