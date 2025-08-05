#include "PagingAllocator.h"
#include "ConsoleManager.h"
#include "Screen.h"

#include <map>
#include <vector> 
#include <string>
#include <memory>  
#include <algorithm>
#include <iostream> 
#include <set>

using namespace std;

PagingAllocator::PagingAllocator(size_t maxMemorySize) : maxMemorySize(maxMemorySize), numFrames(ConsoleManager::getInstance()->getMaxOverallMem() / ConsoleManager::getInstance()->getMemPerFrame())
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


std::mutex allocationMap2Mutex;
std::mutex frameMapMutex;
std::mutex freeFrameListMutex;
std::mutex numPagedOutMutex;

bool PagingAllocator::allocate(std::shared_ptr<Screen> process) {
	{
		std::lock_guard<std::mutex> lock(allocationMap2Mutex);

		string processId = process->getProcessName();
		size_t numFramesNeeded = process->getNumPages();

		if (numFramesNeeded > freeFrameList.size()) {

			return false;
		}

		size_t frameIndex = allocateFrames(numFramesNeeded, processId);
		process->setMemoryUsage(process->getMemoryRequired());
		process->setIsRunning(true);
		processMemoryMap[process->getProcessName()] += process->getMemoryRequired();

		allocationMap.push(process);
		return true;
	}
}

void PagingAllocator::deallocate(std::shared_ptr<Screen> process) {
	std::string processName = process->getProcessName();

	// Lock the frame map to find all frames belonging to the process
	{
		std::lock_guard<std::mutex> lock(frameMapMutex);

		auto it = std::find_if(frameMap.begin(), frameMap.end(),
			[&processName](const std::pair<const size_t, std::string>& entry) {
				return entry.second == processName;
			});

		while (it != frameMap.end()) {
			size_t frameIndex = it->first;

			// Deallocate a single frame
			deallocateFrames(1, frameIndex);

			// Find the next frame for the same process
			it = std::find_if(frameMap.begin(), frameMap.end(),
				[&processName](const std::pair<const size_t, std::string>& entry) {
					return entry.second == processName;
				});
		}
	}

	// Update memory usage for the process
	{
		std::lock_guard<std::mutex> lock(frameMapMutex); // Lock required for safe access to processMemoryMap
		if (processMemoryMap.find(processName) != processMemoryMap.end()) {
			processMemoryMap[processName] -= process->getMemoryRequired();
			process->setMemoryUsage(0);

			if (processMemoryMap[processName] == 0) {
				processMemoryMap.erase(processName); // Remove entry if no memory remains allocated
			}
		}
	}

	// Remove process from allocation map
	{
		std::lock_guard<std::mutex> lock(allocationMap2Mutex);
		allocationMap.pop();
	}
}


bool PagingAllocator::isProcessInMemory(const std::string& processName) {
	for (const auto& frame : frameMap) {
		if (frame.second == processName) {
			return true; // Process is found in memory
		}
	}
	return false; // Process not found in memory
}



void PagingAllocator::visualizeBackingStore() {
	static int fileCounter = 0; // Persistent counter to increment file names

	// Create a file name with the current counter
	std::ostringstream fileName;
	fileName << "BackingStore_Visualization_" << fileCounter++ << ".txt";

	std::ofstream outFile(fileName.str()); // Open the file for writing
	if (!outFile) {
		std::cerr << "Failed to create the file: " << fileName.str() << std::endl;
		return;
	}

	if (backingStore.empty()) {
		outFile << "Backing store is empty." << std::endl;
		return;
	}

	// Write contents to the file
	outFile << "Backing Store Contents:" << std::endl;

	size_t index = 0; // Index to track the position of the process in the queue
	for (const auto& process : backingStore) {
		if (process) { // Ensure process is not nullptr
			outFile << "Index: " << index++ << " " << process->getProcessName() << std::endl;
		}
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

	for (size_t i = 0; i < numFrames; ++i) {
		frameMap[frameIndex + i] = processName;
		freeFrameList.pop_back();
	}
	numPagedIn += numFrames;
	return frameIndex;

}

std::mutex allocationMutex; // Replace frameMapMutex and freeFrameListMutex

void PagingAllocator::deallocateFrames(size_t numFrames, size_t frameIndex) {
	std::lock_guard<std::mutex> lock(allocationMutex); // Use shared mutex
	for (size_t i = 0; i < numFrames; ++i) {
		frameMap.erase(frameIndex + i);
		freeFrameList.push_back(frameIndex + i);
	}
	numPagedOut += numFrames;
}


size_t PagingAllocator::getProcessMemoryUsage(const std::string& processName) {
	std::cout << "Looking for process: " << processName << std::endl;
	for (const auto& entry : processMemoryMap) {
		std::cout << "Process: " << entry.first << ", Memory: " << entry.second << " KB" << std::endl;
	}
	if (processMemoryMap.find(processName) != processMemoryMap.end()) {
		return processMemoryMap.at(processName);
	}
	else {
		std::cout << "Process not found!" << std::endl;
	}
	return 0;

}

size_t PagingAllocator::getUsedMemory()
{
	return this->usedMemory;
}

void PagingAllocator::setUsedMemory(size_t usedMemory) {
	this->usedMemory = usedMemory;
}

std::string PagingAllocator::findOldestProcess() {
	std::shared_ptr<Screen> oldest;
	{
		std::lock_guard<std::mutex> lock(allocationMap2Mutex);
		oldest = allocationMap.front();
	}
	return oldest->getProcessName();

}

void PagingAllocator::findAndRemoveProcessFromBackingStore(std::shared_ptr<Screen> process) {
	bool found = false;

	for (int i = 0; i < backingStore.size(); i++) {
		if (backingStore[i]->getProcessName() == process->getProcessName()) {
			backingStore.erase(backingStore.begin() + i);
			break;
		}
	}
}

void PagingAllocator::allocateFromBackingStore(std::shared_ptr<Screen> process) {
	backingStore.push_back(process);
}

size_t PagingAllocator::getNumPagedIn() const {
	return numPagedIn;
}

void PagingAllocator::setNumPagedIn(size_t value) {
	this->numPagedIn = value;
}

size_t PagingAllocator::getNumPagedOut() const {
	return numPagedOut;
}

void PagingAllocator::setNumPagedOut(size_t value) {
	this->numPagedOut = value;
}