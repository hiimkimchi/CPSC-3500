//Bryan Kim
//mgrep.cpp
//This code imitates a simplified version of the UNIX command 'grep'

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <vector>
#include <climits>

//arbitrary size to accept most cases of input
int const BUFFER_SIZE = INT_MAX;

//global variable for syscall
bool isContinuing = true;

//define the sighandler_t type
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

void sigint_handler(int signalNo) {
    isContinuing = false;
    exit(signalNo);
}

//desc: prints all lines that contain the relevant phrase or char in a certain file
//pre : -file is provided as an integer representing the currently open file
//      -if using standard input, file is 0 (stdin)
//post: none
void printRelevant (int file, std::string targetString);

//desc: displays all lines that correspond to a given character or phrase. if no 
//      file is specified, get from standard input until ^C
//pre : none
//post: -prints all lines corresponding and opened files are closed
int main(int argc, char* argv[]) {
    //if no arguments are provided, give instructions on how to use mgrep and return 1
    if (argc < 2) {
        write(1, "mgrep searchterm [file ...]\n", 28);
        return 1;
    }

    sighandler_t handler = signal(SIGINT, sigint_handler);
    std::string targetString = argv[1];

    //if there is only a target string and no files, loop until SIGINT
    if (argc == 2) {
        while (isContinuing) {
            printRelevant(0, targetString);
        }
        return 0;
    }

    //start loop at the third argument
    for (int i = 2; i < argc; i++) {
        int file = open(argv[i], O_RDONLY);
        
        //if cannot open a file/there is file error, tell user and return 1
        if (file == -1) {
            write (1, "mgrep: cannot open file\n", 24);
            close(file);
            return 1;
        }

        //print relevant lines and then close file
        printRelevant(file, targetString);
        close(file);
    }
    return 0;
}

void printRelevant (int file, std::string targetString) {
    //create a buffer vector for reading a file's characters and initialize variables
    std::vector<char> buffer(BUFFER_SIZE);
    std::vector<char> currentLineBuffer;
    std::string currentLine;
    ssize_t stringRead;

    //read data from the file into a buffer
    while ((stringRead = read(file, buffer.data(), BUFFER_SIZE)) > 0) {
        //loop through the chars on the current stringRead line
        for (int i = 0; i < stringRead; i++) {

            //when newline is found, process string
            if (buffer[i] == '\n') {
                if (!currentLineBuffer.empty()) {
                    //transfer buffer to currentLine for processing
                    for (int j = 0; j < currentLineBuffer.size(); j++) {
                        currentLine += currentLineBuffer[j];
                    }
                    currentLineBuffer.clear();

                    //if we find the target and it is not an invalid format, print
                    if (currentLine.find(targetString) != std::string::size_type(-1)) {
                        write(1, currentLine.c_str(), currentLine.length());
                        write(1, "\n", 1);
                    }
                    currentLine.clear();   
                }
            } else {
                //when it is not a newline character, add to line buffer
                currentLineBuffer.push_back(buffer[i]);
            }
        }
    }

    //if the buffer is empty and the currentLine is not empty, check the line and print
    if(!currentLineBuffer.empty()) {
        for (int j = 0; j < currentLineBuffer.size(); j++) {
            currentLine += currentLineBuffer[j];
        }
        if (currentLine.find(targetString) != std::string::size_type(-1)) {
            write(1, currentLine.c_str(), currentLine.length());
            write(1, "\n", 1);
        }
    }
}