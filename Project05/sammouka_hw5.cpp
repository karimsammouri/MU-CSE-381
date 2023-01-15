/* 
 * A custom shell that uses fork() and execvp() for running commands
 * in serially or in parallel.
 * Copyright (C) 2021 sammouka@miamioh.edu
 */

#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "ChildProcess.h"

// A vector of strings to ease running programs with command-line
// arguments.
using StrVec = std::vector<std::string>;

// A vector of integers to hold child process ID's when operating in
// parallel mode.
using IntVec = std::vector<int>;

/** Convenience method to split a given line into individual words.

    \param[in] line The line to be split into individual words.

    \return A vector strings containing the list of words. 
 */
StrVec split(const std::string& line) {
    StrVec words;  // The list to be created
    // Use a string stream to read individual words 
    std::istringstream is(line);
    std::string word;
    while (is >> std::quoted(word)) {
        words.push_back(word);
    }
    return words;
}

/**
 * Helper method of processCmds() that takes care of instances where commands 
 * are to be read from a file in a parallel fashion.
 * @param is The input stream.
 */
void parallel(std::istream& is) {
    // Variables to process user input and an extra pcs variable to store PID's 
    // of child processes.
    std::string line; StrVec words; IntVec pcs;
    // Go through file line by line.
    while (getline(is, line) && !std::cin.eof()) {
        words = split(line);
        // This ensures that empty lines and comments are ignored.
        if (words.size() == 0 || words[0] == "#")
            continue;
        // This terminates the program if an exit command is detected.
        if (words[0] == "exit")
            break;
        // This prints the command to be run including the provided 
        // arguments
        std::cout << "Running: ";
        size_t count = 0;
        for (const auto& word : words) {
            // This ensures that the last argument is printed without a 
            // space in the end, which is intended for arguments in the 
            // middle.
            (count != words.size() - 1) ? std::cout << word << " " :
                    std::cout << word;
            count++;
        }
        std::cout << std::endl;
        // The following simply runs the command using fork() and exec() and 
        // stores the PID of the child process.
        ChildProcess ps;
        int pid = ps.forkNexec(words);
        pcs.push_back(pid);
    }
    // Goes through each PID and calls waitpit() on each.
    for (const auto& pc : pcs) {
        int exitCode;
        waitpid(pc, &exitCode, 0);
        std::cout << "Exit code: " << exitCode << std::endl;
    }
}

/**
 * Helper method of flow() that takes care of instances where commands are to be 
 * read from a file.
 * @param is The input stream.
 * @param parMode True if commands are to be run in parallel mode, False for 
 * serial.
 */
void processCmds(std::istream& is, bool parMode) {
    // Variables to process user input.
    std::string line;
    StrVec words;
    // Case if user has specified serial processing.
    if (parMode == false) {
        // Go through file line by line.
        while (getline(is, line) && !std::cin.eof()) {
            words = split(line);
            // This ensures that empty lines and comments are ignored.
            if (words.size() == 0 || words[0] == "#")
                continue;
            // This terminates the program if an exit command is detected.
            if (words[0] == "exit")
                break;
            // This prints the command to be run including the provided 
            //  arguments.
            std::cout << "Running: ";
            size_t count = 0;
            for (const auto& word : words) {
                // This ensures that the last argument is printed without a 
                // space in the end, which is intended for arguments in the 
                // middle.
                (count != words.size() - 1) ? std::cout << word << " " :
                        std::cout << word;
                count++;
            }
            std::cout << std::endl;
            // The following simply runs the command using fork() and exec() 
            // and prints the exit code of the child process.
            ChildProcess ps;
            ps.forkNexec(words);
            std::cout << "Exit code: " << ps.wait() << std::endl;
        }
    } else {
        parallel(is);
    }
}

/**
 * Helper method of main() that determines flow of the program.
 * @param words The program to be run and the arguments. If first argument is 
 * either "SERIAL" or "PARALLEL", second argument is assumed to be a file 
 * containing commands to be run.
 * @return The number corresponding to the flow the program should take, dealt 
 * with appropriately by the main method.
 */
int flow(StrVec words) {
    // This ensures that empty lines and comments are ignored.
    if (words.size() == 0 || words[0] == "#") {
        return 1;
    }
    // This terminates the program if an exit command is detected.
    if (words[0] == "exit")
        return 2;
    // This deals with instances where the commands are to be run from a file 
    // using either serial or parallel processing with procdssCmds();
    if (words[0] == "SERIAL" || words[0] == "PARALLEL") {
        std::ifstream file(words[1]);
        processCmds(file, words[0] == "SERIAL" ? false : true);
        return 3;
    }
    return 0;
}

/**
 * The main function that uses the flow helper method to control 
 * flow of program depending on user input as per assignment 
 * instructions.
 * @param argc The number of arguments.
 * @param argv The actual arguments.
 * @return The return code: zero if successful, nonzero otherwise.
 */
int main(int argc, char *argv[]) {
    // Variables to process user inputs.
    std::string line;
    StrVec words;
    // Loop that repeatedly prompts user for and processes input.
    while (std::cout << "> ", std::getline(std::cin, line) && !std::cin.eof()) {
        words = split(line);
        // Utilizes flow helper method to determine flow of program.
        int status = flow(words);
        // This ensures that empty lines and comments are ignored.
        if (status == 1)
            continue;
        // This terminates the program if an exit command is detected.
        if (status == 2)
            break;
        // This means the commands were run from a file using either 
        // serial or parallel processing and that the code below, which assumes 
        // the program to be run is provided in the terminal, should be 
        // skipped. 
        if (status == 3)
            continue;
        // The following code deals with instances in which the name of the 
        // program to be executed along with arguments are directly provided 
        // in the terminal.
        // This prints the command to be run including the provided arguments.
        std::cout << "Running: ";
        size_t count = 0;
        for (const auto& word : words) {
            // This ensures that the last argument is printed without a space 
            // in the end, which is intended for arguments in the middle.
            (count != words.size() - 1) ? std::cout << word << " " :
                    std::cout << word;
            count++;
        }
        std::cout << std::endl;
        // The following simply runs the command using fork() and exec() and 
        // prints the exit code of the child process.
        ChildProcess ps;
        ps.forkNexec(words);
        std::cout << "Exit code: " << ps.wait() << std::endl;
    }
    return 0;
}

