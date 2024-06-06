//Bryan Kim
//p4.cpp
//CPSC 3500
//This code implements a client-server system where the client can manipulate files in the server's directory

#include <iostream>
#include <cstdint>
#include <thread>
// Standard *NIX headers
#include <unistd.h>
// Socket functionality
#include <sys/socket.h>
// TCP/IP protocol functionaltiy
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <climits>
#include <vector>

int BUFFER_SIZE = INT_MAX;
std::string FOUND = "FOUND";
std::string NOT_FOUND = "NOT_FOUND";

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

void sigint_handler(int signum) {
    std::cerr << "\nExiting server...\n";
    exit(1);
}

// Clients need an ip address and a port number to connect to
void client(in_addr_t ip,in_port_t port, std::string path, std::string mode);
// Servers pick an arbitrary port number and reports its port
// number to the user
void server();

// Functions for parsing ip addresses and port numbers from
// c strings
in_addr_t parse_ip(char*   ip_str);
in_port_t parse_port(char* port_str);

// Returns a socket file descriptor that is connected to the
// given ip/port
int connect_to(in_addr_t ip, in_port_t port);
// Returns a socket bound to an arbitrary port
int arbitrary_socket();
// Returns the port of the socket referenced by the input file descriptor
in_port_t get_port(int socket_fd);

//desc: sends message to a socket. it is sent in chunks in order to handle larger inputs.
//pre : -socket fd and the message itself is passed in
//      -the send syscall is warmed up via sending the size of the message
//post: -the socket fd has updated values containing the message
void send_message(int connection_fd, std::string& message);

//desc: receives message from a socket. it processes the message in chunks in order to correspond with the send function.
//pre : -socket fd and the message itself is passed in
//      -the send_message function should have a "chunked" implementation.
//post: -returns the message in the socket fd
std::string recv_message(int connection_fd);

//desc: checks if a file path is valid
//pre : -filepath and socket_no is passed in
//post: -return the corresponding exit status
int check(std::string path, int socket_no);

//desc: prints a file's contents if the file exists
//pre : -filepath and socket_no is passed in
//post: -return the corresponding exit status
int load(std::string path, int socket_no);

//desc: stores input from stdin into a file. If file does not exist, create.
//pre : -filepath and socket_no is passed in
//post: -return the corresponding exit status
int store(std::string path, int socket_no);

//desc: deletes a file
//pre : -filepath and socket_no is passed in
//post: -return the corresponding exit status
int delete_fx(std::string path, int socket_no);


//desc: runs either on client or server mode. the objective is for the client to be able to manipulate files in the server's directory.
//pre : -in order for implementation to be fully shown off, you must have a client to run the program and a server to run the program.
//post: none
int main(int argc, char *argv[]) {
    signal(SIGINT, sigint_handler);
    // A first argument must be supplied to indicate mode (client/server)
    if(argc < 2){
        std::cout << "Usage: p4 [mode] [options ...]" << std::endl;
        exit(1);
    }

    // Switch arg handling and execution based upon the mode
    std::string mode = argv[1];
    if (mode == "server"){
        // Servers need no arguments, and simply report the port and
        // ip address they end up having
        if(argc != 2){
            std::cout << "Usage: p4 server" << std::endl;
            exit(1);
        }
        server();
    } else if (mode == "check") {
        if(argc < 5) {
            std::cout << "Usage: p4 check <ip> <port> <path>" << std::endl;
            exit(1);
        }
        //check if the path exists in the server
        std::string path = argv[4];
        client(parse_ip(argv[2]), parse_port(argv[3]), path, mode);
        //exit with the status it returns
    } else if (mode == "load"){
        if(argc < 5) {
            std::cout << "Usage: p4 load <ip> <port> <path>" << std::endl;
            exit(1);
        }
        std::string path = argv[4];
        client(parse_ip(argv[2]), parse_port(argv[3]), path, mode);
    } else if (mode == "store") {
        if(argc < 5) {
            std::cout << "Usage: p4 store <ip> <port> <path>" << std::endl;
            exit(1);
        }
        std::string path = argv[4];
        client(parse_ip(argv[2]), parse_port(argv[3]), path, mode);
    } else if (mode == "delete") {
        if(argc < 5) {
            std::cout << "Usage: p4 delete <ip> <port> <path>" << std::endl;
            exit(1);
        }
        std::string path = argv[4];
        client(parse_ip(argv[2]), parse_port(argv[3]), path, mode);
    } else {
        std::cout << "Mode '" << mode << "' not recognized" << std::endl;
    }
    return 0;
}


