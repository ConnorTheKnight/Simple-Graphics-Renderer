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
void CircleJContainsI(int i, int j){
    if(i==j){
        return;
    }
    /*float XA = position.at(2*i);                                                       //Store information about Circle A
    float YA = position.at((2*i)+1);
                
    float XB = position.at(2*j);                                                       //Store information about Circle B
    float YB = position.at((2*j)+1);
    float radiusA = infoForShape.at(i);
    float radiusB = infoForShape.at(j);
    float deltaX = (position.at(2*j) - position.at(2*i));
    float deltaY = (position.at((2*j)+1) - position.at((2*i)+1));*/
    if((int)((position.at(2*j) - position.at(2*i))*(position.at(2*j) - position.at(2*i)))<=0&&(int)((position.at((2*j)+1) - position.at((2*i)+1))*(position.at((2*j)+1) - position.at((2*i)+1)))<=0&&(int)((infoForShape.at(i)-infoForShape.at(j))*(infoForShape.at(i)-infoForShape.at(j)))<=0){
        if(i<j){
        cull[i].store(true);
        }
        return;
    }
    if(sqrt(((position.at(2*j) - position.at(2*i))*(position.at(2*j) - position.at(2*i)))+((position.at((2*j)+1) - position.at((2*i)+1))*(position.at((2*j)+1) - position.at((2*i)+1))))<infoForShape.at(j)+infoForShape.at(i)&&infoForShape.at(i)<infoForShape.at(j)){//if Circle A is covered by circle B if the magnitude of the x and y distance of the center of the inner circle from the center of the outer circle  
        cull[i].store(true);                                                            //Dont draw Circle A (Circle A is culled)
    }
}
void sequentialCullingAlgorithmJRange(int i, int j, int k){
    for(int index = j; index < k; index++){//for each circle j
        if(cull[i].load()){//if other worker thread has already evaluated circle i to be culled
            return;
        }
        if(cull[index].load()){//if j has been culled
            continue;
        }
        CircleJContainsI(i,index);
    }
}
void sequentialCullingAlgorithmIRange(int i, int k, int maxJ){
    for(int index1 = i; index1 < k; index1++){//for each circle i
        for(int index2 = 0; index2 < maxJ; index2++){
            if(cull[index2].load()){//if j has been culled
                continue;
            }
            CircleJContainsI(index1,index2);
        }
    }
}
void distributeWorkCullingAlgorithmForI(int i, int maxJ,int threadsAllocated){
    if(maxJ<threadsAllocated){//if there are enough threads for each J value to get its own thread
        vector<thread> threads;
        threads.reserve(maxJ);
        for(int index = 0; index < maxJ; index++){
            threads.emplace_back(CircleJContainsI, i, index);
        }
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }else{//otherwise give each thread a range of J values to solve sequentially
        int JperThread = maxJ/(threadsAllocated+1);//lower bound for the number of J values each thread must evaluate
        int excessJValues = maxJ%(threadsAllocated+1);//number of J values which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(threadsAllocated);
        int currJ = 0;
        for(int index = 0; index < threadsAllocated; index++){
            if(excessJValues>0){
                threads.emplace_back(sequentialCullingAlgorithmJRange, i, currJ, currJ+JperThread+1);
                currJ += JperThread+1;
                excessJValues--;
            }else{
                threads.emplace_back(sequentialCullingAlgorithmJRange, i, currJ, currJ+JperThread);
                currJ += JperThread;
            }         
        }
        sequentialCullingAlgorithmJRange(i,currJ,maxJ);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }
}
void distributeWorkCullingAlgorithm(int maxI, int maxJ,int threadsAllocated){
    if(maxI<threadsAllocated){//if there are enough threads for each I value to get its own thread
        int threadsPerI = (threadsAllocated+1)/maxI;//lower bound for the number of threads that can be allocated for each I value
        int excessThreads = (threadsAllocated+1)%maxI;//number of threads which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(maxI);
        for(int index = 0; index < maxI; index++){
            if(excessThreads>0){
                threads.emplace_back(distributeWorkCullingAlgorithmForI,index,maxJ,threadsPerI);
            }
            threads.emplace_back(distributeWorkCullingAlgorithmForI,index,maxJ,threadsPerI-1);
        }
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }else{//otherwise give each thread a range of I values to solve sequentially
        int IperThread = maxI/(threadsAllocated+1);//lower bound for the number of I values each thread must evaluate
        int excessIValues = maxI%(threadsAllocated+1);//number of I values which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(threadsAllocated);
        int currI = 0;
        for(int index = 0; index < threadsAllocated; index++){
            if(excessIValues>0){
                threads.emplace_back(sequentialCullingAlgorithmIRange, currI, currI+IperThread+1, maxJ);
                currI += IperThread+1;
                excessIValues--;
            }else{
                threads.emplace_back(sequentialCullingAlgorithmIRange, currI, currI+IperThread, maxJ);
                currI += IperThread;
            }         
        }
        sequentialCullingAlgorithmIRange(currI,maxI,maxJ);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }
}
void drawXY(int i, int X, int Y){
    float deltaX = position.at(2*i) - X;
    float deltaY = position.at((2*i)+1) - Y;
    if(sqrt((deltaX*deltaX)+(deltaY*deltaY))<infoForShape.at(i)){
        isFilled[Y][X].store(true);        //this grid unit is filled if the magnitude of the x and y displacement from the center of the circle is less than the radius of the circle
    }
}
void sequentialDrawingAlgorithmYRange(int i, int X, int Y, int k){
    for(int index = Y; index < k; index++){//for each circle j
        drawXY(i,X,index);
    }
}
void sequentialDrawingAlgorithmXRange(int i, int X, int k, int Y, int deltaY){
    for(int index1 = X; index1 < k; index1++){//for each circle i
        for(int index2 = 0; index2 < deltaY; index2++){
            drawXY(i,index1,Y+index2);
        }
    }
}
void distributeWorkDrawingAlgorithmForX(int i, int X, int Y, int Ydelta,int threadsAllocated){
    if(Ydelta<(threadsAllocated+1)){//if there are enough threads for each J value to get its own thread
        vector<thread> threads;
        threads.reserve(Ydelta);
        for(int index = 0; index < Ydelta-1; index++){
            threads.emplace_back(drawXY, i, X, Y+index);
        }
        drawXY(i,X,Ydelta-1);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }else{//otherwise give each thread a range of J values to solve sequentially
        int YperThread = Ydelta/(threadsAllocated+1);//lower bound for the number of J values each thread must evaluate
        int excessYValues = Ydelta%(threadsAllocated+1);//number of J values which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(threadsAllocated);
        int currY = Y;
        for(int index = 0; index < threadsAllocated; index++){
            if(excessYValues>0){
                threads.emplace_back(sequentialDrawingAlgorithmYRange, i, X, currY, currY+YperThread+1);
                currY += YperThread+1;
                excessYValues--;
            }else{
                threads.emplace_back(sequentialDrawingAlgorithmYRange, i, X, currY, currY+YperThread);
                currY += YperThread;
            }         
        }
        sequentialDrawingAlgorithmYRange(i, X,currY,Y+Ydelta);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }
}
void distributeWorkDrawingAlgorithm(int i, int X, int Y, int Xdelta, int Ydelta, int threadsAllocated){
    if(Xdelta<(threadsAllocated+1)){//if there are enough threads for each I value to get its own thread
        int threadsPerX = (threadsAllocated+1)/Xdelta;//lower bound for the number of threads that can be allocated for each I value
        int excessThreads = (threadsAllocated+1)%Xdelta;//number of threads which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(Xdelta);
        for(int index = 0; index < Xdelta-1; index++){
            if(excessThreads>0){
                threads.emplace_back(distributeWorkDrawingAlgorithmForX, i, X+index, Y, Ydelta,threadsPerX);
            }
            threads.emplace_back(distributeWorkDrawingAlgorithmForX, i, X+index, Y, Ydelta,threadsPerX-1);
        }
        distributeWorkDrawingAlgorithmForX(i,Xdelta-1,Y,Ydelta,threadsPerX-1);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }else{//otherwise give each thread a range of I values to solve sequentially
        int XperThread = Xdelta/(threadsAllocated+1);//lower bound for the number of I values each thread must evaluate
        int excessXValues = Xdelta%(threadsAllocated+1);//number of I values which cant be evenly distributed
        vector<thread> threads;
        threads.reserve(threadsAllocated);
        int currX = X;
        for(int index = 0; index < threadsAllocated; index++){
            if(excessXValues>0){
                threads.emplace_back(sequentialDrawingAlgorithmXRange, i, currX, currX+XperThread+1, Y, Ydelta);
                currX += XperThread+1;
                excessXValues--;
            }else{
                threads.emplace_back(sequentialDrawingAlgorithmXRange, i, currX, currX+XperThread, Y, Ydelta);
                currX += XperThread;
            }         
        }
        sequentialDrawingAlgorithmXRange(i, currX, X+Xdelta, Y, Ydelta);
        for(auto& t: threads){//wait for all worker threads to finish
            t.join();
        }
    }
}

