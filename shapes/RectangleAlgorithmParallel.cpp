#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <math.h>

using namespace std;

vector<float> infoForShape;           //Store information on the size of each shape (for now only squares are being considered and thus only 1 sidelength per square is needed)
vector<float> position;               //Store X, Y, and Z coordinate of a known point of this shape (for now that is the bottom left corner of the square [Note: Bottom = minimum Y value, Left = minimum X value])
atomic<bool>* cull;                      //Store whether or not to draw a given shape (output of culling algorithm)
atomic<bool>** isFilled;          //Store whether or not a gridUnit is occupied by a shape (output of draw algorithm)

atomic<int> numberOfUnculledShapes;

int verticalExtentOfGrid;
int horizontalExtentOfGrid;
void sequentialCullingAlgorithmIRange(int i, int k, int maxJ) {
    for (int index1 = i; index1 < k; index1++) {//for each square i
        for (int index2 = 0; index2 < maxJ && !cull[index1].load(memory_order_relaxed); index2++) {
            if (cull[index2].load(memory_order_relaxed)) {//if j has been culled
                continue;
            }
            //---
            if (index1 == index2) {
                continue;
            }
            float XA = position.at(2*index1);                                                       //Store information about Square A
            float YA = position.at((2*index1)+1);
            float widthA = infoForShape.at(2*index1);
            float heightA = infoForShape.at(2*index1+1);
                
            float XB = position.at(2*index2);                                                       //Store information about Square B
            float YB = position.at((2*index2)+1);
            float widthB = infoForShape.at(2*index2);
            float heightB = infoForShape.at(2*index2+1);
            if ((XA>XB)&&(YA>YB)&&(XA+widthA<XB+widthB)&&(YA+heightA<YB+heightB)) {
                if (position.at(2 * index1) == position.at(2 * index2) && position.at(2 * index1 + 1) == position.at(2 * index2 + 1) && infoForShape.at(index1*2) == infoForShape.at(index2*2) && infoForShape.at(index1*2+1) == infoForShape.at(index2*2+1)) {
                    if (index1 < index2) {
                        cull[index1].store(true, memory_order_relaxed);
                        numberOfUnculledShapes--;
                    }
                    continue;
                }
                cull[index1].store(true, memory_order_relaxed);                                                            //Dont draw Circle A (Circle A is culled)
                numberOfUnculledShapes--;
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
        int minX = (int) position.at(2*i);                         //get bounds of Shape in terms of grid units (integers)
        int minY = (int) position.at((2*i)+1);
        int maxX = (int) (position.at(2*i)+infoForShape.at(2*i)+1);
        int maxY = (int) (position.at((2*i)+1)+infoForShape.at(2*i+1)+1);
        if(minX<0){
            minX = 0;
        }
        if(minY<0){
            minY = 0;
        }
        if(maxX>horizontalExtentOfGrid){
            maxX = horizontalExtentOfGrid;
        }
        if(maxY>verticalExtentOfGrid){
            maxY = verticalExtentOfGrid;
        }
        for(int X = minX; X < maxX; X++){                          //for each grid unit in bounds
            for(int Y = minY; Y < maxY; Y++){
                isFilled[Y][X].store(true, memory_order_relaxed);        //this grid unit is filled
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
    infoForShape.resize(numberOfShapesToRender*2);
    //Initialize Positon Array
    position.resize(numberOfShapesToRender * 2);
    //Initialize Cull Array
    cull = new atomic<bool>[numberOfShapesToRender] {};
    //Read in number of threads to use from stdin
    cin >> numberThreads;
    //Read in information on dimensions of each shape (radius of each circle) followed by its X, Y, and Z coordinates from stdin
    for(int i = 0; i < numberOfShapesToRender; i++){
        cin >> infoForShape.at(2*i);     //Read in information on dimensions (width)
        cin >> infoForShape.at(2*i+1);   //Read in information on dimensions (height)
        cin >> position.at(2*i);       //Read in X coordinate
        cin >> position.at((2*i)+1);   //Read in Y coordinate
    }


    //-----Begin Processing-----
    numberOfUnculledShapes.store(numberOfShapesToRender, memory_order_relaxed);
    //-----Culling Algorithim-----
    /*A simple Culling Algorithm for determining whether Square A is covered by square B is:
        if((XA>XB)&&(YA>YB)&&(ZA<ZB)&&(XA+lengthA<XB+lengthB)&&(YA+lengthA<YB+lengthB)){
            square A is covered
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

    //All remaining false values in the cull Array represent squares that are at least in part uncovered (need to be drawn)

    //-----Drawing Algorithim-----
    /*A simple drawing Algorithm for determining which gridUnits are filled by Square I:
        for SquareI
            minX = (int) XI
            minY = (int) YI
            maxX = (int) (XI+length+1)              //Technically wrong on the edge case that XI has no fractional value
            maxY = (int) (YI+length+1)              //Technically wrong on the edge case that YI has no fractional value
            for(int X = minX; X < maxX; X++){
                for(int Y = minY; Y < maxY; Y++){
                    this grid unit [X,Y] is filled
                }
            }
        This implementation will run the above algorithim on each square in O(N*(maxLength^2)) time 
        [Note: this might be able to be sped up by using a Data structure to avoid running a check on every single square]
        */
    vector<thread> threads;
    int shapesPerThread = numberOfUnculledShapes.load(memory_order_relaxed);
    int excessShapes = shapesPerThread % (numberThreads + 1);
    shapesPerThread /= (numberThreads + 1);
    int index = 0;
    for (int i = 0; i < numberThreads; i++) {
        if (excessShapes > 0) {
            threads.emplace_back(sequentialDrawingAlgorithmIRange, index, shapesPerThread + 1, numberOfShapesToRender);
            excessShapes--;
            int numUnculled = 0;
            while (numUnculled < shapesPerThread+1 && index < numberOfShapesToRender) {
                if (!cull[index++].load(memory_order_relaxed)) {
                    numUnculled++;
                }
            }
        }
        else {
            threads.emplace_back(sequentialDrawingAlgorithmIRange, index, shapesPerThread, numberOfShapesToRender);
            int numUnculled = 0;
            while (numUnculled < shapesPerThread && index < numberOfShapesToRender) {
                if (!cull[index++].load(memory_order_relaxed)) {
                    numUnculled++;
                }
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