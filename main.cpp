#include "shape_renderer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>

using namespace std;

mutex mtx;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile) {
        cerr << "Error: Unable to open file " << argv[1] << endl;
        inputFile.close();
        return 1;
    }

    int shapeChoice, verticalExtentOfGrid, horizontalExtentOfGrid, numberOfShapes;
    inputFile >> shapeChoice >> verticalExtentOfGrid >> horizontalExtentOfGrid >> numberOfShapes;

    // Determine the correct number of values per shape
    int valuesPerShape = (shapeChoice == 1) ? 3 : (shapeChoice == 2) ? 6 : 3;

    // Allocate vector size dynamically
    vector<float> shapeValues(numberOfShapes * valuesPerShape);

    for (float& value : shapeValues) {
        inputFile >> value;
    }

    inputFile.close();

    switch (shapeChoice) {
    case 1:
        renderCircles(shapeValues, verticalExtentOfGrid, horizontalExtentOfGrid, numberOfShapes);
        break;
    case 2:
        renderTriangles(shapeValues, verticalExtentOfGrid, horizontalExtentOfGrid, numberOfShapes);
        break;
    case 3:
        renderSquares(shapeValues, verticalExtentOfGrid, horizontalExtentOfGrid, numberOfShapes);
        break;
    default:
        cout << "Invalid choice!" << endl;
    }

    return 0;
}
