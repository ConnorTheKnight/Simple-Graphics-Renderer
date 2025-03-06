#include "shape_renderer.h"

using namespace std;

// Function to check if point (x, y) is inside triangle ABC
bool pointInTriangle(float x, float y, float Ax, float Ay, float Bx, float By, float Cx, float Cy) {
    float areaOrig = abs((Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By)) / 2.0);
    float area1 = abs((x * (By - Cy) + Bx * (Cy - y) + Cx * (y - By)) / 2.0);
    float area2 = abs((Ax * (y - Cy) + x * (Cy - Ay) + Cx * (Ay - y)) / 2.0);
    float area3 = abs((Ax * (By - y) + Bx * (y - Ay) + x * (Ay - By)) / 2.0);
    return (area1 + area2 + area3 == areaOrig);
}

// Culling function to check if Triangle A is fully covered by Triangle B
void checkTriangleCull(int i, int j, vector<float>& position, vector<bool>& cull) {
    if (cull[j]) return;

    float Ax = position[6 * i], Ay = position[6 * i + 1];
    float Bx = position[6 * i + 2], By = position[6 * i + 3];
    float Cx = position[6 * i + 4], Cy = position[6 * i + 5];

    float Px = position[6 * j], Py = position[6 * j + 1];
    float Qx = position[6 * j + 2], Qy = position[6 * j + 3];
    float Rx = position[6 * j + 4], Ry = position[6 * j + 5];

    if (pointInTriangle(Ax, Ay, Px, Py, Qx, Qy, Rx, Ry) &&
        pointInTriangle(Bx, By, Px, Py, Qx, Qy, Rx, Ry) &&
        pointInTriangle(Cx, Cy, Px, Py, Qx, Qy, Rx, Ry)) {
        lock_guard<mutex> lock(mtx);
        cull[i] = true;
    }
}

void renderTriangles(const vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes) {
    vector<float> position(numberOfShapes * 6);
    vector<bool> cull(numberOfShapes, false);
    vector<vector<bool>> isFilled(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));

    for (int i = 0; i < numberOfShapes; i++) {
        position[6 * i] = shapeValues[6 * i];
        position[6 * i + 1] = shapeValues[6 * i + 1];
        position[6 * i + 2] = shapeValues[6 * i + 2];
        position[6 * i + 3] = shapeValues[6 * i + 3];
        position[6 * i + 4] = shapeValues[6 * i + 4];
        position[6 * i + 5] = shapeValues[6 * i + 5];
    }

    vector<thread> threads;
    for (int i = 0; i < numberOfShapes; i++) {
        for (int j = 0; j < numberOfShapes && !cull[i]; j++) {
            if (i != j) {
                threads.emplace_back(checkTriangleCull, i, j, ref(position), ref(cull));
            }
        }
    }
    for (auto& t : threads) {
        t.join();
    }

    for (int i = 0; i < numberOfShapes; i++) {
        if (cull[i]) continue;
        float Ax = position[6 * i], Ay = position[6 * i + 1];
        float Bx = position[6 * i + 2], By = position[6 * i + 3];
        float Cx = position[6 * i + 4], Cy = position[6 * i + 5];

        int minX = max(0, (int)min({ Ax, Bx, Cx }));
        int minY = max(0, (int)min({ Ay, By, Cy }));
        int maxX = min(horizontalExtentOfGrid, (int)max({ Ax, Bx, Cx }) + 1);
        int maxY = min(verticalExtentOfGrid, (int)max({ Ay, By, Cy }) + 1);

        for (int x = minX; x < maxX; x++) {
            for (int y = minY; y < maxY; y++) {
                if (pointInTriangle(x, y, Ax, Ay, Bx, By, Cx, Cy)) {
                    isFilled[y][x] = true;
                }
            }
        }
    }

    for (int y = 0; y < verticalExtentOfGrid; y++) {
        for (int x = 0; x < horizontalExtentOfGrid; x++) {
            cout << (isFilled[y][x] ? "[X]" : "[_]");
        }
        cout << "\n";
    }
}
