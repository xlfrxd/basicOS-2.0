CSOPESY S16

Instructor: Dr. Neil Patrick Del Gallego

Group 6: Izabella Imperial, Marc Marasigan, Nikolai Santiago, Alfred Victoria


Instructions:
1. Open the project folder in Visual Studio 
2. On the ```basicOS.cpp``` file run "Local Window Debugger"
3. Upon console start, type the ```initialize``` command to detect the OS configs (if the console does not detect the .txt file, delete the file and make a new config file)
    - for the "scheduler" config, type the algorithm without quotation marks
4. Typing ```exit``` will terminate the console
5. To change the ```config.txt``` settings, open the .txt file in the file explorer (in case the .txt file might not show in your machine's VS solution explorer) 
6. Upon initialization you now have access to the following features:
    - “screen”:
        o	```screen -s <process name>``` : console will clear its contents and “move” to the process screen
        o	```screen -r <process name>``` : access the created screens anytime as long as they are still running
	        * ```process-smi``` : Prints a simple information of the process. The process contains dummy instructions that the CPU executes in the background. Every time the user types “process-smi”, it provides the updated details. If the process has finished, simply print “Finished!” after the process name and ID has been printed.
          * ```exit``` : Returns the user to the main menu.
        o	```screen -ls``` : should list the CPU utilization, cores used, and cores available, as well as print a summary of the running and finished processes.
    - ```scheduler-test``` : continuously generates a batch of dummy processes for the CPU scheduler. Each process is accessible via the “screen” command.
    - ```scheduler-stop``` : stops generating dummy processes. 
    - ```report-util``` : for generating CPU utilization report
