//Bryan Kim
//p3.cpp
//This code implements Conway's Game of Life with multithreading

#include "grid.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

//global condition variables
std::condition_variable menu_cond;      //to handle opening menu via SIGTSTP
std::condition_variable ready_cond;     //to handle update and print threads to resume after "R" in menu
std::condition_variable terminate_cond; //to handle returning for update and print threads
std::mutex sig_mut;    //mutex that synchronizes update and print threads to the menu thread

bool is_running = true;
bool is_menu_active = false;
bool is_terminated = false;

//desc: handler for SIGINT signal
//pre : -must only be evoked when a SIGINT is done by user
//post: -program is exited with status code 2
void sigint_handler(int signum) {
    //set as not running, unblock the menu, and return from all threads
    is_running = !is_running;
    is_terminated = !is_terminated;
    menu_cond.notify_one();
    std::cerr << "\nExiting immediately...\n";
    exit(2);
}

//desc: handler for SIGTSTP signal
//pre : -must only be evoked when SIGTSTP is done by user
//      -a declared global condition variable is assumed
//post: -unlocks mutex that allows the menu thread to run
void sigstp_handler(int signum) {
    menu_cond.notify_all();
    is_menu_active = !is_menu_active;
}

//desc: displays an input menu and stops all other processes while running
//pre : -should only be invoked via SIGTSTP
//post: -when 'Q' is the input, exit the whole program
//      -when 'R' is the input, resume other processes
void menu (int *frame_rate, int *sim_rate, Grid **display_grid, Grid **working_grid, std::mutex *mut);

//desc: prints the display grid
//pre : none
//post: none
void print_cycle (Grid **display_grid, int *frame_rate, std::mutex *mut);

//desc: updates the working grid and swaps grids after one iteration via thread
//pre : none
//post: -all threads are synchronized correctly
void update_grid (Grid **working_grid, Grid **display_grid, int *sim_rate, std::mutex *mut);

//desc: updates one row of the working grid via thread
//pre : -should be called only within the update thread
//post: none
void update_row (Grid *working_grid, Grid *display_grid, int x);

//desc: runs Conway's Game of life with multithreading. input, printing, and updating will be in seperate threads
//pre : -global conditionals, mutexes, and booleans are initialized before execution
//post: -all threads are synchronized correctly
//      -all dynamic memory is deallocated
int main(int argc, char *argv[]) {
    //initialize signal handlers, 
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigstp_handler);

    //if there are too few or too many arguments, stop
    if (argc != 2) {
        std::cerr << "Usage:\np3 <cgol_file.txt>\n";
        return 1;
    }

    //initialize objects, and method scoping variables
    std::string file_path = argv[1];
    Grid *display_grid = new Grid(file_path);
    Grid *working_grid = new Grid(display_grid->get_width(), display_grid->get_height());
    int frame_rate = 10;
    int sim_rate = 10;
    std::mutex print_mut;

    std::thread input_menu(menu, &frame_rate, &sim_rate, &display_grid, &working_grid, &print_mut);

    std::thread print (print_cycle, &display_grid, &frame_rate, &print_mut);

    std::thread update (update_grid, &working_grid, &display_grid, &sim_rate, &print_mut);

    input_menu.join();
    print.join();
    update.join();

    //deallocate grids
    delete display_grid;
    delete working_grid;

    return 0;
}