int main()
{
    //-----Declare Variables-----   
    int verticalExtentOfGrid = 0;           //Store number of vertical grid units (number of rows)
    int horizontalExtentOfGrid = 0;         //Store number of horizontal grid units (number of columns) 
    int numberOfShapesToRender = 0;         //Store number of shapes to be provided for rendering
    int numberThreads = 0;                  //Store number of threads to use in this algorithm
    
    
    //-----Read User Input-----
    //Read in number of vertical grid units from stdin
    cin >> verticalExtentOfGrid;
    //Read in number of horizontal grid units from stdin
    cin >> horizontalExtentOfGrid;
    //Initialize isFilled Array
    isFilled = new atomic<bool>*[verticalExtentOfGrid];
    for(int i = 0; i < verticalExtentOfGrid; i++){
        isFilled[i] = new atomic<bool>[horizontalExtentOfGrid];
    }
    //Read in number of shapes to render from stdin
    cin >> numberOfShapesToRender;
    //Initialize Info Array (Note that for future 2-D shapes more than one piece of information per shape may be needed)
    infoForShape.resize(numberOfShapesToRender);
    //Initialize Positon Array
    position.resize(numberOfShapesToRender*2);
    //Initialize Cull Array
    cull = new atomic<bool>[numberOfShapesToRender];
    //Read in number of threads to use from stdin
    cin >> numberThreads;
    //Read in information on dimensions of each shape (radius of each circle) followed by its X, Y, and Z coordinates from stdin
    for(int i = 0; i < numberOfShapesToRender; i++){
        cin >> infoForShape.at(i);     //Read in information on dimensions (radius)
        cin >> position.at(2*i);       //Read in X coordinate
        cin >> position.at((2*i)+1);   //Read in Y coordinate
    }
    //-----Begin Processing-----
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
        distributeWorkCullingAlgorithm(numberOfShapesToRender,numberOfShapesToRender,numberThreads);

        //All remaining false values in the cull Array represent circles that are at least in part uncovered (need to be drawn)

        //-----Fairness Algorithm-----
        /*In order to ensure the workload is evenly distributed among all threads,
        extra calculations need to be made to determine the number of pixels which need to be evaluated, these calculations will scale
        to O(number of shapes).*/
        float maxCalculations = 0;
        int numShapes = 0;
        vector<int> XYsideLengthXsideLengthY;
        for(int i = 0; i < numberOfShapesToRender; i++){
            if(cull[i].load()){
                continue;
            }
            int maxX = position.at(2*i) + infoForShape.at(i) + 1;
            int minX = position.at(2*i) - infoForShape.at(i) - 1;
            int maxY = position.at((2*i)+1) + infoForShape.at(i) + 1;
            int minY = position.at((2*i)+1) - infoForShape.at(i) - 1;
            if(minX<0){
                minX = 0;
            }
            if(minY<0){
                minY = 0;
            }
            if(maxX>horizontalExtentOfGrid){
                maxX = horizontalExtentOfGrid;
            }
            if(maxY>horizontalExtentOfGrid){
                maxY = horizontalExtentOfGrid;
            }
            int sideLengthX = maxX-minX;
            int sideLengthY = maxY-minY;
            XYsideLengthXsideLengthY.emplace_back(minX);
            XYsideLengthXsideLengthY.emplace_back(minY);
            XYsideLengthXsideLengthY.emplace_back(sideLengthX);
            XYsideLengthXsideLengthY.emplace_back(sideLengthY);
            maxCalculations += sideLengthX*sideLengthY;
            numShapes++;
        }
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
        if(numberThreads>numShapes){//if at least 1 thread for each shape to render
            vector<thread> threads;
            int index = 0;
            for(int i = 0; i < numberOfShapesToRender; i++){
                if(cull[i].load()){
                    continue;
                }
                int boundX = XYsideLengthXsideLengthY.at(index++);
                int boundY = XYsideLengthXsideLengthY.at(index++);
                int sideLengthX = XYsideLengthXsideLengthY.at(index++);
                int sideLengthY = XYsideLengthXsideLengthY.at(index++);
                int numCalculations = sideLengthX*sideLengthY;
                float fractionThreads = (numCalculations*1.0)/maxCalculations;
                threads.emplace_back(distributeWorkDrawingAlgorithm, i, boundX, boundY, sideLengthX, sideLengthY, ((int)(fractionThreads*numberThreads)));
            }
            for(auto& t: threads){//wait for all worker threads to finish
                t.join();
            }
        }else{
            vector<thread> threads;
            int index = 0;
            int temp = 0;
            for(int i = 0; i < numberOfShapesToRender&&index<numberThreads; i++){
                if(cull[i].load()){
                    continue;
                }
                int boundX = XYsideLengthXsideLengthY.at(index++);
                int boundY = XYsideLengthXsideLengthY.at(index++);
                int sideLengthX = XYsideLengthXsideLengthY.at(index++);
                int sideLengthY = XYsideLengthXsideLengthY.at(index++);
                threads.emplace_back(distributeWorkDrawingAlgorithm, i, boundX, boundY, sideLengthX, sideLengthY, 0);
                temp = i+1;
            }
            int head = 0;
            for(int i = temp; i < numberOfShapesToRender; i++){
                if(cull[i].load()){
                    continue;
                }
                int boundX = XYsideLengthXsideLengthY.at(index++);
                int boundY = XYsideLengthXsideLengthY.at(index++);
                int sideLengthX = XYsideLengthXsideLengthY.at(index++);
                int sideLengthY = XYsideLengthXsideLengthY.at(index++);
                threads.at(head).join();
                threads.erase(std::next(threads.begin(),head));
                threads.emplace(std::next(threads.begin(),head++),distributeWorkDrawingAlgorithm, i, boundX, boundY, sideLengthX, sideLengthY, 0);
            }
            for(auto& t: threads){//wait for all worker threads to finish
                t.join();
            }
        }
        
    //-----End Processing-----
    
    //-----Output Results-----
    /*All the data needed for the output is stored in the isFilled array, for the sake of visualization this array will be printed to stdout in O(n*m) time*/
    for(int Y = 0; Y < verticalExtentOfGrid; Y++){          //for each unit of the grid
        for(int X = 0; X < horizontalExtentOfGrid; X++){
            if((isFilled[Y])[X]){                             //if the gridUnit is filled
                cout << "[X]";                              //print X in the cell
            }else{                                          //else
                cout << "[_]";                              //print _ in the cell
            }
        }
        cout << "\n";
    }


    delete[] cull;
    for(int i = 0; i < verticalExtentOfGrid; i++){
        delete[] isFilled[i];
    }
    delete[] isFilled;

    return 0;
}

