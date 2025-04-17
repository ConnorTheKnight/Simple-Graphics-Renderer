#include "engine.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

int main()
{
    // call my circle algorthm file
    //-----Declare Variables-----
    int verticalExtentOfGrid = 0;           //Store number of vertical grid units (number of rows)
    int horizontalExtentOfGrid = 0;         //Store number of horizontal grid units (number of columns) 
    int numberOfShapesToRender = 0;         //Store number of shapes to be provided for rendering
    vector<float> infoForShape;             //Store information on the size of each shape (for now only circles are being considered and thus only radius is needed)
    vector<float> position;                 //Store X, Y, and Z coordinate of a known point of this shape (for now that is the bottom left corner of the circle [Note: Bottom = minimum Y value, Left = minimum X value])
    bool* cull;                      //Store whether or not to draw a given shape (output of culling algorithm)
    bool** isFilled;          //Store whether or not a gridUnit is occupied by a shape (output of draw algorithm)


    //-----Read User Input from "input.txt"-----
    std::ifstream in("C:\\Users\\Gavin Cruz\\Documents\\ParDisSys\\Final-Project\\Simple-Graphics-Renderer\\src\\input.txt");
    if (!in) {
        std::cerr << "Failed to open input.txt\n";
        return 1;  // or handle error appropriately
    }

    // Read grid dimensions
    in >> verticalExtentOfGrid;
    in >> horizontalExtentOfGrid;

    // Initialize isFilled Array (all false by default)
    isFilled = new bool*[verticalExtentOfGrid];
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        isFilled[i] = new bool[horizontalExtentOfGrid];
        std::fill_n(isFilled[i], horizontalExtentOfGrid, false);
    }

    // Read number of shapes
    in >> numberOfShapesToRender;

    // Initialize Info and Position arrays
    infoForShape.resize(numberOfShapesToRender);
    position.resize(numberOfShapesToRender * 2);

    // Initialize Cull Array (all false by default)
    cull = new bool[numberOfShapesToRender];
    std::fill_n(cull, numberOfShapesToRender, false);

    // Read (and ignore) thread count for compatibility
    int unusedThreads;
    in >> unusedThreads;

    // Read each shape’s radius and X,Y coordinates
    for (int i = 0; i < numberOfShapesToRender; i++) {
        in >> infoForShape[i];          // radius
        in >> position[2*i];            // X
        in >> position[2*i + 1];        // Y
    }
    in.close();


    //-----Begin Processing-----
    /* Note that this is the part of the algorithm which will need concurrency to be implemneted efficiently*/
    //-----Culling Algorithim-----
    /*A simple Culling Algorithm for determining whether Circle A is covered by Circle B is:
    if(sqrt((XB-XA)^2 + (YB-YA)^2)<radiusB){
        circle A is covered
    }
    
    This implementation will compare each pair of circles using that algorithim in O(N^2) time 
    [Note: this might be able to be sped up by using an O(NLog(N)) sort to limit which circles need to be compared by Z value]
    [Another potential optimization is to store nearby shapes in some kind of buffer so that shapes very far apart do not need to be compared]
    */
    for(int i = 0; i < numberOfShapesToRender; i++){                                           //for each Circle A
        for(int j = 0; j < numberOfShapesToRender&&!cull[i]; j++){                          //for each Circle B (Skip further comparisons if Circle A has already been covered)
            if(cull[j]){                                                                       //if Circle B is covered by another Circle C
                continue;                                                                      //Dont bother comparing against Circle A as if Circle B covers Circle A then Circle C will cover Circle A
            }
            if(i==j){ 
                continue; 
            }
            float XA = position[2*i];                                                       //Store information about Circle A
            float YA = position[(2*i)+1];
            float lengthA = infoForShape[i];
            
            float XB = position[2*j];                                                       //Store information about Circle B
            float YB = position[(2*j)+1];
            float lengthB = infoForShape[j];
            float deltaX = XB - XA;
            float deltaY = YB - YA;
            if(sqrt((deltaX*deltaX)+(deltaY*deltaY))<infoForShape[i]+infoForShape[j]&&infoForShape[i]<infoForShape[j]){                //if Circle A is covered by circle B
                cull[i] = true;                                                            //Dont draw Circle A (Circle A is culled)
            }
        }
    }                                                                                       
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
                if(Math.sqrt((deltaX*deltaX)+(deltaY*deltaY))<radiusI){
                    this grid unit is filled
                }
            }
        }
    This implementation will run the above algorithim on each circle in O(N*(maxLength^2)) time 
    [Note: this might be able to be sped up by using a Data structure to avoid running a check on every single circle]
    */
    for(int i = 0; i < numberOfShapesToRender; i++){
        if(cull[i]){                                                                                //if shape has been culled by culling algorithim
            continue;                                                                                  //Do not evaluate
        }
        int minX = (int) position[2*i] - infoForShape[i];                                        //get bounds of Shape in terms of grid units (integers)
        int minY = (int) position[(2*i)+1] - infoForShape[i];
        int maxX = (int) position[2*i]+infoForShape[i]+1;
        int maxY = (int) position[(2*i)+1]+infoForShape[i]+1;
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
        for(int X = minX; X < maxX; X++){                                                                 //for each grid unit in bounds
            float deltaX = position[2*i] - X;
            for(int Y = minY; Y < maxY; Y++){
                float deltaY = position[(2*i)+1] - Y;
                if(sqrt((deltaX*deltaX)+(deltaY*deltaY))<infoForShape[i]){
                    isFilled[Y][X] = true;   //this grid unit is filled if Math.sqrt((deltaX*deltaX)+(deltaY*deltaY))<radiusI
                }
            }
        }
    }

    // draw to screen
    Engine engine(isFilled, horizontalExtentOfGrid, verticalExtentOfGrid, "Parallel Circle Rendering Project");

    if(!engine.Initialize())
    {
        std::cout << std::endl << "Press any key to close program..." << std::endl;
        std::cin.get();
    }

    return 0;
}