// desc : Parses a string to an ip address
// pre  : ip_str points to a valid c_string
// post : Returns the parsed ip or throws a runtime error
in_addr_t parse_ip(char*   ip_str) {
    // The 'in_addr_t' type represents an ip address, and can be
    // parsed from a string using 'inet_addr'
    in_addr_t ip_addr = inet_addr(ip_str);
    // If the parsing failed, the INADDR_NONE value will be produced
    if (ip_addr == INADDR_NONE) {
        throw std::runtime_error("Failed to convert input ip address.");     
    }
    return ip_addr;
}

// desc : Parses a string to a port number
// pre  : port_str points to a valid c_string
// post : Returns the parsed port number or throws a runtime exception
in_port_t parse_port(char* port_str) {
    // Parse port number from argument
    in_port_t port = atoi(port_str);
    // 'atoi' returns zero on error. Port zero is not a 'real' port.
    if(port == 0) {
        throw std::runtime_error("Invalid port argument.");     
    }
    return port;
}


// desc : Returns a tcp/ip socket
// pre  : None
// post : Returns a tcp/ip socket or throws a runtime exception
int make_tcp_ip_socket() {
    // Make a socket, which is a special type of file that acts as a
    // holding area for sent/recieved data.
    //
    //  - PF_INET means the Port Family is for InterNET
    //  - SOCK_STREAM indicates it should use the TCP protocol
    int socket_fd = socket(PF_INET,SOCK_STREAM,0);
    // If the fd is negative, socket allocation failed
    if(socket_fd < 0){
        throw std::runtime_error("Could not allocate socket.");
    }
    return socket_fd;
}


// desc : Returns a socket connected to the given ip address and
//        port number
// pre  : ip is a valid ip address. port is a valid port number
// post : If an error is encountered, a runtime exception is thrown
int connect_to(in_addr_t ip, in_port_t port) {
    // Set up socket address data structure, which we will use
    // to tell the OS what we want to do with our socket
    sockaddr_in socket_addr;
    // AF_INET means the Address Family is for InterNET
    socket_addr.sin_family = AF_INET;
    // Set the ip address to connect to
    socket_addr.sin_addr.s_addr = ip;
    // Set the port to connect to
    // htons converts endianness from host to network
    socket_addr.sin_port = htons(port);

    // Make socket to connect through
    int socket_fd = make_tcp_ip_socket();

    // Tell the OS we want to connect to the ip/port indicated by
    // socket_addr through the socket represented by the file
    // descriptor 'socket_fd'
    int status = connect(socket_fd,(sockaddr*)&socket_addr,sizeof(socket_addr));
    // If output is negative, the connection was not successful.
    if(status < 0) {
        // Make sure socket get cleaned up
        close(socket_fd);
        throw std::runtime_error("Connection failed.");
    }
    return socket_fd;
}

// desc : Returns a socket bound to an arbitrary port
// pre  : None
// post : If an error is returned, a runtime exception is thrown
int arbitrary_socket() {
    // Set up socket address data structure, which we will use
    // to tell the OS what we want to do with our socket
    sockaddr_in socket_addr;
    // AF_INET means the Address Family is for InterNET
    socket_addr.sin_family = AF_INET;
    // Indicate we are willing to connect with any ip address
    socket_addr.sin_addr.s_addr = INADDR_ANY;
    // Use zero-valued port to tell OS to pick any available
    // port number
    socket_addr.sin_port = 0;

    // Make a socket to listen through
    int socket_fd = make_tcp_ip_socket();

    // Bind socket to an arbitrary available port
    int status = bind(
        socket_fd,
        (struct sockaddr *) &socket_addr,
        sizeof(sockaddr_in)
    );
    if(status < 0) {
        throw std::runtime_error("Binding failed.");
    }
    return socket_fd;
}

// desc : Returns the port that the provided file descriptor's
//        socket is bound to
// pre  : The provided socket file descriptor is valid
// post : If an error is encountered, a runtime exception is thrown
in_port_t get_port(int socket_fd) {
    // A receptacle for the syscall to write the port number
    sockaddr_in socket_addr;
    // You need to supply the size of the receptacle through
    // a pointer. This seems rather silly, but is onetheless necessary.
    socklen_t socklen = sizeof(sockaddr_in);
    // Get the "name" (aka port number) of socket
    int status = getsockname(
        socket_fd,
        (struct sockaddr *) &socket_addr,
        &socklen
    );
    if (status < 0) {
        throw std::runtime_error("Failed to find socket's port number.");
    }
    // Flip endianness from network to host
    return ntohs(socket_addr.sin_port);
}

