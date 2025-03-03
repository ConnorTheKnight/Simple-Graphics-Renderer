#include <iostream>
#include <vector>

using namespace std;

int main()
{
    //-----Declare Variables-----
    int verticalExtentOfGrid = 0;           //Store number of vertical grid units (number of rows)
    int horizontalExtentOfGrid = 0;         //Store number of horizontal grid units (number of columns) 
    int numberOfShapesToRender = 0;         //Store number of shapes to be provided for rendering
    vector<float> infoForShape;             //Store information on the size of each shape (for now only circles are being considered and thus only radius is needed)
    vector<float> position;                 //Store X, Y, and Z coordinate of a known point of this shape (for now that is the bottom left corner of the circle [Note: Bottom = minimum Y value, Left = minimum X value])
    vector<bool> cull;                      //Store whether or not to draw a given shape (output of culling algorithm)
    vector<vector<bool>> isFilled;          //Store whether or not a gridUnit is occupied by a shape (output of draw algorithm)
    
    
    //-----Read User Input-----
    //Read in number of vertical grid units from stdin
    cin >> verticalExtentOfGrid;
    //Read in number of horizontal grid units from stdin
    cin >> horizontalExtentOfGrid;
    //Initialize isFilled Array
    isFilled.assign(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));
    //Read in number of shapes to render from stdin
    cin >> numberOfShapesToRender;
    //Initialize Info Array (Note that for future 2-D shapes more than one piece of information per shape may be needed)
    infoForShape.resize(numberOfShapesToRender);
    //Initialize Positon Array
    position.resize(numberOfShapesToRender*3);
    //Initialize Cull Array
    cull.assign(numberOfShapesToRender, false);
    //Read in information on dimensions of each shape (radius of each circle) followed by its X, Y, and Z coordinates from stdin
    for(int i = 0; i < numberOfShapesToRender; i++){
        cin >> infoForShape.at(i);     //Read in information on dimensions (radius)
        cin >> position.at(3*i);       //Read in X coordinate
        cin >> position.at((3*i)+1);   //Read in Y coordinate
        cin >> position.at((3*i)+2);   //Read in Z coordinate
    }
    
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
            for(int j = 0; j < numberOfShapesToRender&&!cull.at(i); j++){                          //for each Circle B (Skip further comparisons if Circle A has already been covered)
                if(cull[j]){                                                                       //if Circle B is covered by another Circle C
                    continue;                                                                      //Dont bother comparing against Circle A as if Circle B covers Circle A then Circle C will cover Circle A
                }
                float XA = position.at(3*i);                                                       //Store information about Circle A
                float YA = position.at((3*i)+1);
                float ZA = position.at((3*i)+2);
                float lengthA = infoForShape.at(i);
                
                float XB = position.at(3*j);                                                       //Store information about Circle B
                float YB = position.at((3*j)+1);
                float ZB = position.at((3*j)+2);
                float lengthB = infoForShape.at(j);
                float deltaX = XB - XA;
                float deltaY = YB - YA;
                if(Math.sqrt((deltaX*deltaX)+(deltaY*deltaY))<infoForShape.at(j)){                //if Circle A is covered by circle B
                    cull.at(i) = true;                                                            //Dont draw Circle A (Circle A is culled)
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
            if(cull.at(i)){                                                                                //if shape has been culled by culling algorithim
                continue;                                                                                  //Do not evaluate
            }
            int minX = (int) position.at(3*i) - infoForShape.at(i);                                        //get bounds of Shape in terms of grid units (integers)
            int minY = (int) position.at((3*i)+1) - infoForShape.at(i);
            int maxX = (int) (position.at(3*i)+infoForShape.at(i)+1);
            int maxY = (int) (position.at((3*i)+1)+infoForShape.at(i)+1);
            for(int X = 0; X < maxX; X++){                                                                 //for each grid unit in bounds
                float deltaX = position.at(3*i) - X;
                float deltaY = position.at((3*i)+1) - Y;
                for(int Y = 0; Y < maxX; Y++){
                    isFilled[Y][X] = Math.sqrt((deltaX*deltaX)+(deltaY*deltaY))<infoForShape.at(i);        //this grid unit is filled if Math.sqrt((deltaX*deltaX)+(deltaY*deltaY))<radiusI
                }
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
    
    return 0;
}
