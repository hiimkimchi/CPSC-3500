#include <fstream>

///////////////////////////////////////////////////////////
// Represents a grid of tiles in Conways Game of Life.
///////////////////////////////////////////////////////////
class Grid {

    int   height;
    int   width;
    char *buffer;

    public:

    // Reports whether or not the input coordinates
    // correspond to a valid position in the grid
    bool exists(int x, int y);

    // Returns whether or not the cell/tile at the input
    // coordinates is alive.
    // Precondition: coordinates must be valid for the grid
    bool get_tile(int x, int y);

    // Sets whether or not the cell/tile at the input
    // coordinates is alive.
    // Precondition: coordinates must be valid for the grid
    void set_tile(int x, int y, bool value);

    public:

    // Returns whether or not the cell/tile at the input
    // coordinates is alive.
    // Precondition: coordinates must be valid for the grid
    void print();

    // Overwrites the state of the tile identified by the
    // input coordinates, following the rules of Conway's
    // Game of Life and using the input grid (other) as
    // the state of the preceding generation.
    // Precondition: coordinates must be valid for the grid
    void update_tile(Grid& other, int x, int y);

    // Returns grid width
    int get_width();

    // Returns grid height
    int get_height();

    // Constructor: Creates a grid with dimensions matching
    // the input height and width, initializing all tiles as
    // 'dead'
    // Precondition: width and height must be positive
    Grid(int w, int h);

    // Constructor: Creates a grid with dimensions and tile
    // states matching the content of the input file, with
    // each line interpreted as a row and all non-space
    // characters counted as 'alive'.
    Grid(std::string file_name);

    // Destructor: frees the tile buffer
    ~Grid();

};