void send_message(int connection_fd, std::string& message) {
    size_t message_length = message.size();

    //send the size of the message
    if (send(connection_fd, &message_length, sizeof(message_length), 0) != sizeof(message_length)) {
        close(connection_fd);
        throw std::runtime_error("Failed to send message size.");
    }

    //send the actual message
    size_t total_sent = 0;
    while (total_sent < message_length) {
        ssize_t sent = send(connection_fd, message.c_str() + total_sent, message_length - total_sent, 0);
        if (sent <= 0) {
            close(connection_fd);
            throw std::runtime_error("Failed to send message.");
        }
        total_sent += sent;
    }
}

std::string recv_message(int connection_fd) {
    size_t message_size;
    std::vector<char> buffer(BUFFER_SIZE);

    //receive the message size
    size_t total_received = 0;
    while (total_received < sizeof(message_size)) {
        ssize_t received = recv(connection_fd, buffer.data() + total_received, sizeof(message_size) - total_received, 0);
        if (received <= 0) {
            throw std::runtime_error("Failed to receive message size.");
        }
        total_received += received;
    }

    //get the message size from the buffer and resize the buffer
    std::memcpy(&message_size, buffer.data(), sizeof(message_size));
    buffer.resize(message_size + sizeof(message_size));

    //receive the message data
    total_received = sizeof(message_size);
    while (total_received < sizeof(message_size) + message_size) {
        ssize_t received = recv(connection_fd, buffer.data() + total_received, sizeof(message_size) + message_size - total_received, 0);
        if (received <= 0) {
            throw std::runtime_error("Failed to receive message.");
        }
        total_received += received;
    }

    //extract the message data from the buffer
    std::string message(buffer.data() + sizeof(message_size), buffer.data() + sizeof(message_size) + message_size);
    return message;
}

void connection (int connection_fd) {
    std::string message = recv_message(connection_fd);
    //handle server via splitting into modes
    std::string command(message);
    std::string mode;
    std::string path;
    std::string input;

    mode = command.substr(0, command.find(' '));  
    path = command.substr(command.find(' ') + 1);   

    //check if the message contains a tilde. if so, it is for the store method
    size_t tilde_pos = path.find('~');
    if (tilde_pos != std::string::npos) {
        input = path.substr(tilde_pos + 1);
        path = path.substr(0, tilde_pos);
    }

    //give extra permissions to store function
    int fd;
    if (mode == "CHECK" || mode == "LOAD" || mode == "DELETE") {
        fd = open(path.c_str(), O_RDONLY);
    } else if (mode == "STORE") {
        fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    }


    if (mode == "CHECK") {
        //if file exists, found. if not, it is not found.
        if (fd == -1) {
            send_message(connection_fd, NOT_FOUND);
        } else {
            send_message(connection_fd, FOUND);
        }
    } else if (mode == "LOAD") {
        //check if the file opens, and then send everything
        if (fd == -1) {
            send_message(connection_fd, NOT_FOUND);
        } else {
            std::vector<char> buffer(BUFFER_SIZE);
            ssize_t bytes_read;

            //eead the file in chunks and send each chunk
            while ((bytes_read = read(fd, buffer.data(), BUFFER_SIZE)) > 0) {
                std::string chunk(buffer.data(), bytes_read);
                send_message(connection_fd, chunk);
            }

            if (bytes_read < 0) {
                std::cerr << "Error reading file: " << std::endl;
            }
        }
    } else if (mode == "STORE") {
        //check if file error
        if (fd == -1) {
            send_message(connection_fd, NOT_FOUND);
        } else {
            ssize_t bytes_written = write(fd, input.c_str(), input.length());
            if (bytes_written == -1) {
                send_message(connection_fd, NOT_FOUND);
            } else {
                send_message(connection_fd, FOUND);
            }
        }
    } else if (mode == "DELETE") {
        //check if file exists, if it does, unlink it (delete)
        if (fd == -1) {
            send_message(connection_fd, NOT_FOUND);
        } else {
            unlink(path.c_str());
            send_message(connection_fd, FOUND);
        }
    }

    //close connection once transaction is complete 
    close(connection_fd);
    close(fd);
}

