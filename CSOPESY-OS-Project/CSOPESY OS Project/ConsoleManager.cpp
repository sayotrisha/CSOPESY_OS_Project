/**
 *  ConsoleManager.cpp
 *
 *  Purpose:
 *      This file implements the ConsoleManager class, which acts as the central
 *      controller for managing different console screens and coordinating process
 *      scheduling. It handles screen registration, switching between views,
 *      drawing output to the console, and interacting with the Scheduler.
 *
 *      ConsoleManager also parses configuration settings from a file,
 *      initializes system parameters (e.g., number of CPUs, delay per instruction),
 *      tracks process states, and generates runtime reports.
 *
 *      It serves as the core hub that bridges user input, process management,
 *      and visual output in the simulation.
 */
using namespace std;

#include "ConsoleManager.h"
#include <iostream>
#include <unordered_map>
#include "Screen.h"
#include "Colors.h"
#include <random>

/*------------------------------------------------- ConsoleManager (Constructor) -----
 |  Function ConsoleManager()
 |
 |  Purpose:  Default constructor for the ConsoleManager class.
 |      Prepares an instance of the manager, though initialization is completed
 |      through the static initialize() method.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 |
 |  Comments: stores the created instance of console manager
 *-------------------------------------------------------------------*/
ConsoleManager* ConsoleManager::consoleManager = nullptr;

/*---------------------------------------------------------------------
 |  Function ConsoleManager()
 |
 |  Purpose:  Default constructor for the ConsoleManager class.
 |      Initializes an instance of ConsoleManager. This constructor is
 |      called internally via the initialize() method to set up the
 |      singleton instance.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing (constructor)
 *-------------------------------------------------------------------*/
ConsoleManager::ConsoleManager() {
};

/*--------------------------------------------------------------------
 |
 |  Purpose:  Initializes the ConsoleManager singleton by creating a new instance
 |      and storing it in a static pointer. This sets up the manager for use
 |      across the application.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::initialize() {
    consoleManager = new ConsoleManager();
}

/*----------------------------------------------------------------------
 |  Function initializeConfiguration()
 |
 |  Purpose:  Reads settings from a configuration file (config.txt) and applies them
 |      to the ConsoleManager (e.g., number of CPUs, scheduler type, delay values).
 |      Also initializes the Scheduler instance.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::initializeConfiguration() {
    FILE* file;
    errno_t err = fopen_s(&file, "config.txt", "r");
    if (err != 0) {
        cout << "Error opening file" << endl;
    }

    else {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            string str = line;
            size_t space_pos = str.find(" ");

            if (space_pos == string::npos) continue; // Skip malformed lines

            string key = str.substr(0, space_pos);
            string value = str.substr(space_pos + 1);
            value.erase(remove(value.begin(), value.end(), '\n'), value.end()); // Remove newline

            if (key == "num-cpu") {
                ConsoleManager::getInstance()->setNumCpu(stoi(value));
            }
            else if (key == "scheduler") {
                value.erase(remove(value.begin(), value.end(), '\"'), value.end()); // Remove quotes
                ConsoleManager::getInstance()->setSchedulerConfig(value);
            }
            else if (key == "quantum-cycles") {
                ConsoleManager::getInstance()->setTimeSlice(stoi(value));
            }
            else if (key == "min-ins") {
                ConsoleManager::getInstance()->setMinIns(stoi(value));
            }
            else if (key == "max-ins") {
                ConsoleManager::getInstance()->setMaxIns(stoi(value));
            }
            else if (key == "delay-per-exec") {
                ConsoleManager::getInstance()->setDelayPerExec(stoi(value));
            }
            else if (key == "batch-process-freq") {
                ConsoleManager::getInstance()->setBatchProcessFrequency(stoi(value));
            }
            else if (key == "max-overall-mem") {
                ConsoleManager::getInstance()->setMaxOverallMem(stoi(value));
            }
            else if (key == "mem-per-frame") {
                ConsoleManager::getInstance()->setMemPerFrame(stoi(value));
            }
            else if (key == "mem-per-proc") {
                ConsoleManager::getInstance()->setMemPerProc(stoi(value));
            }
        }
        fclose(file);
    }


	Scheduler* scheduler = Scheduler::getInstance();
}

/*--------------------------------------------------------------------
 |  Function schedulerTest()
 |
 |  Purpose:  Repeatedly generates dummy process screens in batches and registers
 |      them to the scheduler queue. This simulates dynamic CPU usage and
 |      supports stress testing of the scheduling system.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::schedulerTest() {
    static int process_counter = 0;
    process_counter++;
    while (Scheduler::getInstance()->getSchedulerTestRunning()) {
        for (int i = 0; i < ConsoleManager::getInstance()->getBatchProcessFrequency(); i++) {
            string processName = "P" + std::to_string(ConsoleManager::getInstance()->cpuCycles);
            //string processName = "cycle" + std::to_string(ConsoleManager::getInstance()->cpuCycles) + "processName" + std::to_string(i);
            shared_ptr<BaseScreen> processScreen = make_shared<Screen>(processName, 0, ConsoleManager::getInstance()->getCurrentTimestamp(), ConsoleManager::getInstance()->getMemPerProc());
            shared_ptr<Screen> screenPtr = static_pointer_cast<Screen>(processScreen);
            Scheduler::getInstance()->addProcessToQueue(screenPtr);
            ConsoleManager::getInstance()->registerConsole(processScreen);
            ConsoleManager::getInstance()->cpuCycles++;
            ConsoleManager::getInstance()->printProcess(processName);
            
        }
        process_counter++;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

/*--------------------------------------------------------------------
 |  Function getCpuCycles()
 |
 |  Purpose:  Returns the number of CPU cycles completed since initialization.
 |      Typically used to name processes sequentially (e.g., process0, process1...).
 |
 |  Parameters: None
 |
 |  Returns:  int -- the number of completed CPU cycles.
 *-------------------------------------------------------------------*/
