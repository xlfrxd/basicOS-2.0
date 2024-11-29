/*
  CSOPESY - S16 (Group 6)
  Members:
  Izabella Imperial
  Marc Daniel Marasigan
  Nikolai Santiago
  Alfred Victoria
*/
#include <iostream>
#include <cctype>
#include <vector>
#include <ctime>
#include <fstream>
#include <random>

#include "ConsoleManager.h"
#include "InputHandler.h"
#include "ProcessScreen.h"
#include "ConsoleScreen.h"
#include "Scheduler.h"
#include "FlatMemoryAllocator.h"
#include "PagingAllocator.h"

using namespace std;

int main()
{
    ConsoleManager::initialize();
    InputHandler::initialize();

    shared_ptr<ProcessScreen> mainScreen = make_shared<ConsoleScreen>(MAIN_CONSOLE);

    ConsoleManager::getInstance()->registerConsole(mainScreen);
    ConsoleManager::getInstance()->setCurrentConsole(mainScreen);
    
    bool running = true;
    ConsoleManager::getInstance()->drawConsole();
	size_t maxOverallMem = ConsoleManager::getInstance()->getMaxOverallMem();

    FlatMemoryAllocator::initialize(maxOverallMem);
    PagingAllocator::initialize(maxOverallMem);

    while (running){
        InputHandler::getInstance()->handleMainConsoleInput();
        running = ConsoleManager::getInstance()->isRunning();
    }
    
    InputHandler::getInstance()->destroy();
    ConsoleManager::getInstance()->destroy();

    return 0;
}
