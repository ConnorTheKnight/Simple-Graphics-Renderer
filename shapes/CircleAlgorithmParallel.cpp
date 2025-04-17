#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <math.h>

using namespace std;

vector<float> infoForShape;             //Store information on the size of each shape (for now only circles are being considered and thus only radius is needed)
vector<float> position;                 //Store X, Y, and Z coordinate of a known point of this shape (for now that is the bottom left corner of the circle [Note: Bottom = minimum Y value, Left = minimum X value])
atomic<bool>* cull;                      //Store whether or not to draw a given shape (output of culling algorithm)
atomic<bool>** isFilled;          //Store whether or not a gridUnit is occupied by a shape (output of draw algorithm)

atomic<int> shapesToDraw;

int verticalExtentOfGrid;
int horizontalExtentOfGrid;
void sequentialCullingAlgorithmIRange(int i, int k, int maxJ) {
    for (int index1 = i; index1 < k; index1++) {//for each circle i
        for (int index2 = 0; index2 < maxJ && !cull[index1].load(memory_order_relaxed); index2++) {
            if (cull[index2].load(memory_order_relaxed)) {//if j has been culled
                continue;
            }
            //---
            /*float XA = position.at(2*index1);                                                       //Store information about Circle A
            float YA = position.at((2*index1)+1);

            float XB = position.at(2*index2);                                                       //Store information about Circle B
            float YB = position.at((2*index2)+1);
            float radiusA = infoForShape.at(index1);
            float radiusB = infoForShape.at(index2);
            float deltaX = (position.at(2*index2) - position.at(2*index1));
            float deltaY = (position.at((2*index2)+1) - position.at((2*index1)+1));*/
            if (index1 == index2) {
                continue;
            }
            if (sqrt(((position.at(2 * index2) - position.at(2 * index1)) * (position.at(2 * index2) - position.at(2 * index1))) + ((position.at((2 * index2) + 1) - position.at((2 * index1) + 1)) * (position.at((2 * index2) + 1) - position.at((2 * index1) + 1)))) + infoForShape.at(index1) < infoForShape.at(index2) && infoForShape.at(index1) < infoForShape.at(index2)) {//if Circle A is covered by circle B if the magnitude of the x and y distance of the center of the inner circle from the center of the outer circle  
                if (position.at(2 * index1) == position.at(2 * index2) && position.at(2 * index1 + 1) == position.at(2 * index2 + 1) && infoForShape.at(index1) == infoForShape.at(index2)) {
                    if (index1 < index2) {
                        cull[index1].store(true, memory_order_relaxed);
                        shapesToDraw--;
                    }
                    continue;
                }
                cull[index1].store(true, memory_order_relaxed);                                                            //Dont draw Circle A (Circle A is culled)
                shapesToDraw--;
                continue;
            }
            //---
        }
    }
}
void distributeWorkCullingAlgorithm(int maxI, int maxJ, int threadsAllocated) {
    int IperThread = maxI / (threadsAllocated + 1);//lower bound for the number of I values each thread must evaluate
    int excessIValues = maxI % (threadsAllocated + 1);//number of I values which cant be evenly distributed
    vector<thread> threads;
    int currI = 0;
    for (int index = 0; index < threadsAllocated; index++) {
        if (excessIValues > 0) {
            threads.emplace_back(sequentialCullingAlgorithmIRange, currI, currI + IperThread + 1, maxJ);
            currI += IperThread + 1;
            excessIValues--;
        }
        else {
            threads.emplace_back(sequentialCullingAlgorithmIRange, currI, currI + IperThread, maxJ);
            currI += IperThread;
        }
    }
    sequentialCullingAlgorithmIRange(currI, maxI, maxJ);
    for (auto& t : threads) {//wait for all worker threads to finish
        t.join();
    }
}
void sequentialDrawingAlgorithmIRange(int i, int k, int maxI) {
    int shapesDrawn = 0;
    for (int index0 = i; shapesDrawn < k && index0 < maxI; index0++) {//for each circle i
        if (cull[index0].load(memory_order_relaxed)) {
            continue;
        }
        shapesDrawn++;
        int boundX = position.at((index0 * 2)) - infoForShape.at(index0);
        int boundY = position.at((index0 * 2) + 1) - infoForShape.at(index0);
        int sideLengthX = (infoForShape.at(index0) * 2) + 1;
        int sideLengthY = (infoForShape.at(index0) * 2) + 1;
        if (boundX < 0) {
            boundX = 0;
        }
        if (boundY < 0) {
            boundY = 0;
        }
        if (boundX + sideLengthX >= horizontalExtentOfGrid) {
            sideLengthX = horizontalExtentOfGrid - boundX;
        }
        if (boundY + sideLengthY >= verticalExtentOfGrid) {
            sideLengthY = verticalExtentOfGrid - boundY;
        }
        for (int index1 = boundX; index1 < boundX + sideLengthX; index1++) {
            for (int index2 = boundY; index2 < boundY + sideLengthY; index2++) {
                float deltaX = position.at(2 * index0) - index1;
                float deltaY = position.at((2 * index0) + 1) - index2;
                if (sqrt((deltaX * deltaX) + (deltaY * deltaY)) < infoForShape.at(index0)) {
                    isFilled[index2][index1].store(true, memory_order_relaxed);        //this grid unit is filled if the magnitude of the x and y displacement from the center of the circle is less than the radius of the circle
                }
            }
        }
    }
}


