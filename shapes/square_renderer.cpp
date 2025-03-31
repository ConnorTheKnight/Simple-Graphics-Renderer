#include "shape_renderer.h"

using namespace std;

// Check if square A is fully inside square B
bool isSquareFullyInside(
    float xA, float yA, float lenA,
    float xB, float yB, float lenB
) {
    return (xA >= xB) && (yA >= yB) &&
        (xA + lenA <= xB + lenB) &&
        (yA + lenA <= yB + lenB);
}

void renderSquares(const vector<float>& shapeValues, int gridHeight, int gridWidth, int numSquares) {
    vector<float> lengths(numSquares);
    vector<float> posX(numSquares), posY(numSquares);
    vector<bool> culled(numSquares, false);
    vector<vector<bool>> grid(gridHeight, vector<bool>(gridWidth, false));

    // Parse square data
    for (int i = 0; i < numSquares; ++i) {
        lengths[i] = shapeValues[i * 3];
        posX[i] = shapeValues[i * 3 + 1];
        posY[i] = shapeValues[i * 3 + 2];
    }

    // Greedy culling: only cull a square if it's fully inside any already-retained square
    vector<int> retainedIndices;
    for (int i = 0; i < numSquares; ++i) {
        bool shouldCull = false;
        for (int j : retainedIndices) {
            if (isSquareFullyInside(posX[i], posY[i], lengths[i], posX[j], posY[j], lengths[j])) {
                shouldCull = true;
                break;
            }
        }
        if (!shouldCull) {
            retainedIndices.push_back(i);
        }
        else {
            culled[i] = true;
        }
    }

    // Rasterize retained squares
    for (int i = 0; i < numSquares; ++i) {
        if (culled[i]) continue;

        int minX = max(0, (int)floor(posX[i]));
        int minY = max(0, (int)floor(posY[i]));
        int maxX = min(gridWidth, (int)ceil(posX[i] + lengths[i]));
        int maxY = min(gridHeight, (int)ceil(posY[i] + lengths[i]));

        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                grid[y][x] = true;
            }
        }
    }

    // Output grid
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            cout << (grid[y][x] ? "[X]" : "[_]");
        }
        cout << "\n";
    }
}
