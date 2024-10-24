// basicOS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "ConsoleManager.h"

using namespace std;

bool isRunning;
vector <string> commandList;

#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// Function to print the header and welcome message
void printHeader() {
    cout << "                              #######  #####  \n";
    cout << "#####    ##    ####  #  ####  #     # #     # \n";
    cout << "#    #  #  #  #      # #    # #     # #       \n";
    cout << "#####  #    #  ####  # #      #     #  #####  \n";
    cout << "#    # ######      # # #      #     #       # \n";
    cout << "#    # #    # #    # # #    # #     # #     # \n";
    cout << "#####  #    #  ####  #  ####  #######  #####  \n\n";
    cout << "Hello, welcome to the basicOS command line! \n";
    cout << "Type 'exit' to quit, 'clear' to clear the screen. \n";
}

// Function to print the command prompt
void printInstruc() {
    cout << "cmd: "; // Prompting user to enter a command
}

// Function to retrieve command arguments from user input
vector<string> getCommandArgs(string command, vector<string> commandArgs) {
    getline(cin, command); // Get the full line of command input from the user

    stringstream ss(command); // Create a stringstream for parsing the input
    string token; // Variable to store each token (word) extracted
    while (ss >> token) { // While there are still tokens in the input
        commandArgs.push_back(token); // Add each token to the commandArgs vector
    }
    return commandArgs; // Return the vector of command arguments
}

// Function to validate if the given command is in the list of valid commands
bool commandIsValid(string command, vector<string> commandList) {
    for (int i = 0; i < commandList.size(); i++) { // Iterate through the command list
        if (command == commandList[i]) return true; // Return true if command is valid
    }
    return false; // Return false if command is not found in the list
}

// Function to execute the command entered by the user
void executeCommand(string command) {
    // Placeholder for command execution logic
}

// Main function where the program execution begins
int main() {
    printHeader(); // Call function to print the header and welcome message

    bool isRunning = true; // Flag to control the main loop
    while (isRunning) { // Loop until isRunning is set to false
        string command = ""; // Variable to hold user command
        vector<string> commandArgs; // Vector to store command arguments

        printInstruc(); // Print the command prompt

        commandArgs = getCommandArgs(command, commandArgs); // Retrieve command arguments

        // Execute the command (currently a placeholder)
        executeCommand(command);
    }
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
