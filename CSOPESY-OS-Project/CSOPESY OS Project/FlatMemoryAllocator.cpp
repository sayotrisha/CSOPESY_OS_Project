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

void FlatMemoryAllocator::initialize(size_t maximumMemorySize) {
	flatMemoryAllocator = new FlatMemoryAllocator(maximumMemorySize);
}

FlatMemoryAllocator* FlatMemoryAllocator::getInstance() {
	return flatMemoryAllocator;
}

std::mutex allocationMapMutex;  // Mutex for protecting allocationMap

void* FlatMemoryAllocator::allocate(size_t size, string process) {
	{
		std::lock_guard<std::mutex> lock(allocationMapMutex);  // Lock to ensure thread safety

		// Check for the availability of a suitable block
		for (size_t i = 0; i < maximumSize - size + 1; ++i) {
			// Check if the memory block is available
			if (allocationMap.find(i) == allocationMap.end() || allocationMap[i].empty()) {
				if (canAllocateAt(i, size)) {
					// Ensure that the requested block doesn't go out of bounds
					if (i + size <= maximumSize) {
						allocateAt(i, size, process);
						return &memory[i];  // Return pointer to allocated memory
					}
				}
			}
		}
	}

	return nullptr;  // Return nullptr if allocation fails
}



void FlatMemoryAllocator::deallocate(void* ptr) {
	std::lock_guard<std::mutex> lock(allocationMapMutex);
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index] != "") {
		deallocateAt(index);
	}
}

std::string FlatMemoryAllocator::visualizeMemory() {
	return std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::visualizeMemoryASCII() {



}



void FlatMemoryAllocator::initializeMemory() {
	// Initialize the memory vector with '.'
	memory.resize(maximumSize, '.');

	// Initialize the allocationMap with 'false' for all indices
	for (size_t i = 0; i < maximumSize; ++i) {
		allocationMap[i] = "";
	}
}



bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) {
	return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size, string process) {
	// Fill allocation map with true values starting from index until the process size
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = process;
	}
	allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	size_t size = ConsoleManager::getInstance()->getMemPerProc();
	for (size_t i = index; i < index + size && i < maximumSize; ++i) {
		allocationMap[i] = "";
	}
	allocatedSize -= size;
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
		else if (incomingProcessName.empty() && i == maximumSize - 1) {
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
			if (allocationMap.find(i) != allocationMap.end() && !allocationMap[i].empty()) {  // If the block is allocated
				allocatedBlocks++;
			}
		}
	}
	// Calculate the number of processes
	size_t processSizeInBlocks = ConsoleManager::getInstance()->getMemPerProc();  // Number of blocks per process
	return allocatedBlocks / processSizeInBlocks;  // Total number of processes in memory
}