int main()
{
    //-----Declare Variables-----   
    int numberOfShapesToRender = 0;         //Store number of shapes to be provided for rendering
    int numberThreads = 0;                  //Store number of threads to use in this algorithm


    //-----Read User Input-----
    //Read in number of vertical grid units from stdin
    cin >> verticalExtentOfGrid;//Store number of vertical grid units (number of rows)
    //Read in number of horizontal grid units from stdin
    cin >> horizontalExtentOfGrid;//Store number of horizontal grid units (number of columns) 
    //Initialize isFilled Array
    isFilled = new atomic<bool>*[verticalExtentOfGrid];
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        isFilled[i] = new atomic<bool>[horizontalExtentOfGrid];
    }
    //Read in number of shapes to render from stdin
    cin >> numberOfShapesToRender;
    //Initialize Info Array (Note that for future 2-D shapes more than one piece of information per shape may be needed)
    infoForShape.resize(numberOfShapesToRender);
    //Initialize Positon Array
    position.resize(numberOfShapesToRender * 2);
    //Initialize Cull Array
    cull = new atomic<bool>[numberOfShapesToRender] {};
    //Read in number of threads to use from stdin
    cin >> numberThreads;
    //Read in information on dimensions of each shape (radius of each circle) followed by its X, Y, and Z coordinates from stdin
    for (int i = 0; i < numberOfShapesToRender; i++) {
        cin >> infoForShape.at(i);     //Read in information on dimensions (radius)
        cin >> position.at(2 * i);       //Read in X coordinate
        cin >> position.at((2 * i) + 1);   //Read in Y coordinate
    }


    //-----Begin Processing-----
    shapesToDraw.store(numberOfShapesToRender, memory_order_relaxed);
    //-----Culling Algorithim-----
    /*A simple Culling Algorithm for determining whether Circle A is covered by Circle B is:
    if(sqrt((XB-XA)^2 + (YB-YA)^2)<radiusB){
        circle A is covered
    }

    This implementation will uses threads to simultaneously compare each pair of circles using that algorithim in (O(N)/numThreads)*O(N) time
    [Note: this might be able to be sped up by using an O(NLog(N)) sort to limit which circles need to be compared by Z value]
    [Another potential optimization is to store nearby shapes in some kind of buffer so that shapes very far apart do not need to be compared]
    */
    /*Potential performance optimization, thre current implementation is going to have a very large amount of overhead when the number of threads available is
    less than the number of shapes to render, this is because every time a thread finishes calculations for a shape, it is destroyed and a new thread is created
    to handle the next shape. This performance overhead could be greatly diminished if the threads were reused, however it would likely involve the management of
    a concurrent queue or other shared data structure.
    */
    distributeWorkCullingAlgorithm(numberOfShapesToRender, numberOfShapesToRender, numberThreads);

    //All remaining false values in the cull Array represent circles that are at least in part uncovered (need to be drawn)

    //-----Drawing Algorithim-----
    /*A simple drawing Algorithm for determining which gridUnits are filled by Circle I:
    for CircleI
        minX = (int) XI-radiusI
        minY = (int) YI-radiusI
        maxX = (int) (XI+radiusI+1)              //Technically wrong on the edge case that XI has no fractional value
        maxY = (int) (YI+radiusI+1)              //Technically wrong on the edge case that YI has no fractional value
        for(int X = minX; X < maxX; X++){
            float deltaX = XI-X
            for(int Y = minY; Y < maxY; Y++){
                float deltaY = YI-Y
                if(sqrt((deltaX*deltaX)+(deltaY*deltaY))<radiusI){
                    this grid unit is filled
                }
            }
        }
    This implementation will run the above algorithim on each circle in O(N*(maxLength^2)) time
    [Note: this might be able to be sped up by using a Data structure to avoid running a check on every single circle]
    */
    vector<thread> threads;
    int shapesPerThread = shapesToDraw.load(memory_order_relaxed);
    int excessShapes = shapesPerThread % (numberThreads + 1);
    shapesPerThread /= (numberThreads + 1);
    int index = 0;
    for (int i = 0; i < numberThreads; i++) {
        if (excessShapes > 0) {
            threads.emplace_back(sequentialDrawingAlgorithmIRange, index, shapesPerThread + 1, numberOfShapesToRender);
            excessShapes--;
        }
        else {
            threads.emplace_back(sequentialDrawingAlgorithmIRange, index, shapesPerThread, numberOfShapesToRender);
        }
        int numUnculled = 0;
        while (numUnculled < shapesPerThread && index < numberOfShapesToRender) {
            if (!cull[index++].load(memory_order_relaxed)) {
                numUnculled++;
            }
        }
    }
    sequentialDrawingAlgorithmIRange(index, numberOfShapesToRender, numberOfShapesToRender);
    for (auto& t : threads) {//wait for all worker threads to finish
        t.join();
    }

    //-----End Processing-----

    //-----Output Results-----
    /*All the data needed for the output is stored in the isFilled array, for the sake of visualization this array will be printed to stdout in O(n*m) time*/
    for (int Y = 0; Y < verticalExtentOfGrid; Y++) {          //for each unit of the grid
        for (int X = 0; X < horizontalExtentOfGrid; X++) {
            if ((isFilled[Y])[X].load(memory_order_relaxed)) {                             //if the gridUnit is filled
                cout << "[X]";                              //print X in the cell
            }
            else {                                          //else
                cout << "[_]";                              //print _ in the cell
            }
        }
        cout << "\n";
    }


    delete[] cull;
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        delete[] isFilled[i];
    }
    delete[] isFilled;

    return 0;
}