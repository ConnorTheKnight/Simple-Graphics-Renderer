#include "shape_renderer.h"

using namespace std;

// Function to check if Circle A is covered by Circle B
void checkCircleCull(int i, int j, vector<float>& position, vector<float>& infoForShape, vector<bool>& cull) {
    if (cull[j]) return; // Skip if Circle B is culled

    float XA = position[3 * i];  // X-coordinate of Circle A
    float YA = position[3 * i + 1];  // Y-coordinate of Circle A
    float radiusA = infoForShape[i];  // Radius of Circle A

    float XB = position[3 * j];  // X-coordinate of Circle B
    float YB = position[3 * j + 1];  // Y-coordinate of Circle B
    float radiusB = infoForShape[j];  // Radius of Circle B

    float deltaX = XB - XA;
    float deltaY = YB - YA;

    // If Circle A is completely covered by Circle B, mark it as culled
    if (sqrt((deltaX * deltaX) + (deltaY * deltaY)) < radiusB) {
        lock_guard<mutex> lock(mtx);
        cull[i] = true;
    }
}

// Function to render circles on the grid
void renderCircles(const vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes) {
    vector<float> infoForShape(numberOfShapes);  // Stores the radius of each circle
    vector<float> position(numberOfShapes * 3);  // Stores X, Y, and Z coordinates
    vector<bool> cull(numberOfShapes, false);  // Stores whether a circle should be culled
    vector<vector<bool>> isFilled(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));  // Grid representation

    // Read circle data from shapeValues
    for (int i = 0; i < numberOfShapes; i++) {
        infoForShape[i] = shapeValues[3 * i];  // Radius
        position[3 * i] = shapeValues[3 * i + 1];  // X-coordinate
        position[3 * i + 1] = shapeValues[3 * i + 2];  // Y-coordinate
        position[3 * i + 2] = 0;  // Z-coordinate (not used)
    }

    // Multi-threaded culling process
    vector<thread> threads;
    for (int i = 0; i < numberOfShapes; i++) {
        for (int j = 0; j < numberOfShapes && !cull[i]; j++) {
            if (i != j) {
                threads.emplace_back(checkCircleCull, i, j, ref(position), ref(infoForShape), ref(cull));
            }
        }
    }
    for (auto& t : threads) {
        t.join();
    }

    // Fill the grid with remaining (non-culled) circles
    for (int i = 0; i < numberOfShapes; i++) {
        if (cull[i]) continue;

        int minX = max(0, (int)(position[3 * i] - infoForShape[i]));
        int minY = max(0, (int)(position[3 * i + 1] - infoForShape[i]));
        int maxX = min(horizontalExtentOfGrid, (int)(position[3 * i] + infoForShape[i] + 1));
        int maxY = min(verticalExtentOfGrid, (int)(position[3 * i + 1] + infoForShape[i] + 1));

        for (int X = minX; X < maxX; X++) {
            for (int Y = minY; Y < maxY; Y++) {
                float deltaX = position[3 * i] - X;
                float deltaY = position[3 * i + 1] - Y;
                if (sqrt((deltaX * deltaX) + (deltaY * deltaY)) < infoForShape[i]) {
                    isFilled[Y][X] = true;
                }
            }
        }
    }

    // Output the final grid
    for (int Y = 0; Y < verticalExtentOfGrid; Y++) {
        for (int X = 0; X < horizontalExtentOfGrid; X++) {
            cout << (isFilled[Y][X] ? "[X]" : "[_]");  // Display filled or empty grid cells
        }
        cout << "\n";
    }
}
