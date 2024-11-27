using namespace std;

#include "FlatMemoryAllocator.h"
#include "ConsoleManager.h"
#include <algorithm>
#include <fstream>


FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0)
{
	memory.resize(maximumSize);

	initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
	memory.clear();
}

FlatMemoryAllocator* FlatMemoryAllocator::flatMemoryAllocator = nullptr;

void FlatMemoryAllocator::initialize(size_t maximumMemorySize){
    flatMemoryAllocator = new FlatMemoryAllocator(maximumMemorySize);
}

FlatMemoryAllocator* FlatMemoryAllocator::getInstance() {
	return flatMemoryAllocator;
}

std::mutex allocationMapMutex;  // Mutex for protecting allocationMap

void* FlatMemoryAllocator::allocate(size_t size, string processName, std::shared_ptr<Process> process) {
	{
		std::lock_guard<std::mutex> lock(allocationMapMutex);  // Lock to ensure thread safety

		// Check for the availability of a suitable block
		for (size_t i = 0; i < maximumSize - size + 1; ++i) {
			// Check if the memory block is available
			if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
				if (canAllocateAt(i, size)) {
					// Ensure that the requested block doesn't go out of bounds
					if (i + size <= maximumSize) {
						allocateAt(i, size, processName);
						process->setMemoryUsage(FlatMemoryAllocator::getInstance()->getProcessMemoryUsage(processName));
						process->setIsRunning(true);
						return &memory[i];  // Return pointer to allocated memory
					}
				}
			}
		}

		//if (!processMemoryMap.empty()) {
		//	// Find the oldest process in the allocation map
		//	auto oldest = processMemoryMap.begin();
		//	std::string oldestProcessName = oldest->first;
		//	size_t oldestProcessMemorySize = oldest->second;

		//	// Find the corresponding Screen object
		//	std::shared_ptr<Screen> oldestProcess = ConsoleManager::getInstance()->getScreenByProcessName(oldestProcessName);

		//	if (oldestProcess) {
		//		
		//		// Move the process to the backing store
		//		backingStore.push(oldestProcess);

		//		// Deallocate the oldest process
		//		deallocateAt(findProcessStartIndex(oldestProcessName), oldestProcess);

		//		// Retry allocation
		//		for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		//			// Check if the memory block is available
		//			if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
		//				if (canAllocateAt(i, size)) {
		//					// Ensure that the requested block doesn't go out of bounds
		//					if (i + size <= maximumSize) {
		//						allocateAt(i, size, processName);
		//						process->setMemoryUsage(FlatMemoryAllocator::getInstance()->getProcessMemoryUsage(processName));
		//						process->setIsRunning(true);
		//						return &memory[i];  // Return pointer to allocated memory
		//					}
		//				}
		//			}
		//		}
		//	
		//	}
		//}

		//if (!backingStore.empty()) {
		//	// Attempt to restore a process from the backing store
		//	auto oldestProcess = backingStore.front();  // Get the oldest process from the backing store
		//	backingStore.pop();  // Remove it from the backing store

		//	// Find a free spot in memory
		//	for (size_t i = 0; i < maximumSize - oldestProcess->getMemoryRequired() + 1; ++i) {
		//		// Check if the memory block is available
		//		if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
		//			if (canAllocateAt(i, oldestProcess->getMemoryRequired())) {
		//				// Ensure that the requested block doesn't go out of bounds
		//				if (i + oldestProcess->getMemoryRequired() <= maximumSize) {
		//					allocateAt(i, oldestProcess->getMemoryRequired(), oldestProcess->getProcessName());
		//					oldestProcess->setMemoryUsage(FlatMemoryAllocator::getInstance()->getProcessMemoryUsage(oldestProcess->getProcessName()));
		//					oldestProcess->setIsRunning(true);
		//					return &memory[i];  // Return pointer to allocated memory
		//				}
		//			}
		//		}
		//	}
		//}
	}

	return nullptr;  // Return nullptr if allocation fails
}

size_t FlatMemoryAllocator::findProcessStartIndex(const std::string& processName) {
	// Iterate over the allocation map
	for (const auto& entry : allocationMap) {
		size_t index = entry.first;
		const std::string& name = entry.second;

		// Check if the current memory block belongs to the process
		if (name == processName) {
			return index;  // Return the starting index
		}
	}

	// If process not found, return a value indicating failure
	throw std::runtime_error("Process not found in memory allocation map.");
}



void FlatMemoryAllocator::deallocate(void* ptr, std::shared_ptr<Process> process) {
	std::lock_guard<std::mutex> lock(allocationMapMutex);
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index] != "") {
		deallocateAt(index, process);
		process->setIsRunning(false);
	}
}

std::string FlatMemoryAllocator::visualizeMemory()
{
	// Calculate the memory usage as a string
	std::string memoryUsage = std::to_string(allocatedSize) + " / " + std::to_string(maximumSize);

	// Return the memory usage
	return memoryUsage;
}


void FlatMemoryAllocator::visualizeMemoryASCII() {
	
	

}

size_t FlatMemoryAllocator::getProcessMemoryUsage(const std::string& processName) const {
	if (processMemoryMap.find(processName) != processMemoryMap.end()) {
		return processMemoryMap.at(processName);
	}
	return 0;  // Process not found
}

size_t FlatMemoryAllocator::getTotalMemoryUsage() const {
	size_t totalMemoryUsage = 0;

	// Iterate over the map and sum up the memory usage
	for (const auto& entry : processMemoryMap) {
		totalMemoryUsage += entry.second; // Add memory usage for each process
	}

	return totalMemoryUsage;
}


