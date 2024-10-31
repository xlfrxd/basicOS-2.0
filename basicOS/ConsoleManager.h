
//-
//  ConsoleManager.h
//  basicOS
//
//  CSOPESY S16
//  Group 2
//  
//  Izabella Imperial
//  Marc Daniel Marasigan
//  Nikolai Santiago
//  Alfred Victoria
//

#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <iostream> // basic input output   
#include <string> // getline func
#include <stdlib.h> // clear screen
#include <windows.h> // colors 
#include <map> // screen manager
#include <ctime> // time stamp
#include <iomanip> // time format
#include <sstream> // tokenize
#include <vector> // token vector
#include <fstream> // text file r/w
#include <queue> // queue lib
#include <thread> // threading
#include <mutex> // mutex
#include <condition_variable> // thread syncing
#include <chrono> // time
#include <atomic> // atomic

// Constants for colors
extern const int RED;
extern const int GREEN;
extern const int YELLOW;
extern const int BLUE;
extern const int RESET;

// Struct to hold information about each screen/process
struct ScreenInfo {
    std::string processName;
    int currentLine;
    int totalLines;
    std::string creationTimestamp;
    std::vector<std::string> commandArr;
    std::string logFileName;
};

// Global variables
extern std::map<std::string, ScreenInfo> screens;
extern std::string currentScreen;
extern std::vector<std::string> MAIN_MENU_CMD;
extern bool isRunning;

// Function declarations
void printHeader();
std::vector<std::string> getCommandArgs(std::string, std::vector<std::string> commandArgs);
bool commandIsValid(std::string, std::vector<std::string>);
void executeCommand(std::string);

#endif // CONSOLEMANAGER_H