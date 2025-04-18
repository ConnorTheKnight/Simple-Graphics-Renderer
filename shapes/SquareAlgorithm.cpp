#include <iostream>
#include <vector>

using namespace std;

int main()
{
    //-----Declare Variables-----
    int verticalExtentOfGrid = 0;   //Store number of vertical grid units (number of rows)
    int horizontalExtentOfGrid = 0; //Store number of horizontal grid units (number of columns) 
    int numberOfShapesToRender = 0; //Store number of shapes to be provided for rendering
    vector<float> infoForShape;           //Store information on the size of each shape (for now only squares are being considered and thus only 1 sidelength per square is needed)
    vector<float> position;               //Store X, Y, and Z coordinate of a known point of this shape (for now that is the bottom left corner of the square [Note: Bottom = minimum Y value, Left = minimum X value])
    bool* cull;                      //Store whether or not to draw a given shape (output of culling algorithm)
    bool** isFilled;          //Store whether or not a gridUnit is occupied by a shape (output of draw algorithm)
    
    
    //-----Read User Input-----
    //Read in number of vertical grid units from stdin
    cin >> verticalExtentOfGrid;
    //Read in number of horizontal grid units from stdin
    cin >> horizontalExtentOfGrid;
    //Initialize isFilled Array
    isFilled = new bool* [verticalExtentOfGrid];
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        isFilled[i] = new bool[horizontalExtentOfGrid];
    }
    //Read in number of shapes to render from stdin
    cin >> numberOfShapesToRender;
    //Initialize Info Array (Note that for future 2-D shapes more than one piece of information per shape may be needed)
    infoForShape.resize(numberOfShapesToRender);
    //Initialize Positon Array
    position.resize(numberOfShapesToRender*2);
    //Initialize Cull Array
    cull = new bool[numberOfShapesToRender] {};
    //Read in number of threads from stdin to be compatible with input formatting
    int unused;
    cin >> unused;
    //Read in information on dimensions of each shape (sidelength of each square) followed by its X, Y, and Z coordinates from stdin
    for(int i = 0; i < numberOfShapesToRender; i++){
        cin >> infoForShape.at(i);     //Read in information on dimensions (sidelength)
        cin >> position.at(2*i);       //Read in X coordinate
        cin >> position.at((2*i)+1);   //Read in Y coordinate
    }
    
    //-----Begin Processing-----
    /* Note that this is the part of the algorithm which will need concurrency to be implemneted efficiently*/
        //-----Culling Algorithim-----
        /*A simple Culling Algorithm for determining whether Square A is covered by square B is:
        if((XA>XB)&&(YA>YB)&&(ZA<ZB)&&(XA+lengthA<XB+lengthB)&&(YA+lengthA<YB+lengthB)){
            square A is covered
        }
        
        This implementation will compare each pair of squares using that algorithim in O(N^2) time 
        [Note: this might be able to be sped up by using an O(NLog(N)) sort to limit which squares need to be compared by Z value]
        [Another potential optimization is to store nearby shapes in some kind of buffer so that shapes very far apart do not need to be compared]
        */
        for(int i = 0; i < numberOfShapesToRender; i++){                                        //for each Square A
            for(int j = 0; j < numberOfShapesToRender&&!cull[i]; j++){                           //for each Square B (Skip further comparisons if Square A has already been covered)
                if(cull[j]){                                                                   //if Square B is covered by another Square C
                    continue;                                                                   //Dont bother comparing against Square A as if Square B covers Square A then Square C will cover Square A
                }
                if(i==j){                                                                   //if Square B is covered by another Square C
                    continue;                                                                   //Dont bother comparing against Square A as if Square B covers Square A then Square C will cover Square A
                }
                float XA = position.at(2*i);                                                       //Store information about Square A
                float YA = position.at((2*i)+1);
                float lengthA = infoForShape.at(i);
                
                float XB = position.at(2*j);                                                       //Store information about Square B
                float YB = position.at((2*j)+1);
                float lengthB = infoForShape.at(j);
                if((XA>XB)&&(YA>YB)&&(XA+lengthA<XB+lengthB)&&(YA+lengthA<YB+lengthB)){//if Square A is covered by square B
                    cull[i] = true;                                                            //Dont draw Square A (Square A is culled)
                }
            }
        }                                                                                       
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
        for(int i = 0; i < numberOfShapesToRender; i++){
            if(cull[i]){                                            //if shape has been culled by culling algorithim
                continue;                                           //Do not evaluate
            }
            int minX = (int) position.at(2*i);                         //get bounds of Shape in terms of grid units (integers)
            int minY = (int) position.at((2*i)+1);
            int maxX = (int) (position.at(2*i)+infoForShape.at(i)+1);
            int maxY = (int) (position.at((2*i)+1)+infoForShape.at(i)+1);
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
                    isFilled[Y][X] = true;                          //this grid unit is filled
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

    delete[] cull;
    for (int i = 0; i < verticalExtentOfGrid; i++) {
        delete[] isFilled[i];
    }
    delete[] isFilled;
    return 0;
}
