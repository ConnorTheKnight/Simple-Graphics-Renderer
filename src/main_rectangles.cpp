#include "engine.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm> // For std::max and std::min

using namespace std;

struct Rectangle {
    float width;
    float height;
    float x;
    float y;
};

bool checkOverlap(const Rectangle& r1, const Rectangle& r2) {
    // Check for overlap, considering the 1-unit space
    return r1.x < r2.x + r2.width + 1 &&
           r1.x + r1.width + 1 > r2.x &&
           r1.y < r2.y + r2.height + 1 &&
           r1.y + r1.height + 1 > r2.y;
}

int main() {
    //-----Declare Variables-----
    int verticalExtentOfGrid = 0;         //Store number of vertical grid units (number of rows)
    int horizontalExtentOfGrid = 0;       //Store number of horizontal grid units (number of columns)
    int numberOfShapesToRender = 0;       //Store number of shapes to be provided for rendering
    vector<Rectangle> rectangles;         //Store properties of each rectangle
    bool** isFilled;                      //Store whether or not a gridUnit is occupied

    //-----Read User Input from "rectangle_input.txt"-----
    std::ifstream in("C:\\Users\\nwass\\Desktop\\Simple-Graphics-Renderer\\src\\rectangle_input.txt");
    if (!in) {
        std::cerr << "Failed to open rectangle_input.txt\n";
        return 1; // or handle error appropriately
    }

    // Read grid dimensions
    in >> verticalExtentOfGrid;
    in >> horizontalExtentOfGrid;

    // Read number of shapes
    in >> numberOfShapesToRender;

    // Read (and ignore) thread count for compatibility
    int unusedThreads;
    in >> unusedThreads;

    // Initialize the vector of rectangles
    rectangles.resize(numberOfShapesToRender);

    // Initialize isFilled Array (all false by default)
    isFilled = new bool*[verticalExtentOfGrid];
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        isFilled[i] = new bool[horizontalExtentOfGrid];
        std::fill_n(isFilled[i], horizontalExtentOfGrid, false);
    }

    // Read each rectangle's data (width, height, x, y)
    for (int i = 0; i < numberOfShapesToRender; i++) {
        in >> rectangles[i].width;   // width
        in >> rectangles[i].height;  // height
        in >> rectangles[i].x;       // X
        in >> rectangles[i].y;       // Y
    }
    in.close();

    //-----Begin Processing and Drawing-----
    for (int i = 0; i < numberOfShapesToRender; ++i) {
        bool canPlace = true;
        for (int j = 0; j < i; ++j) {
            if (checkOverlap(rectangles[i], rectangles[j])) {
                canPlace = false;
                std::cerr << "Warning: Rectangle " << i << " overlaps with Rectangle " << j << ". Skipping placement." << std::endl;
                break;
            }
        }

        if (canPlace) {
            // Calculate grid bounds for this rectangle
            int minX = static_cast<int>(rectangles[i].x);
            int minY = static_cast<int>(rectangles[i].y);
            int maxX = static_cast<int>(rectangles[i].x + rectangles[i].width + 0.5f); // +0.5f for rounding
            int maxY = static_cast<int>(rectangles[i].y + rectangles[i].height + 0.5f);

            // Clamp to grid boundaries
            minX = std::max(0, minX);
            minY = std::max(0, minY);
            maxX = std::min(horizontalExtentOfGrid, maxX);
            maxY = std::min(verticalExtentOfGrid, maxY);

            // Fill in the grid cells covered by this rectangle
            for (int Y = minY; Y < maxY; Y++) {
                for (int X = minX; X < maxX; X++) {
                    if (Y >= 0 && Y < verticalExtentOfGrid && X >= 0 && X < horizontalExtentOfGrid) {
                        isFilled[Y][X] = true;
                    }
                }
            }
        }
    }

    // Draw to screen
    Engine engine(isFilled, horizontalExtentOfGrid, verticalExtentOfGrid, "Rectangle Rendering Project");

    if (!engine.Initialize()) {
        std::cout << std::endl << "Press any key to close program..." << std::endl;
        std::cin.get();
    }

    // Clean up allocated memory
    for (int i = 0; i < verticalExtentOfGrid; ++i) {
        delete[] isFilled[i];
    }
    delete[] isFilled;

    return 0;
}