// desc : Connects to server and sends a one-line message
// pre  : ip is a vaid ip address and port is a valid port number
// post : If an error is encountered, a runtime exception is thrown
void client(in_addr_t ip, in_port_t port, std::string path, std::string mode) {
    // Attempt to connect to server through a new socket.
    // Return early if this fails.
    int socket_fd = connect_to(ip,port);
    int result = 1;
    if(socket_fd < 0) {
        return;
    }

    if (mode == "check") {
        result = check(path, socket_fd);
        close(socket_fd);
        exit(result);
    } else if (mode == "load") {
        result = load(path, socket_fd);
        close(socket_fd);
        exit(result);
    } else if (mode == "store") {
        result = store(path, socket_fd);
        close(socket_fd);
        exit(result);
    } else if (mode == "delete") {
        result = delete_fx(path, socket_fd);
        close(socket_fd);
        exit(result);
    }

}

// desc : Listens on an arbitrary port (announced through stdout)
//        for connections, recieving messages as 32-bit string
//        lengths followed my a sequence of characters matching that
//        length.
// pre  : None
// post : If a listening socket cannot be set up, a runtime exception
//        is thrown. If a connection fails or disconnects early, the
//        error is announced but the server continues operation.
void server() {

    // Make an arbitrary socket to listen through
    int socket_fd = arbitrary_socket();
    int port = get_port(socket_fd);

    std::cout << "Setup server at port "<< port << std::endl;

    // Tell OS to start listening at port for its set protocol
    // (in this case, TCP IP), with a waiting queue of size 1.
    // Additional connection requests that cannot fit in the
    // queue will be refused.
    int status = listen(socket_fd,1024);
    if( status < 0 ) {
        std::cout << "Listening failed." << std::endl;
        return;
    }

    // A receptacle to store information about sockets we will
    // accept connections through
    sockaddr_storage storage;
    socklen_t socket_len = sizeof(sockaddr_storage);
    while(true){
        
        // Wait until a connection request arrives at the port
        int connection_fd = accept(
            socket_fd,
            (sockaddr*)&storage,
            &socket_len
        );

        // Ignore failures. Clients can do weird things sometimes, and
        // it's not a good idea to simply crash whenever that happens.
        if(connection_fd < 0){
            std::cout << "Could not accept connection.\n";
            continue;
        }
        connection(connection_fd);
        /*
        std::thread connect(connection, connection_fd);
        connect.detach();
        */
    }
}


int check (std::string path, int socket_no) { 
    std::string message = "CHECK " + path;

    //send and receive
    send_message(socket_no, message);
    std::string received_message = recv_message(socket_no);

    //if the file path is not found, return 1, if not return 0
    if (received_message != "FOUND") {
        std::cerr << "\nFile path not found.\n";
        return 1;
    } else {
        std::cout << "\nFile path found!\n";
        return 0;
    }
}

int load (std::string path, int socket_no) {
    //while there are things to read, write to cout (print it)
    std::string message = "LOAD " + path;   
    send_message(socket_no, message);
    std::string received_message = recv_message(socket_no);

    if (received_message == "NOT_FOUND") {
        std::cerr << "\nFile path not found.\n";
        return 1;
    } else {
        received_message.pop_back();
        std::cout << received_message;
        std::cout << std::endl;
        return 0;
    }
}

int store(std::string path, int socket_no) {
    std::cout << "Enter the content to store (Ctrl+D to end input):\n";
    std::string input;
    std::string line;
    while (std::getline(std::cin, line)) {
        input += line + '\n'; 
    }

    std::string message = "STORE " + path + '~' + input;
    std::cout << std::endl << std::endl << "You have inputted: " << input;

    send_message(socket_no, message);
    std::string received_message = recv_message(socket_no);

    if (received_message == "FOUND") {
        std::cout << "\nSuccessfully stored data.\n";
        return 0;
    } else {
        std::cout << "\nStoring was unsuccessful.\n";
        return 1;
    }
}

int delete_fx(std::string path, int socket_no) {
    std::string message = "DELETE " + path;

    send_message(socket_no, message);
    std::string received_message = recv_message(socket_no);

    if (received_message == "FOUND") {
        std::cout << "\nSuccessfully deleted file.\n";
        return 0;
    } else {
        std::cout << "\nDeletion was unsuccessful.\n";
        return 1;
    }
    return 0;
} 