int ConsoleManager::getCpuCycles() {
	return this->cpuCycles;
}

/*--------------------------------------------------------------------
 |  Function drawConsole()
 |
 |  Purpose:  Clears the screen and redraws either the main screen or a process screen,
 |      depending on what is currently selected. Displays instruction progress
 |      and timestamp for process screens.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::drawConsole() {
    if (this->switchSuccessful) {
        system("cls");
        string consoleName = this->getCurrentConsole()->getConsoleName();

        if (this->getCurrentConsole()->getConsoleName() == MAIN_CONSOLE) {
            this->printHeader();
        }
        else {
            if (this->screenMap.contains(consoleName)) {
                cout << "Screen Name: " << this->screenMap[consoleName]->getConsoleName() << endl;
                cout << "Current line of instruction / Total line of instruction: ";
                cout << this->screenMap[consoleName]->getCurrentLine();
                cout << "/" << this->screenMap[consoleName]->getTotalLine() << endl;
                cout << "Timestamp: " << this->screenMap[consoleName]->getTimestamp() << endl;
            }
        }
    }
}

/*--------------------------------------------------------------------
 |  Function destroy()
 |
 |  Purpose:  Cleans up the singleton instance and stops the scheduler.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::destroy() {
    consoleManager->scheduler.stop(); // Stop the scheduler
    delete consoleManager;
}

/*------------------------------------------------- getCurrentTimestamp -----
 |  Function getCurrentTimestamp()
 |
 |  Purpose:  Retrieves the current date and time formatted as a string.
 |
 |  Parameters: None
 |
 |  Returns:  string -- current system time in MM/DD/YYYY, HH:MM:SS AM/PM format
 *-------------------------------------------------------------------*/
string ConsoleManager::getCurrentTimestamp() {
    // Get current time as time_t object
    time_t currentTime = time(nullptr);
    // Create tm structure to store local time
    tm localTime;
    // Convert time_t to tm structure 
    localtime_s(&localTime, &currentTime);
    // Create a buffer to store the formatted time
    char timeBuffer[100];
    // Format the time (MM/DD/YYYY, HH:MM:SS AM/PM)
    strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y, %I:%M:%S %p", &localTime);
    return timeBuffer;
}