void menu (int *frame_rate, int *sim_rate, Grid **display_grid, Grid **working_grid, std::mutex *mut) {
    std::string input;
    while (is_running) {
        //wait until the SIGTSTP is invoked
        if(!is_menu_active) {
            std::unique_lock ulock(*mut);
            menu_cond.wait(ulock);
        }

        //in the case SIGINT is done while the menu is up
        if (is_terminated) {
            return;
        }

        std::cout << "\n\nCGOL Menu:\n\n\"Q\": quit the current game\n\"R\": exit the menu\n\"S+\": simulation rate increase by 1"
                  << "\n\"S-\": simulation rate decrease by 1\n\"D+\": frame rate increase by 1\n\"D-\": frame rate decrease by 1\n";
        std::cin >> input;
        if (input == "Q") {
        //return and deal with deallocation
            kill(0,SIGINT);
            ready_cond.notify_all();
            is_running = false;
            is_menu_active = false;
            is_terminated = true;
            std::unique_lock ulock(sig_mut);
            terminate_cond.wait(ulock);
            delete *display_grid;
            delete *working_grid;
            kill(0, SIGINT);
            return;
        } else if (input == "R") {
        //unblock update and print threads and return to the simulation
            std::cout << "\nReturning to the game...\n";
            ready_cond.notify_all();
            is_menu_active = false;
        } else if (input == "S+") {
        //sim rate up 1
            (*sim_rate)++;
            std::cout << "Simulation Rate has been increased by 1.\nIt is now: " << *sim_rate;
        } else if (input == "S-") {
        //sim rate down 1
            if (*sim_rate == 0) {
                std::cout << "Simulation Rate cannot be negative.\n";
            }
            (*sim_rate)--;
            std::cout << "Simulation Rate has been decreased by 1.\nIt is now: " << *sim_rate;
        } else if (input == "D+") {
        //frame rate up 1
            (*frame_rate)++;
            std::cout << "Frame Rate has been increased by 1.\nIt is now: " << *frame_rate;
        } else if (input == "D-") {
        //frame rate down 1
            if (*frame_rate == 0) {
                std::cout << "Frame Rate cannot be negative.\n";
            }
            (*frame_rate)--;
            std::cout << "Frame Rate has been decreased by 1.\nIt is now: " << *frame_rate;
        } else {
            std::cout << "Invalid input. Printing instructions again...\n\n";
        }
    }
}


void print_cycle (Grid **display_grid, int *frame_rate, std::mutex *mut) {
    while (is_running) {
        //if the menu is open, the condition waits until the menu returns to game
        if(is_menu_active) {
            std::unique_lock ulock(sig_mut);
            ready_cond.wait(ulock);
        }

        //if the game is terminated in the menu, wait until update sends condition and return to end the process
        if(is_terminated) {
            std::unique_lock ulock(*mut);
            terminate_cond.wait(ulock);
            return;
        }

        //prints INDEPENDENT of the update
        mut->lock();
        if (!is_menu_active) {
            (*display_grid)->print();
        }
        mut->unlock();

        //sleep for 1/frame_rate seconds
        int sleep_duration_ms = 1000 / *frame_rate;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration_ms));
    }
}


void update_row (Grid *working_grid, Grid *display_grid, int y) {
    for (int x = 0; x < display_grid->get_width(); x++) {
        working_grid->update_tile(*display_grid, x, y);
    }
}


void update_grid (Grid **working_grid, Grid **display_grid, int *sim_rate, std::mutex *mut) {
    while (is_running) {
        //if the menu is open, the condition waits until the menu returns to game
        if(is_menu_active) {
            std::unique_lock ulock(sig_mut);
            ready_cond.wait(ulock);
        }

        //if the game is terminated in the menu, notify menu and print thread to deallocate and return and return to end the process
        if(is_terminated) {
            terminate_cond.notify_all();
            return;
        }

        //store threads
        std::vector<std::thread> threads;

        //create threads to update each row of the grid
        for (int y = 0; y < (*display_grid)->get_height(); y++) {
            threads.push_back(std::thread (update_row, *working_grid, *display_grid, y));
        }

        //join all threads before returning
        for (int i = 0; i < threads.size(); i++) {
            threads[i].join();
        }

        //swap the display and working grids INDEPENDENT of the print
        mut->lock();
        Grid *temp = *display_grid;
        *display_grid = *working_grid;
        *working_grid = temp;
        mut->unlock();

        //sleep for 1/sim_rate seconds
        int sleep_duration_ms = 1000 / *sim_rate;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration_ms));
    }
    delete *display_grid;
    delete *working_grid;
}