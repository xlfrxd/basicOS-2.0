#include "PagingAllocator.h"
#include "ConsoleManager.h"
#include "Process.h"

#include <map>
#include <vector> 
#include <string>
#include <memory>  
#include <algorithm>
#include <iostream> 

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
	if (numFramesNeeded > freeFrameList.size()) {
		std::cerr << "Memory allocation failed. Not enough free frames.\n";
		return nullptr;
	}

	size_t frameIndex = allocateFrames(numFramesNeeded, processId);
	process->setMemoryUsage(PagingAllocator::getInstance()->getProcessMemoryUsage(process->getProcessName()));
	process->setIsRunning(true);
	processMemoryMap[process->getProcessName()] += process->getMemoryRequired();
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
		process->setIsRunning(false);

	}
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

//size_t PagingAllocator::getNumPagedIn() const {
//	return numPagedIn;
//}
//
//size_t PagingAllocator::getNumPagedOut() const {
//	return numPagedOut;
//}