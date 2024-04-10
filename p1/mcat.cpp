//Bryan Kim
//mcat.cpp
//This code imitates a simplified version of the UNIX command 'cat'

#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <climits>

//arbitrary size to accept most cases of input
int const BUFFER_SIZE = INT_MAX;

//desc: displays contents of all files specified after the command call
//pre : none
//post: -prints all files' content and opened files are closed
int main(int argc, char* argv[]) {
    //if no arguments are provided, give instructions on how to use mcat and return 1
    if (argc == 1) {
        write(1, "mcat file [file ...]\n", 21);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        //create a buffer vector for reading a file's characters
        std::vector<char> buffer(BUFFER_SIZE);
        ssize_t stringRead;
        
        int file = open(argv[i], O_RDONLY); //NOTE TO SELF: O_RDONLY allows only read permissions

        //if cannot open a file/there is file error, tell user and return 1
        if (file == -1) {
            write(1, "mcat: cannot open file\n", 23);
            close(file);
            return 1;
        }

        //while there are things to read, write to cout (print it)
        while ((stringRead = read(file, buffer.data(), BUFFER_SIZE)) > 0) {
            write(1, buffer.data(), stringRead);
        }

        //close the opened file
        close(file);
    }
    return 0;
}