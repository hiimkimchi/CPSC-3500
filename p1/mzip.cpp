//Bryan Kim
//mzip.cpp
//This code imitates a simplified version of the UNIX command 'zip'

#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <vector>
#include <climits>

//arbitrary size to accept most cases of input
int const BUFFER_SIZE = INT_MAX;

// desc : Writes a string to a zipped binary file into 5-byte entries 
//        (4 bytes describing run length, 1 byte for the character)
// pre  : none
// post : none
void writeToZip(int file, std::vector<char> content);

//desc: compresses a file into 5-byte entries 
//pre : -a destination file must be provided for correct implementation
//      (otherwise, it will just print to stdout)
//post: -stores all lines corresponding and opened files are closed
int main (int argc, char* argv[]) {
    //if no arguments are provided, give instructions on how to use mzip and return 1
    if (argc == 1) {
        write(1, "mzip: file1 [file2 ...]\n", 24);
        return 1;
    }

    //while there are files that are to be zipped, carry out zipping action
    for (int i = 1; i < argc; i++) {
        int readFile = open(argv[i], O_RDONLY);
        std::vector<char> buffer(BUFFER_SIZE);
        std::vector<char> readFileContents;
        ssize_t stringRead;

        //if cannot open a file/there is file error, tell user and return 1
        if (readFile == -1) {
            write(1, "mcat: cannot open file\n", 23);
            close(readFile);
            return 1;
        }

        //put all of current opened file into a buffer and then append it to another vector
        while ((stringRead = read(readFile, buffer.data(), BUFFER_SIZE)) > 0) {
            readFileContents.insert(readFileContents.end(), buffer.begin(), buffer.begin() + stringRead);
        }
        close(readFile);

        //use writeToZip to write to stdout (which will be put into another file via shell redirection)
        writeToZip(1, readFileContents);
    }
    return 0;
}


void writeToZip(int file, const std::vector<char> content) {
    //if the file contents is empty, stop
    if (content.empty()) {
        return;
    }

    char targetChar = content[0];
    int count = 0;

    //iterate through the vector
    for (size_t i = 0; i < content.size(); i++) {
        //if the character is the same as the current, increment size
        if (content[i] == targetChar) {
            count++;
        //if the character is not the same as the current, write size and character into stdout
        //then set the target character as the current and set size = 1
        } else {
            //pretty much a precautionary condition to make sure the size is not 0
            if (count > 0) {
                write(file, (char*)&count, sizeof(count));
                write(file, (char*)&targetChar, sizeof(targetChar));
            }
            targetChar = content[i];
            count = 1;
        }
    }

    //write the last count and character to the destination file
    if (count > 0) {
        write(file, (char*)&count, sizeof(count));
        write(file, (char*)&targetChar, sizeof(targetChar));
    }
}