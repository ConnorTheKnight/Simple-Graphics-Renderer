#include "shape_renderer.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <mutex>
#include <algorithm>

using namespace std;

// Precision tolerance for floating-point comparisons
const float EPSILON = 1e-3f;

// Check if point (px, py) lies inside triangle (Ax, Ay)-(Bx, By)-(Cx, Cy)
bool pointInTriangle(float px, float py, float Ax, float Ay, float Bx, float By, float Cx, float Cy) {
    float areaOrig = fabs((Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By)) / 2.0f);
    float area1 = fabs((px * (By - Cy) + Bx * (Cy - py) + Cx * (py - By)) / 2.0f);
    float area2 = fabs((Ax * (py - Cy) + px * (Cy - Ay) + Cx * (Ay - py)) / 2.0f);
    float area3 = fabs((Ax * (By - py) + Bx * (py - Ay) + px * (Ay - By)) / 2.0f);
    return fabs((area1 + area2 + area3) - areaOrig) < EPSILON;
}

// Check if triangle A is fully inside triangle B
bool isTriangleFullyCovered(int i, int j, const vector<float>& positions) {
    float Ax = positions[6 * i], Ay = positions[6 * i + 1];
    float Bx = positions[6 * i + 2], By = positions[6 * i + 3];
    float Cx = positions[6 * i + 4], Cy = positions[6 * i + 5];

    float Px = positions[6 * j], Py = positions[6 * j + 1];
    float Qx = positions[6 * j + 2], Qy = positions[6 * j + 3];
    float Rx = positions[6 * j + 4], Ry = positions[6 * j + 5];

    return pointInTriangle(Ax, Ay, Px, Py, Qx, Qy, Rx, Ry) &&
        pointInTriangle(Bx, By, Px, Py, Qx, Qy, Rx, Ry) &&
        pointInTriangle(Cx, Cy, Px, Py, Qx, Qy, Rx, Ry);
}

void renderTriangles(const vector<float>& shapeValues, int verticalExtent, int horizontalExtent, int numTriangles) {
    vector<float> positions(numTriangles * 6);
    vector<bool> culled(numTriangles, false);
    vector<vector<bool>> grid(verticalExtent, vector<bool>(horizontalExtent, false));

    // Copy shape values into positions
    for (int i = 0; i < numTriangles * 6; ++i) {
        positions[i] = shapeValues[i];
    }

    // Culling pass : check for full containment
    for (int i = 0; i < numTriangles; ++i) {
        if (culled[i]) continue;

        for (int j = 0; j < numTriangles; ++j) {
            if (i == j || culled[j]) continue;

            if (isTriangleFullyCovered(i, j, positions)) {
                culled[i] = true;
                break;
            }
        }
    }

    // Rasterize triangles into the grid
    for (int i = 0; i < numTriangles; ++i) {
        if (culled[i]) continue;

        float Ax = positions[6 * i], Ay = positions[6 * i + 1];
        float Bx = positions[6 * i + 2], By = positions[6 * i + 3];
        float Cx = positions[6 * i + 4], Cy = positions[6 * i + 5];

        int minX = max(0, (int)floor(min({ Ax, Bx, Cx })));
        int minY = max(0, (int)floor(min({ Ay, By, Cy })));
        int maxX = min(horizontalExtent, (int)ceil(max({ Ax, Bx, Cx })));
        int maxY = min(verticalExtent, (int)ceil(max({ Ay, By, Cy })));

        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                if (pointInTriangle(x + 0.5f, y + 0.5f, Ax, Ay, Bx, By, Cx, Cy)) {
                    grid[y][x] = true;
                }
            }
        }
    }

    // Render grid to console
    for (int y = 0; y < verticalExtent; ++y) {
        for (int x = 0; x < horizontalExtent; ++x) {
            cout << (grid[y][x] ? "[X]" : "[_]");
        }
        cout << "\n";
    }
}
