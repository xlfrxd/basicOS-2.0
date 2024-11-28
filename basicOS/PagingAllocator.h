#pragma once

#include <unordered_map>
#include <map>
#include <queue>

#include "Process.h"

class PagingAllocator
{
public:
	PagingAllocator(size_t maxMemorySize);

	static void initialize(size_t maximumMemorySize);

	static PagingAllocator* getInstance();

	bool allocate(std::shared_ptr<Process> process);
	void deallocate(std::shared_ptr<Process> process);
	void visualizeMemory();
	size_t calculateUsedFrames();
	size_t getProcessMemoryUsage(const std::string& processName);
	size_t getUsedMemory();
	void allocateFromBackingStore(std::shared_ptr<Process> process);
	void findAndRemoveProcessFromBackingStore(std::shared_ptr<Process> process);
	void setUsedMemory(size_t usedMemory);
	std::string findOldestProcess();
	bool isProcessInMemory(const std::string& processName);
	void visualizeBackingStore();

	size_t getNumPagedIn() const;
	size_t getNumPagedOut() const;

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

	std::vector<shared_ptr<Process>> backingStore;
	std::queue<shared_ptr<Process>> allocationMap;

	size_t numPagedIn = 0;  // Tracks the number of pages paged into memory
	size_t numPagedOut = 0; // Tracks the number of pages paged out of memory
};