/*--------------------------------------------------------------------
 |  Function registerConsole(screenRef)
 |
 |  Purpose:  Registers a new BaseScreen object into the screen map for tracking and
 |      interaction. Each screen is uniquely identified by its name.
 |
 |  Parameters:
 |      screenRef (IN) -- shared_ptr to the BaseScreen object to register.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::registerConsole(shared_ptr<BaseScreen> screenRef) {
    this->screenMap[screenRef->getConsoleName()] = screenRef; //it should accept MainScreen and ProcessScreen
    //system("cls");
}


/*---------------------------------------------------------------------
 |  Function switchConsole(consoleName)
 |
 |  Purpose:  Changes the current active screen based on the provided name, and redraws
 |      it on the console. If the screen is not found, an error message is shown.
 |
 |  Parameters:
 |      consoleName (IN) -- name of the screen to switch to.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::switchConsole(string consoleName)
{
    if (this->screenMap.contains(consoleName)) {
        this->currentConsole = this->screenMap[consoleName];
		this->consoleName = consoleName;

        if (consoleName == MAIN_CONSOLE) {
            this->drawConsole();
        }
    
        this->switchSuccessful = true;
    }
    else {
        cout << RED << "Console name " << consoleName << " not found. Was it initialized?" << RESET << endl;
        this->switchSuccessful = false;
    }
}

/*-------------------------------------------------------------------
 |  Function displayProcessList()
 |
 |  Purpose:  Lists running and completed processes, along with CPU core assignments,
 |      instruction progress, and timestamps. Useful for monitoring system load.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/

void ConsoleManager::displayProcessList() {
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = (float)coresUsed / (coresUsed + coresAvailable) * 100;

    cout << "\nCPU Utilization: " << cpuUtilization << "%" << endl;
    cout << "Cores used: " << coresUsed << endl;
    cout << "Cores available: " << coresAvailable << endl;
    cout << BLUE << "-----------------------------------" << RESET << endl;
    cout << "Running processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);

        if (screenPtr && !screenPtr->isFinished()) {

            auto coreID = screenPtr->getCPUCoreID();
            string coreIDstr;
            if (coreID == -1) {
                coreIDstr = "N/A";
            }
            else {
                coreIDstr = to_string(coreID);
            }

            cout << "Name: " << screenPtr->getProcessName() << BLUE << " (" << RESET << YELLOW
                << screenPtr->getTimestamp() << RESET << BLUE << ") " << RESET 
                << "Core" << BLUE << ": "<< RESET << YELLOW << coreIDstr <<  "   "
                << screenPtr->getCurrentLine() << RESET << BLUE << "/" 
                << YELLOW << screenPtr->getTotalLine() << "   " << RESET << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);


        if (screenPtr && screenPtr->isFinished()) {
            cout << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestampFinished() << " | "
                << "Finished" << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << endl;
        }
    }
    cout << BLUE << "-----------------------------------" << RESET << endl;
}

/*-------------------------------------------------------------------
 |  Function reportUtil()
 |
 |  Purpose:  Logs a CPU utilization report and active process info to a file named
 |      "csopesy-log.txt". Helps analyze performance after test runs.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::reportUtil() {
    std::ostringstream logStream;
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    Scheduler* scheduler = Scheduler::getInstance();
    int coresUsed = scheduler->getCoresUsed();
    int coresAvailable = scheduler->getCoresAvailable();
    float cpuUtilization = static_cast<float>(coresUsed) / (coresUsed + coresAvailable) * 100;

    // Log CPU utilization and core details
    logStream << "\nCPU Utilization: " << cpuUtilization << "%" << std::endl;
    logStream << "Cores used: " << coresUsed << std::endl;
    logStream << "Cores available: " << coresAvailable << std::endl;
    logStream << "-----------------------------------" << std::endl;
    logStream << "Running processes:" << std::endl;

    // Log details of running processes
    for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Screen>(pair.second);
        if (screenPtr && !screenPtr->isFinished()) {
            auto coreID = screenPtr->getCPUCoreID();
            std::string coreIDstr = (coreID == -1) ? "N/A" : std::to_string(coreID);

            logStream << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestamp() << " | "
                << "Core: " << coreIDstr << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << std::endl;
        }
    }

    logStream << "\nFinished processes:" << std::endl;

    // Log details of finished processes
    for (const auto& pair : screenMap) {
        auto screenPtr = std::dynamic_pointer_cast<Screen>(pair.second);
        if (screenPtr && screenPtr->isFinished()) {
            logStream << "Name: " << screenPtr->getProcessName() << " | "
                << screenPtr->getTimestampFinished() << " | "
                << "Finished" << " | "
                << screenPtr->getCurrentLine() << "/"
                << screenPtr->getTotalLine() << " | " << std::endl;
        }
    }

    logStream << "-----------------------------------" << std::endl;

    // Write the log data to a file
    std::ofstream file("text_files/csopesy-log.txt", std::ios::out);
    if (file.is_open()) {
        file << logStream.str(); // Write log contents to file
        file.close();
        std::cout << CYAN << "Report generated at text_files/csopesy-log.txt" << RESET << std::endl;
    }
    else {
        std::cerr << RED << "Error: Could not open file for writing." << RESET << std::endl;
    }
}

/*-------------------------------------------------------------------
 |  Function getNumCpu()
 |
 |  Purpose:  Retrieves the number of CPU cores configured in the system.
 |
 |  Parameters: None
 |
 |  Returns:  int -- the number of CPU cores set during configuration.
 *-------------------------------------------------------------------*/
