#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    srand(time(0));

    int inputFileNum = 8;
    int numberOfThreads = 1;

    int numberOfShapes = 1000;
    int minX = 0;
    int maxX = 30;
    int minY = 0;
    int maxY = 30;
    int minSize = 0;
    int maxSize = 5;

    cin >> inputFileNum;
    cin >> numberOfThreads;
    cin >> numberOfShapes;
    cin >> minX;
    cin >> maxX;
    cin >> minY;
    cin >> maxY;
    cin >> minSize;
    cin >> maxSize;

    ofstream inputFile("inputs/Input"+to_string(inputFileNum)+".txt");
    if (!inputFile) {
        cerr << "Error: Unable to open file ";
        inputFile.close();
        return 1;
    }

    int deltaX = maxX-minX;
    int deltaY = maxY-minY;
    int deltaS = maxSize-minSize;
    inputFile << deltaX << "\n" << deltaY << "\n" << numberOfShapes << "\n" << numberOfThreads << "\n";
    for(int i = 0; i < numberOfShapes; i++){
        inputFile << (rand()%(deltaX)+minX+((rand()*1.0)/RAND_MAX)) << "\n" << (rand()%(deltaY)+minY+((rand()*1.0)/RAND_MAX)) << "\n" << (rand()%(deltaS)+minSize+((rand()*1.0)/RAND_MAX)) << "\n";
    }
}