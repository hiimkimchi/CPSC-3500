#include <iostream>
#include "grid.h"
#include <unistd.h>

// Reports whether or not the input coordinates
// correspond to a valid position in the grid
bool Grid::exists(int x, int y){
    if( (x < 0) || (x >= width) ){
        return false;
    } else if( (y < 0) || (y >= height)){
        return false;
    }
    return true;
}

// Returns whether or not the cell/tile at the input
// coordinates is alive.
// Precondition: coordinates must be valid for the grid
bool Grid::get_tile(int x, int y) {
    int index = (width+1) * y + x;
    return buffer[index] == '#';
}

// Sets whether or not the cell/tile at the input
// coordinates is alive.
// Precondition: coordinates must be valid for the grid
void Grid::set_tile(int x, int y, bool value){
    int index = (width+1) * y + x;
    buffer[index] = value ? '#' : ' ';
}

// Returns grid width
int Grid::get_width(){
    return width;
}

// Returns grid height
int Grid::get_height(){
    return height;
}

// Returns whether or not the cell/tile at the input
// coordinates is alive.
// Precondition: coordinates must be valid for the grid
void Grid::print(){
    std::cout.write(buffer,(width+1)*height);
    std::cout.flush();
}

// Overwrites the state of the tile identified by the
// input coordinates, following the rules of Conway's
// Game of Life and using the input grid (other) as
// the state of the preceding generation.
// Precondition: coordinates must be valid for the grid
void Grid::update_tile(Grid& other, int x, int y){
    // Intentional delay
    usleep(100);

    // Check if cell was alive in previous state
    bool alive = other.get_tile(x,y);

    // For each cell in the 3x3 grid centered on the
    // input coordinates in the input Grid 'other'...
    int count  = 0;
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            // ... do nothing for the cell exactly
            // at (x,y)
            if( (i==0) && (j==0) ){
                continue;
            }

            // Add one to the count if adjacent
            // cells are alive
            int query_x = x+i;
            int query_y = y+j;
            if( !other.exists(query_x,query_y)){
                continue;
            }
            if( other.get_tile(query_x,query_y) ){
                count += 1;
            }
        }
    }

    // Cell is born if 3 adjacent cells are alive
    // Cell keeps living if 2 or 3 adjacent cells are alive
    if( (count == 2 && alive) || (count == 3) ){
        set_tile(x,y,true);
    } else {
        set_tile(x,y,false);
    }
}

// Constructor: Creates a grid with dimensions matching
// the input height and width, initializing all tiles as
// 'dead'
// Precondition: width and height must be positive
Grid::Grid(int w, int h)
    : height(h)
    , width(w)
{
    buffer = new char[(width+1)*height];
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            set_tile(j,i,false);
        }
    }
    for(int i=0; i<height; i++){
        buffer[(width+1)*i+width] = '\n';
    }
}

// Constructor: Creates a grid with dimensions and tile
// states matching the content of the input file, with
// each line interpreted as a row and all non-space
// characters counted as 'alive'.
Grid::Grid(std::string file_name)
{
    std::ifstream file;
    file.open(file_name);
    width  = 0;
    height = 0;
    std::string line;

    // Determine height by the number of lines
    // Determine width  by the maximum line width
    while(std::getline(file,line)){
        int line_size = line.size();
        width = (width>line_size) ? width : line.size();
        height++;
    }

    // Allocate character buffer to store tile data
    buffer = new char[(width+1)*height];

    // Reset position in the file to the start
    file.clear();
    file.seekg(0,std::ios::beg);

    // Initialize each row based off of each line in
    // the file
    int y = 0;
    while(std::getline(file,line)){
        for(int x=0; x<width; x++){
            bool val = false;
            int line_size = line.size();
            if( x<line_size ){
                val = line[x] != ' ';
            }
            this->set_tile(x,y,val);
        }
        buffer[(width+1)*y+width] = '\n';
        y++;
    }
}

// Destructor: frees the tile buffer
Grid::~Grid(){
    delete[] buffer;
}
