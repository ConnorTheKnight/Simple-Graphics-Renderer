#include "shape_renderer.h"

using namespace std;

void checkSquareCull(int i, int j, vector<float>& position, vector<float>& infoForShape, vector<bool>& cull) {
    if (cull[j]) return;

    float XA = position.at(3 * i);
    float YA = position.at((3 * i) + 1);
    float ZA = position.at((3 * i) + 2);
    float lengthA = infoForShape.at(i);

    float XB = position.at(3 * j);
    float YB = position.at((3 * j) + 1);
    float ZB = position.at((3 * j) + 2);
    float lengthB = infoForShape.at(j);

    if ((XA > XB) && (YA > YB) && (ZA < ZB) && (XA + lengthA < XB + lengthB) && (YA + lengthA < YB + lengthB)) {
        lock_guard<mutex> lock(mtx);
        cull.at(i) = true;
    }
}

void renderSquares(const vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes) {
    vector<float> infoForShape(numberOfShapes);
    vector<float> position(numberOfShapes * 3);
    vector<bool> cull(numberOfShapes, false);
    vector<vector<bool>> isFilled(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));

    for (int i = 0; i < numberOfShapes; i++) {
        infoForShape[i] = shapeValues[i * 3];
        position[3 * i] = shapeValues[i * 3 + 1];
        position[3 * i + 1] = shapeValues[i * 3 + 2];
        position[3 * i + 2] = 0; // Z coordinate not needed
    }

    vector<thread> threads;
    for (int i = 0; i < numberOfShapes; i++) {
        for (int j = 0; j < numberOfShapes && !cull[i]; j++) {
            if (i != j) {
                threads.emplace_back(checkSquareCull, i, j, ref(position), ref(infoForShape), ref(cull));
            }
        }
    }
    for (auto& t : threads) {
        t.join();
    }

    for (int i = 0; i < numberOfShapes; i++) {
        if (cull[i]) continue;
        int minX = max(0, (int)position[3 * i]);
        int minY = max(0, (int)position[3 * i + 1]);
        int maxX = min(horizontalExtentOfGrid, (int)(position[3 * i] + infoForShape[i] + 1));
        int maxY = min(verticalExtentOfGrid, (int)(position[3 * i + 1] + infoForShape[i] + 1));

        for (int X = minX; X < maxX; X++) {
            for (int Y = minY; Y < maxY; Y++) {
                if (X >= 0 && X < horizontalExtentOfGrid && Y >= 0 && Y < verticalExtentOfGrid) {
                    isFilled[Y][X] = true;
                }
            }
        }
    }

    for (int Y = 0; Y < verticalExtentOfGrid; Y++) {
        for (int X = 0; X < horizontalExtentOfGrid; X++) {
            cout << (isFilled[Y][X] ? "[X]" : "[_]");
        }
        cout << "\n";
    }
}