void FlatMemoryAllocator::initializeMemory() {
	// Initialize the memory vector with '.'
	memory.resize(maximumSize, '.');

	// Initialize the allocationMap with 'false' for all indices
	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = "";
	}
}



bool FlatMemoryAllocator:: canAllocateAt(size_t index, size_t size) {
	return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size, string processName) {
	// Fill allocation map with true values starting from index until the process size
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = processName; 
	}
	allocatedSize += size;
	processMemoryMap[processName] += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index, std::shared_ptr<Process> process){
	size_t size = ConsoleManager::getInstance()->getMinMemPerProc();
	for (size_t i = index; i < index + size && i < maximumSize; ++i) {
		allocationMap[i]  = "";
	}
	allocatedSize -= size;

	// Deduct from process memory usage
	if (processMemoryMap.find(process->getProcessName()) != processMemoryMap.end()) {
		processMemoryMap[process->getProcessName()] -= size;
		if (processMemoryMap[process->getProcessName()] == 0) {
			processMemoryMap.erase(process->getProcessName());  // Clean up zero usage
		}
	}
}

void FlatMemoryAllocator::restoreFromBackingStore() {
	if (!backingStore.empty()) {
		std::shared_ptr<Process> restoredProcess = backingStore.front();
		backingStore.pop();

		size_t memorySize = restoredProcess->getMemoryUsage();
		std::string processName = restoredProcess->getProcessName();

		// Attempt to reallocate memory for the process
		void* restoredMemory = allocate(memorySize, processName, restoredProcess);
		if (restoredMemory) {
			restoredProcess->setMemoryUsage(getProcessMemoryUsage(processName));
			restoredProcess->setIsRunning(true);
		}
		else {
			// Re-add to the backing store if reallocation fails
			backingStore.push(restoredProcess);
		}
	}
}

void FlatMemoryAllocator::visualizeBackingStore() {
	// Create a temporary copy of the queue for iteration
	std::queue<std::shared_ptr<Process>> tempQueue = backingStore;

	if (tempQueue.empty()) {
		std::cout << "Backing store is empty." << std::endl;
		cout << processMemoryMap.empty() << endl;
		return;
	}

	std::cout << "Backing Store Contents:" << std::endl;

	size_t index = 0; // Index to track the position of the process in the queue
	while (!tempQueue.empty()) {
		std::shared_ptr<Process> process = tempQueue.front();
		tempQueue.pop();

		// Access information from the Screen object
		std::cout << "Index: " << index++
			<< ", Process Name: " << process->getProcessName()
			<< ", Memory Usage: " << process->getMemoryUsage()
			<< " KB" << std::endl;
	}
}

void FlatMemoryAllocator::printMemoryInfo(int quantum_size) {
	static int curr_quantum_cycle = 0;  // Counter for unique file naming
	curr_quantum_cycle = curr_quantum_cycle + quantum_size;

	// Create a unique filename for each call
	std::string filename = "memory_stamp_" + std::to_string(curr_quantum_cycle) + ".txt";
	std::ofstream outFile(filename);  // Open a uniquely named file for writing

	if (!outFile) {  // Check if the file was successfully opened
		std::cerr << "Error opening file for writing.\n";
		return;
	}

	string timestamp = ConsoleManager::getInstance()->getCurrentTimestamp();
	size_t numProcessesInMemory = FlatMemoryAllocator::getInstance()->getNumberOfProcessesInMemory();

	// Print the information to the file
	outFile << "Timestamp: " << timestamp << "\n";
	outFile << "Number of processes in memory: " << numProcessesInMemory << "\n";
	outFile << "Total External fragmentation in KB: " << calculateExternalFragmentation() << "\n\n";
	outFile << "----end---- = " << maximumSize << "\n\n";

	std::string currentProcessName = "";
	std::string incomingProcessName = "";

	for (size_t i = maximumSize - 1; i > 0; i--) {
		incomingProcessName = allocationMap[i];
		if (currentProcessName.empty() && !incomingProcessName.empty()) {
			currentProcessName = incomingProcessName;
			outFile << i + 1 << "\n" << currentProcessName << "\n";
		}
		else if (currentProcessName != incomingProcessName) {
			outFile << i + 1 << "\n";
			currentProcessName = incomingProcessName;
			outFile << currentProcessName << "\n";
		}
		else if (incomingProcessName.empty() && i == maximumSize-1) {
			outFile << i + 1 << "\n";
		}
	}
	outFile << "----start---- = 0\n";

	outFile.close();  // Close the file
}



size_t FlatMemoryAllocator::calculateExternalFragmentation() {
	size_t externalFragmentation = 0;
	for (size_t i = 0; i < maximumSize; i += 1) {
		if (allocationMap[i] == "") {
			externalFragmentation++;
		}
	}
	return externalFragmentation;
}


size_t FlatMemoryAllocator::getNumberOfProcessesInMemory() {
	size_t allocatedBlocks = 0;

	{
		std::lock_guard<std::mutex> lock(allocationMapMutex);

		// Iterate through the allocation map and count allocated blocks
		for (size_t i = 0; i < allocationMap.size(); ++i) {
			//cout << "i: " << allocationMap[i] << endl;
			if (allocationMap.find(i) != allocationMap.end() && !allocationMap[i].empty()) {  // If the block is allocated
				allocatedBlocks++;
			}
		}
	}
		// Calculate the number of processes
		size_t processSizeInBlocks = ConsoleManager::getInstance()->getMinMemPerProc();  // Number of blocks per process
	return allocatedBlocks / processSizeInBlocks;  // Total number of processes in memory
}




