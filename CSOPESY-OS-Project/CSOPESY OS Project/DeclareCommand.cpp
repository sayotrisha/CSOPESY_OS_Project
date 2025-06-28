#include "DeclareCommand.h"
#include <iostream>

DeclareCommand::DeclareCommand(int pid, const std::string& var, uint16_t val,
    std::unordered_map<std::string, uint16_t>& table)
    : ICommand(pid, CommandType::DECLARE), variable(var), value(val), symbolTable(table) {
}

void DeclareCommand::execute() {
    symbolTable[variable] = value;
    std::cout << "PID: " << pid << "  DECLARE: " << variable << " = " << value << std::endl;
}
