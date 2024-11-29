#include <algorithm>
#include <fstream>

#include "FlatMemoryAllocator.h"
#include "ConsoleManager.h"

using namespace std;

// Constructor: initialize the memory vector with the maximum size
FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0)
{
	memory.resize(maximumSize);

	initializeMemory();
}

// Destructor: clear the memory vector
FlatMemoryAllocator::~FlatMemoryAllocator()
{
	memory.clear();
}

// Static instance of the FlatMemoryAllocator
FlatMemoryAllocator* FlatMemoryAllocator::flatMemoryAllocator = nullptr;

// Initialize the memory allocator with the maximum size
void FlatMemoryAllocator::initialize(size_t maximumMemorySize) {
	flatMemoryAllocator = new FlatMemoryAllocator(maximumMemorySize);
}

// Get the static instance of the FlatMemoryAllocator
FlatMemoryAllocator* FlatMemoryAllocator::getInstance() {
	return flatMemoryAllocator;
}

// Mutex for protecting allocationMap
std::mutex allocationMapMutex;

// Allocate memory for a process
void* FlatMemoryAllocator::allocate(size_t size, string processName, std::shared_ptr<Process> process) {
	{
		std::lock_guard<std::mutex> lock(allocationMapMutex);  // Ensure thread safety	

		// Check for the availability of a suitable block
		for (size_t i = 0; i < maximumSize - size + 1; ++i) {
			// Check if the memory block is available
			if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
				if (canAllocateAt(i, size)) {
					// Ensure that the requested block doesn't go out of bounds
					allocateAt(i, size, processName);
					process->setMemoryUsage(FlatMemoryAllocator::getInstance()->getProcessMemoryUsage(processName));
					process->setIsRunning(true);
					return &memory[i];  // Return pointer to allocated memory

				}
			}
		}

	}
	return nullptr;  // Return nullptr if allocation fails
}

// Add a process to the backing store
void FlatMemoryAllocator::allocateFromBackingStore(std::shared_ptr<Process> process) {
	backingStore.push_back(process);
}

// Find the oldest process in memory
std::shared_ptr<Process> FlatMemoryAllocator::findOldestProcess() {
	if (!allocationMap.empty()) {
		size_t oldestIndex = maximumSize; // Start with the highest possible index
		std::string oldestProcessName;

		// Iterate through the allocationMap to find the smallest key with a valid process name
		for (const auto& entry : allocationMap) {
			size_t index = entry.first;
			const std::string& processName = entry.second;

			if (!processName.empty() && index < oldestIndex) {
				oldestIndex = index;
				oldestProcessName = processName;
			}
		}
		// If a valid process was found, retrieve the corresponding Screen object
		if (!oldestProcessName.empty()) {
			return ConsoleManager::getInstance()->getScreenByProcessName(oldestProcessName);
		}
	}
	return nullptr; // Return nullptr if no process is found
}

// Find the starting index of a process in memory
size_t FlatMemoryAllocator::findProcessStartIndex(const std::string& processName) {
	for (const auto& entry : allocationMap) {
		size_t index = entry.first;
		const std::string& name = entry.second;

		if (name == processName) {
			return index;  // Return the starting index
		}
	}
	throw std::runtime_error("Process not found in memory allocation map."); // If process not found
}

// Deallocate memory for a process
void FlatMemoryAllocator::deallocate(void* ptr, std::shared_ptr<Process> process) {
	std::lock_guard<std::mutex> lock(allocationMapMutex);
	size_t index = static_cast<char*>(ptr) - &memory[0];

	if (allocationMap[index] != "") {
		deallocateAt(index, process);
	}
	process->setMemoryUsage(0);
}

// Visualize memory usage as a string
std::string FlatMemoryAllocator::visualizeMemory() {
	std::string memoryUsage = std::to_string(allocatedSize) + " / " + std::to_string(maximumSize);

	return memoryUsage;
}

// Get memory usage for a specific process
size_t FlatMemoryAllocator::getProcessMemoryUsage(const std::string& processName) const {
	if (processMemoryMap.find(processName) != processMemoryMap.end()) {
		return processMemoryMap.at(processName);
	}
	return 0;  // Process not found
}

// Get total memory usage
size_t FlatMemoryAllocator::getTotalMemoryUsage() const {
	size_t totalMemoryUsage = 0;

	for (const auto& entry : processMemoryMap) {
		totalMemoryUsage += entry.second; // Add memory usage for each process
	}
	return totalMemoryUsage;
}