int ConsoleManager::getNumCpu() {
	return this->num_cpu;
}

/*--------------------------------------------------------------------
 |  Function getSchedulerConfig()
 |
 |  Purpose:  Retrieves the scheduler algorithm type (e.g., "fcfs", "rr").
 |
 |  Parameters: None
 |
 |  Returns:  string -- name of the scheduler algorithm.
 *-------------------------------------------------------------------*/
string ConsoleManager::getSchedulerConfig() {
	return this->schedulerConfig;
}

/*---------------------------------------------------------------------
 |  Function getTimeSlice()
 |
 |  Purpose:  Returns the time quantum value used by the Round-Robin scheduler.
 |
 |  Parameters: None
 |
 |  Returns:  int -- time slice duration in cycles.
 *-------------------------------------------------------------------*/
int ConsoleManager::getTimeSlice() {
	return this->timeSlice;
}

/*---------------------------------------------------------------------
 |  Function getBatchProcessFrequency()
 |
 |  Purpose:  Returns how frequently new process batches are generated during scheduler testing.
 |
 |  Parameters: None
 |
 |  Returns:  int -- number of processes to spawn per cycle.
 *-------------------------------------------------------------------*/
int ConsoleManager::getBatchProcessFrequency() {
	return this->batchProcessFrequency;
}

/*--------------------------------------------------------------------
 |  Function getMinIns()
 |
 |  Purpose:  Retrieves the configured minimum number of instructions per process.
 |
 |  Parameters: None
 |
 |  Returns:  int -- minimum instruction count.
 *-------------------------------------------------------------------*/
int ConsoleManager::getMinIns() {
	return this->minIns;
}

/*-------------------------------------------------------------------
 |  Function getMaxIns()
 |
 |  Purpose:  Retrieves the configured maximum number of instructions per process.
 |
 |  Parameters: None
 |
 |  Returns:  int -- maximum instruction count.
 *-------------------------------------------------------------------*/
int ConsoleManager::getMaxIns() {
	return this->maxIns;
}

/*------------------------------------------------------------------
 |  Function getDelayPerExec()
 |
 |  Purpose:  Returns the configured artificial delay (in 100ms units) applied
 |      to each instruction execution for simulation pacing.
 |
 |  Parameters: None
 |
 |  Returns:  int -- delay value in multiplier units of 100ms.
 *-------------------------------------------------------------------*/
