#pragma once
#include "Process.h"
#include <unordered_map>
#include <map>

class PagingAllocator
{
public:
	PagingAllocator(size_t maxMemorySize);

	static void initialize(size_t maximumMemorySize);

	static PagingAllocator* getInstance();

	void* allocate(std::shared_ptr<Process> process);
	void deallocate(std::shared_ptr<Process> process);
	void visualizeMemory();
	size_t calculateUsedFrames();
	size_t getProcessMemoryUsage(const std::string& processName);
	size_t getUsedMemory();
	void setUsedMemory(size_t usedMemory);

	//size_t getNumPagedIn() const;
	//size_t getNumPagedOut() const;

private:
	static PagingAllocator* pagingAllocator;
	std::map<size_t, std::string> frameMap; // Maps frame index to process name
	std::vector<size_t> freeFrameList;

	size_t maxMemorySize;
	size_t numFrames;
	size_t usedMemory = 0;

	size_t allocateFrames(size_t numFrames, string processName);
	void deallocateFrames(size_t numFrames, size_t frameIndex);
	std::unordered_map<std::string, size_t> processMemoryMap;

	//size_t numPagedIn = 0;  // Tracks the number of pages paged into memory
	//size_t numPagedOut = 0; // Tracks the number of pages paged out of memory
};

