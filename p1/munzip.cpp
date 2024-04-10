//"cat -A text.zip" once we zip a text to see all the hidden bytes
//@ and ^ are 0
//everything that isnt that is the ASCII number for said character

//Bryan Kim
//munzip.cpp
//This code imitates a simplified version of the UNIX command 'unzip'

#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <climits>

//arbitrary size to accept most cases of input
int const BUFFER_SIZE = INT_MAX;

int main(int argc, char* argv[]) {
    //if no arguments are provided, give instructions on how to use munzip and return 1
    if (argc == 1) {
        write(1, "munzip: file1 [file2 ...]\n", 26);
        return 1;
    }

    //while there are files that are to be unzipped, carry out unzipping action
    for (int i = 1; i < argc; i++) {
        int zipFile = open(argv[i], O_RDONLY);
        std::vector<char> buffer(BUFFER_SIZE);
        ssize_t stringRead;

        //if cannot open a file/there is file error, tell user and return 1
        if (zipFile == -1) {
            write(1, "munzip: cannot open file\n", 25);
            close(zipFile);
            return 1;
        }

        while ((stringRead = read(zipFile, buffer.data(), BUFFER_SIZE)) > 0) {
            //process 5 bytes at a time
            for (size_t i = 0; i < stringRead; i += 5) {
                //set runLength to the first 4 chars and character as the 5th char
                uint32_t runLength = buffer[i];
                uint8_t character = buffer[i + 4];
                
                //write character runLength amount of times
                for (uint32_t j = 0; j < runLength; ++j) {
                    write(1, &character, sizeof(character));
                }
            }
        }
    }
    return 0;
}