int ConsoleManager::getDelayPerExec() {
	return this->delayPerExec;
}

/*-------------------------------------------------------------------
 |  Function setNumCpu(int num_cpu)
 |
 |  Purpose:  Sets the number of CPU cores available to the scheduler.
 |
 |  Parameters:
 |      num_cpu (IN) -- number of cores to be simulated.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setNumCpu(int num_cpu) {
	this->num_cpu = num_cpu;
}

/*--------------------------------------------------------------------
 |  Function setSchedulerConfig(string scheduler)
 |
 |  Purpose:  Sets the scheduler algorithm to be used.
 |
 |  Parameters:
 |      scheduler (IN) -- scheduler type as a string (e.g., "fcfs", "rr").
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setSchedulerConfig(string scheduler) {
	this->schedulerConfig = scheduler;
}

/*--------------------------------------------------------------------
 |  Function setTimeSlice(int timeSlice)
 |
 |  Purpose:  Sets the quantum time used in Round-Robin scheduling.
 |
 |  Parameters:
 |      timeSlice (IN) -- number of instructions per quantum.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setTimeSlice(int timeSlice) {
	this->timeSlice = timeSlice;
}

/*---------------------------------------------------------------------
 |  Function setBatchProcessFrequency(int batchProcessFrequency)
 |
 |  Purpose:  Sets how many processes to spawn per cycle during scheduler testing.
 |
 |  Parameters:
 |      batchProcessFrequency (IN) -- number of new processes per interval.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setBatchProcessFrequency(int batchProcessFrequency) {
	this->batchProcessFrequency = batchProcessFrequency;
}

/*---------------------------------------------------------------------
 |  Function setMinIns(int minIns)
 |
 |  Purpose:  Sets the minimum number of instructions per process.
 |
 |  Parameters:
 |      minIns (IN) -- minimum number of lines/instructions.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setMinIns(int minIns) {
	this->minIns = minIns;
}

/*---------------------------------------------------------------------
 |  Function setMaxIns(int maxIns)
 |
 |  Purpose:  Sets the maximum number of instructions per process.
 |
 |  Parameters:
 |      maxIns (IN) -- maximum number of lines/instructions.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setMaxIns(int maxIns) {
	this->maxIns = maxIns;
}

/*----------------------------------------------------------------------
 |
 |  Purpose:  Sets the artificial delay between instruction executions (for pacing).
 |
 |  Parameters:
 |      delayPerExec (IN) -- number of 100ms units to delay each instruction.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/

void ConsoleManager::setDelayPerExec(int delayPerExec) {
	this->delayPerExec = delayPerExec;
}

/*-------------------------------------------------------------------
 |  Function printProcess(enteredProcess)
 |
 |  Purpose:  Prints logs of a specific finished process and displays its output file
 |      contents if available.
 |
 |  Parameters:
 |      enteredProcess (IN) -- name of the process to view.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::printProcess(string enteredProcess){
    unordered_map<string, shared_ptr<BaseScreen>> screenMap = ConsoleManager::getInstance()->getScreenMap();
    auto it = screenMap.find(enteredProcess);

    if (it == screenMap.end()) {
        cout << RED << "Process: '" << enteredProcess << "' not found." << RESET << endl;
        return;
    }

    shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(it->second);
    if (!screenPtr) {
        cout << RED << "Screen '" << enteredProcess << "' is not a process screen." << RESET << endl;
        return;
    }
    if (!screenPtr->isFinished()) {
        //cout << RED << "Process is not yet finished." << RESET << endl;
        return;
    }
    for (const auto& pair : screenMap) {
        shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(pair.second);
        if (!screenPtr) continue;

        //check if process name exits
        if (screenPtr->getProcessName() == enteredProcess) {

            //check if process is finished
            if (screenPtr && screenPtr->isFinished()){
                shared_ptr<Screen> screenPtr = dynamic_pointer_cast<Screen>(screenMap.find(enteredProcess)->second);

                auto coreID = screenPtr->getCPUCoreID();
                string coreIDstr;
                if (coreID == -1) {
                    coreIDstr = "N/A";
                }
                else {
                    coreIDstr = to_string(coreID);
                }

                cout << BLUE << "Process Name: " << enteredProcess << endl;
                cout << "Logs:" << endl;
                cout << "(" << screenPtr->getTimestamp() << ")  "
                    << "Core: " << coreIDstr << "  " << RESET;
                //where to put createfile?
                screenPtr->createFile();
                screenPtr->viewFile();
            }
            else {
                cout << RED << "Process is not yet finished" << RESET << endl;
            }
            
        }

    }
}

/*--------------------------------------------------------------------
 |  Function printProcessSmi()
 |
 |  Purpose:  Displays current line and total lines of the active process screen.
 |      If the process is finished, it shows a "Finished!" message.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::printProcessSmi() {
    cout << "Process: " << this->consoleName << endl;
    if (this->screenMap[consoleName]->getCurrentLine() == this->screenMap[consoleName]->getTotalLine()) {
        cout << GREEN << "Finished!" << RESET << endl;
    }
    else {
        cout << YELLOW << "Current Line: " << this->screenMap[consoleName]->getCurrentLine() << endl;
        cout << "Lines of Code: " << this->screenMap[consoleName]->getTotalLine() << RESET << endl;
    }

    // ??? New logic: generate alternating PRINT/ADD instructions ???
    auto proc = this->screenMap[consoleName];
    int instrCount = proc->getTotalLine();                     // total instructions per process
    std::random_device rd;                                     // seed for RNG
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);               // offsets between 1 and 10

    bool nextIsPrint = true;
    for (int i = 0; i < instrCount; ++i) {
        int x = proc->getCurrentLine();                        // or use i if you prefer
        if (nextIsPrint) {
            //cout << "PRINT(\"Value from: \" + std::to_string(" << x << "))" << endl;
            cout << "Print(\"Value from: \" +" << x << ")" << endl;
        }
        else {
            int offset = dist(gen);
            cout << "ADD(" << x << ", " << x << ", " << offset << ")" << endl;
        }
        nextIsPrint = !nextIsPrint;
    }
}


/*--------------------------------------------------------------------
 |  Function getCurrentConsole()
 |
 |  Purpose:  Retrieves the currently active screen.
 |
 |  Parameters: None
 |
 |  Returns:  shared_ptr<BaseScreen> -- the screen currently being displayed.
 *-------------------------------------------------------------------*/