// Initialize memory and allocation map
void FlatMemoryAllocator::initializeMemory() {
	memory.resize(maximumSize, '.'); // Initialize the memory vector with '.'

	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = ""; // Initialize the allocationMap with empty strings
	}
}

// Check if memory can be allocated at a specific index
bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) {
	return (index + size <= maximumSize);
}

// Allocate memory at a specific index
void FlatMemoryAllocator::allocateAt(size_t index, size_t size, string processName) {
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = processName; // Mark memory as allocated
	}
	allocatedSize += size;
	processMemoryMap[processName] += size;
}

// Deallocate memory at a specific index
void FlatMemoryAllocator::deallocateAt(size_t index, std::shared_ptr<Process> process) {
	size_t size = ConsoleManager::getInstance()->getMinMemPerProc();
	for (size_t i = index; i < index + size && i < maximumSize; ++i) {
		allocationMap[i] = ""; // Mark memory as deallocated
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

// Restore a process from the backing store
void FlatMemoryAllocator::restoreFromBackingStore() {
	if (!backingStore.empty()) {
		std::shared_ptr<Process> restoredProcess = backingStore.front();
		backingStore.pop_back();

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
			backingStore.push_back(restoredProcess);
		}
	}
}

// Visualize the contents of the backing store
void FlatMemoryAllocator::visualizeBackingStore() {
	if (backingStore.empty()) {
		std::cout << "Backing store is empty." << std::endl;
		return;
	}

	std::cout << "Backing Store Contents:" << std::endl;

	size_t index = 0; // Index to track the position of the process in the queue
	for (const auto& process : backingStore) {
		std::cout << "Index: " << index++
			<< ", Process Name: " << process->getProcessName()
			<< ", Memory Usage: " << process->getMemoryUsage()
			<< " KB" << std::endl;
	}
	std::cout << "\n" << std::endl;
}

// Print memory information to a file
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
		else if (incomingProcessName.empty() && i == maximumSize - 1) {
			outFile << i + 1 << "\n";
		}
	}
	outFile << "----start---- = 0\n";

	outFile.close();
}

// Calculate external fragmentation
size_t FlatMemoryAllocator::calculateExternalFragmentation() {
	size_t externalFragmentation = 0;
	for (size_t i = 0; i < maximumSize; i += 1) {
		if (allocationMap[i] == "") {
			externalFragmentation++;
		}
	}
	return externalFragmentation;
}

// Get the number of processes in memory
size_t FlatMemoryAllocator::getNumberOfProcessesInMemory() {
	size_t allocatedBlocks = 0;
	{
		std::lock_guard<std::mutex> lock(allocationMapMutex);

		// Iterate through the allocation map and count allocated blocks
		for (size_t i = 0; i < allocationMap.size(); ++i) {
			if (allocationMap.find(i) != allocationMap.end() && !allocationMap[i].empty()) {
				allocatedBlocks++;
			}
		}
	}
	size_t processSizeInBlocks = ConsoleManager::getInstance()->getMinMemPerProc();  // Number of blocks per process
	return allocatedBlocks / processSizeInBlocks;  // Total number of processes in memory
}

// Get a pointer to the memory block of a specific process
void* FlatMemoryAllocator::getMemoryPtr(size_t size, string processName, std::shared_ptr<Process> process) {
	std::lock_guard<std::mutex> lock(allocationMapMutex);  // Ensure thread safety

	// Iterate through the memory to find the block allocated to the process
	for (size_t i = 0; i < maximumSize - size + 1; ++i) {
		if (processName == allocationMap[i]) {
			return &memory[i];  // Return pointer to the memory block
		}
	}

	// Return nullptr if the process is not found
	return nullptr;
}

// Find and remove a process from the backing store
void FlatMemoryAllocator::findAndRemoveProcessFromBackingStore(std::shared_ptr<Process> process) {
	// Iterate through the backing store to find the process
	for (size_t i = 0; i < backingStore.size(); ++i) {
		if (backingStore[i]->getProcessName() == process->getProcessName()) {
			// Remove the process from the backing store
			backingStore.erase(backingStore.begin() + i);
			break;
		}
	}
}

// Get the size of allocated memory
size_t FlatMemoryAllocator::getAllocatedSize() {
	return allocatedSize;
}

// Get the current allocation map
std::unordered_map<size_t, string> FlatMemoryAllocator::getAllocationMap() {
	return allocationMap;
}
