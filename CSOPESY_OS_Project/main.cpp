/**
*  CSOPESY_OS_Project.cpp : This file contains the 'main' function. Program execution begins and ends there.
*  CSOPESY - S19 (Group 10)
*  Members:
*          - MARTIN, ELAINE RIZ COCAL
*          - ORDINARIO, KARL ANDREI LARITA
*          - SAYO, TRISHA ALISSANDRA
*
*  Submitted to: Mr. DE LA CRUZ, REN TRISTAN A.
**/

using namespace std;

#include <iostream>
#include <cctype>


void header() {
    cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n";
    cout << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .-'\\  `.'  /  \n";
    cout << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n";
    cout << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n";
    cout << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'     \n";
    cout << "                                                            \n";
}

void newLine() {
    cout << "\n";
}

void help() {
    cout << "List of commands:\n";
    cout << "initialize     - Initializes the program\n";
    cout << "screen         - Displays the screen\n";
    cout << "scheduler-test - Tests the scheduler\n";
    cout << "scheduler-stop - Stops the scheduler\n";
    cout << "report-util    - Generates a report\n";
    cout << "clear          - Clears the screen\n";
    cout << "exit           - Exits the program\n";
}

int main()
{
    header();
    newLine();
    help();
    string command = "";

    while (command != "exit") {
        cout << "Enter command: ";
        cin >> command;
        // insert code that converts command variable to lowercase
        for (int i = 0; i < command.length(); i++) {
            command[i] = tolower(command[i]);
        }

        if (command == "initialize") {
            cout << "'initialize' command recognized. Doing something.";
            newLine();
        }
        else if (command == "screen") {
            cout << "'screen' command recognized. Doing something.";
            newLine();
        }
        else if (command == "scheduler-test") {
            cout << "'scheduler-test' command recognized. Doing something.";
            newLine();
        }
        else if (command == "scheduler-stop") {
            cout << "'scheduler-stop' command recognized. Doing something.";
            newLine();
        }
        else if (command == "report-util") {
            cout << "'report-util' command recognized. Doing something.";
            newLine();
        }
        else if (command == "clear") {
            system("cls");
            header();
        }
        else if (command != "exit") {
            cout << "'" << command << "' command not recognized.";
            newLine();
        }
    }

    return 0;
}