shared_ptr<BaseScreen> ConsoleManager::getCurrentConsole()
{
    return this->currentConsole;
}

/*--------------------------------------------------------------------
 |  Function setCurrentConsole(screenRef)
 |
 |  Purpose:  Sets the currently active screen object.
 |
 |  Parameters:
 |      screenRef (IN) -- pointer to the screen to activate.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/

void ConsoleManager::setCurrentConsole(shared_ptr<BaseScreen> screenRef)
{
    this->currentConsole = screenRef;
}

/*---------------------------------------------------------------------
 |  Function getInstance()
 |
 |  Purpose:  Provides access to the singleton instance of ConsoleManager.
 |
 |  Parameters: None
 |
 |  Returns:  ConsoleManager* -- pointer to the global ConsoleManager instance.
 *-------------------------------------------------------------------*/
ConsoleManager* ConsoleManager::getInstance()
{
    return consoleManager;
}

/*-------------------------------------------------------------------
 |  Function exitApplication()
 |
 |  Purpose:  Gracefully stops the scheduler and ends the program loop by marking
 |      the running flag as false.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::exitApplication() {
    this->running = false;
	Scheduler::getInstance()->stop();
}

/*---------------------------------------------------------------------
 |  Function isRunning()
 |
 |  Purpose:  Checks if the application is currently running.
 |
 |  Parameters: None
 |
 |  Returns:  bool -- true if the app should continue running; false otherwise.
 *-------------------------------------------------------------------*/
bool ConsoleManager::isRunning() {
    return this->running;
}

/*---------------------------------------------------------------------
 |  Function getScreenMap()
 |
 |  Purpose:  Retrieves the internal map of all registered screens.
 |
 |  Parameters: None
 |
 |  Returns:  unordered_map<string, shared_ptr<BaseScreen>> -- all registered screens.
 *-------------------------------------------------------------------*/
unordered_map<string, shared_ptr<BaseScreen>> ConsoleManager::getScreenMap() {
    return this->screenMap;
}

/*---------------------------------------------------------------------
 |  Function setInitialized(bool initialized)
 |
 |  Purpose:  Marks whether the processor configuration has been initialized.
 |
 |  Parameters:
 |      initialized (IN) -- true if setup is done, false otherwise.
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::setInitialized(bool initialized) {
	this->initialized = initialized;
}

/*---------------------------------------------------------------------
 |  Function getInitialized()
 |
 |  Purpose:  Checks if the configuration setup is complete.
 |
 |  Parameters: None
 |
 |  Returns:  bool -- true if initialized, false if not.
 *-------------------------------------------------------------------*/
bool ConsoleManager::getInitialized() {
	return this->initialized;
}

void ConsoleManager::setMaxOverallMem(size_t maxOverallMem) {
    this->maxOverallMem = maxOverallMem;
}

void ConsoleManager::setMemPerFrame(size_t memPerFrame) {
    this->memPerFrame = memPerFrame;
}

void ConsoleManager::setMemPerProc(size_t memPerProc) {
    this->memPerProc = memPerProc;
}

size_t ConsoleManager::getMaxOverallMem() {
    return this->maxOverallMem;
}

size_t ConsoleManager::getMemPerFrame() {
    return this->memPerFrame;
}

size_t ConsoleManager::getMemPerProc() {
    return this->memPerProc;
}

/*---------------------------------------------------------------------
 |  Function printHeader()
 |
 |  Purpose:  Prints a stylized banner header for the main screen view.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::printHeader() {
    cout << PASTEL_PINK << "________________________________________________________________________________\n";
    cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n";
    cout << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .-'\\  `.'  /  \n";
    cout << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n";
    cout << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n";
    cout << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'     \n";
    cout << "________________________________________________________________________________\n" << RESET;
    cout << "\n";
}

/*---------------------------------------------------------------------
 |  Function printMarquee()
 |
 |  Purpose:  Prints a marquee welcome banner and commands for the main screen view.
 |
 |  Parameters: None
 |
 |  Returns:  Nothing
 *-------------------------------------------------------------------*/
void ConsoleManager::printMarquee() {
    std::string text = "Welcome to our Command Line Emulator!!! ";
    int len = static_cast<int>(text.size());

    // Marquee effect: one full cycle
    for (int offset = 0; offset < len; ++offset) {
        // Move cursor to beginning of line
        cout << "\r";

        // Print substring starting at offset
        cout << PASTEL_PINK <<text.substr(offset);

        // Then print the leading part to complete the loop
        cout << text.substr(0, offset) << RESET;

        cout.flush();

        // Sleep using std::this_thread::sleep_for
        this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    cout << CYAN << "\n> List of commands:" << endl
        << "    - initialize            (initializes processor configuration and scheduler based on config.txt)" << endl
        << "    - screen -s <name>      (start a new process)" << endl
        << "    - screen -r <name>      (reattaches to an existing process)" << endl
        << "    - screen -ls            (list all processes)" << endl
        << "    - process-smi           (prints process info, only applicable when attached to a process)" << endl
        << "    - scheduler-start       (starts the creation of dummy processes at configured intervals)" << endl
        << "    - scheduler-stop        (stops the creation of dummy processes initiated by scheduler-test)" << endl
        << "    - report-util           (generates a CPU utilization report and writes it to csopesy-log.txt)" << endl
        << "    - clear                 (clears the screen)" << endl
        << "    - help                  (displays list of commands)" << endl
        << "    - exit                  (exits the emulator)" << RESET << endl;
}
