#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <GL/glut.h>

using namespace std;

// Global variables
int verticalExtentOfGrid = 0;           // Store number of vertical grid units (number of rows)
int horizontalExtentOfGrid = 0;         // Store number of horizontal grid units (number of columns) 
int numberOfShapesToRender = 0;         // Store number of shapes to be provided for rendering
int numberThreads = 0;                  // Store number of threads to use in this algorithm
vector<float> infoForShape;             // Store information on the size of each shape
vector<float> position;                 // Store X, Y, and Z coordinate of shape
vector<bool> cull;                      // Store whether or not to draw a given shape
vector<vector<bool>> isFilled;          // Store whether or not a gridUnit is occupied by a shape
bool drawCircles = true;                // Toggle between circles and squares

// Thread pool and synchronization
vector<thread> threadPool;
mutex gridMutex;
mutex cullMutex;
condition_variable cv;
bool processingComplete = false;

// Function to check if Circle j contains Circle i
void CircleJContainsI(int i, int j) {
    if(cull[j]) {
        return; // If Circle B is covered by another Circle C, skip comparison
    }
    
    float XA = position.at(3*i);                 // Store information about Circle A
    float YA = position.at((3*i)+1);
    float ZA = position.at((3*i)+2);
            
    float XB = position.at(3*j);                 // Store information about Circle B
    float YB = position.at((3*j)+1);
    float ZB = position.at((3*j)+2);
    
    if (ZA >= ZB) {
        return; // Object A is in front of or at the same depth as object B
    }
    
    float radiusB = infoForShape.at(j);
    float deltaX = XB - XA;
    float deltaY = YB - YA;
    
    if(sqrt((deltaX*deltaX)+(deltaY*deltaY)) <= radiusB) {  // If Circle A is covered by circle B
        lock_guard<mutex> lock(cullMutex);
        cull.at(i) = true;                      // Don't draw Circle A (Circle A is culled)
    }
}

// Function to check if Square j contains Square i
void SquareJContainsI(int i, int j) {
    if(cull[j]) {
        return; // If Square B is covered by another Square C, skip comparison
    }
    
    float XA = position.at(3*i);                 // Store information about Square A
    float YA = position.at((3*i)+1);
    float ZA = position.at((3*i)+2);
    float lengthA = infoForShape.at(i);
            
    float XB = position.at(3*j);                 // Store information about Square B
    float YB = position.at((3*j)+1);
    float ZB = position.at((3*j)+2);
    float lengthB = infoForShape.at(j);
    
    if((XA>XB)&&(YA>YB)&&(ZA<ZB)&&(XA+lengthA<XB+lengthB)&&(YA+lengthA<YB+lengthB)) {
        lock_guard<mutex> lock(cullMutex);
        cull.at(i) = true;                      // Don't draw Square A (Square A is culled)
    }
}

// Function to check if a grid unit is inside a circle
void UnitXYinCircle(int X, int Y, int i) {
    float centerX = position.at(3*i);
    float centerY = position.at((3*i)+1);
    float radius = infoForShape.at(i);
    
    float deltaX = centerX - X;
    float deltaY = centerY - Y;
    
    if(sqrt((deltaX*deltaX)+(deltaY*deltaY)) < radius) {
        lock_guard<mutex> lock(gridMutex);
        isFilled[Y][X] = true;
    }
}

// Function to fill a circle with parallel processing
void fillCircleI(int i, int allocatedThreads, int threadID) {
    if(cull.at(i)) {
        return; // Skip if circle is culled
    }
    
    float centerX = position.at(3*i);
    float centerY = position.at((3*i)+1);
    float radius = infoForShape.at(i);
    
    int minX = max(0, static_cast<int>(centerX - radius));
    int minY = max(0, static_cast<int>(centerY - radius));
    int maxX = min(horizontalExtentOfGrid, static_cast<int>(centerX + radius + 1));
    int maxY = min(verticalExtentOfGrid, static_cast<int>(centerY + radius + 1));
    
    // Divide work among threads
    int totalPixels = (maxX - minX) * (maxY - minY);
    int pixelsPerThread = totalPixels / allocatedThreads;
    int startPixel = threadID * pixelsPerThread;
    int endPixel = (threadID == allocatedThreads - 1) ? totalPixels : (threadID + 1) * pixelsPerThread;
    
    for (int pixelIndex = startPixel; pixelIndex < endPixel; pixelIndex++) {
        int row = pixelIndex / (maxX - minX);
        int col = pixelIndex % (maxX - minX);
        
        int X = minX + col;
        int Y = minY + row;
        
        UnitXYinCircle(X, Y, i);
    }
}

// Function to fill a square
void fillSquareI(int i, int allocatedThreads, int threadID) {
    if(cull.at(i)) {
        return; // Skip if square is culled
    }
    
    float X = position.at(3*i);
    float Y = position.at((3*i)+1);
    float length = infoForShape.at(i);
    
    int minX = max(0, static_cast<int>(X));
    int minY = max(0, static_cast<int>(Y));
    int maxX = min(horizontalExtentOfGrid, static_cast<int>(X + length + 1));
    int maxY = min(verticalExtentOfGrid, static_cast<int>(Y + length + 1));
    
    // Divide work among threads
    int totalPixels = (maxX - minX) * (maxY - minY);
    int pixelsPerThread = totalPixels / allocatedThreads;
    int startPixel = threadID * pixelsPerThread;
    int endPixel = (threadID == allocatedThreads - 1) ? totalPixels : (threadID + 1) * pixelsPerThread;
    
    for (int pixelIndex = startPixel; pixelIndex < endPixel; pixelIndex++) {
        int row = pixelIndex / (maxX - minX);
        int col = pixelIndex % (maxX - minX);
        
        int fillX = minX + col;
        int fillY = minY + row;
        
        lock_guard<mutex> lock(gridMutex);
        isFilled[fillY][fillX] = true;
    }
}

// Worker function for parallel culling
void cullWorker(int startShape, int endShape) {
    for (int i = startShape; i < endShape; i++) {
        for (int j = 0; j < numberOfShapesToRender && !cull.at(i); j++) {
            if (i == j) continue; // Don't compare shape with itself
            
            if (drawCircles) {
                CircleJContainsI(i, j);
            } else {
                SquareJContainsI(i, j);
            }
        }
    }
}

// Worker function for parallel filling
void fillWorker(int startShape, int endShape) {
    for (int i = startShape; i < endShape; i++) {
        if (cull.at(i)) continue;
        
        // Calculate number of threads to allocate for this shape
        int shapeSideLength = ceil(infoForShape.at(i) * 2);
        int pixelsToProcess = shapeSideLength * shapeSideLength;
        int threadsForShape = min(numberThreads / 4, max(1, pixelsToProcess / 1000));
        
        vector<thread> shapeThreads;
        for (int t = 0; t < threadsForShape; t++) {
            if (drawCircles) {
                shapeThreads.push_back(thread(fillCircleI, i, threadsForShape, t));
            } else {
                shapeThreads.push_back(thread(fillSquareI, i, threadsForShape, t));
            }
        }
        
        for (auto& t : shapeThreads) {
            t.join();
        }
    }
}

// Perform culling algorithm with parallel processing
void performCulling() {
    // Reset cull array
    cull.assign(numberOfShapesToRender, false);
    
    // Determine number of shapes per thread
    int shapesPerThread = ceil(static_cast<float>(numberOfShapesToRender) / numberThreads);
    
    // Create and launch threads
    vector<thread> cullThreads;
    for (int t = 0; t < numberThreads; t++) {
        int startShape = t * shapesPerThread;
        int endShape = min((t + 1) * shapesPerThread, numberOfShapesToRender);
        
        if (startShape < numberOfShapesToRender) {
            cullThreads.push_back(thread(cullWorker, startShape, endShape));
        }
    }
    
    // Join all threads
    for (auto& t : cullThreads) {
        t.join();
    }
}

// Fill grid based on shapes with parallel processing
void fillGrid() {
    // Reset grid
    isFilled.assign(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));
    
    // Determine number of shapes per thread
    int shapesPerThread = ceil(static_cast<float>(numberOfShapesToRender) / numberThreads);
    
    // Create and launch threads
    vector<thread> fillThreads;
    for (int t = 0; t < numberThreads; t++) {
        int startShape = t * shapesPerThread;
        int endShape = min((t + 1) * shapesPerThread, numberOfShapesToRender);
        
        if (startShape < numberOfShapesToRender) {
            fillThreads.push_back(thread(fillWorker, startShape, endShape));
        }
    }
    
    // Join all threads
    for (auto& t : fillThreads) {
        t.join();
    }
}

// Function to draw a circle using OpenGL
void drawCircle(float x, float y, float radius, float z) {
    const int segments = 50;
    glPushMatrix();
    glTranslatef(x, y, z * 0.01); // Scale Z for visualization
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);  // Center
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float dx = radius * cos(angle);
        float dy = radius * sin(angle);
        glVertex2f(dx, dy);
    }
    glEnd();
    glPopMatrix();
}

// Function to draw a square using OpenGL
void drawSquare(float x, float y, float size, float z) {
    glPushMatrix();
    glTranslatef(x, y, z * 0.01); // Scale Z for visualization
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(size, 0.0f);
    glVertex2f(size, size);
    glVertex2f(0.0f, size);
    glEnd();
    glPopMatrix();
}

// Display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Draw the grid
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i <= horizontalExtentOfGrid; i++) {
        glVertex2f(i, 0);
        glVertex2f(i, verticalExtentOfGrid);
    }
    for (int i = 0; i <= verticalExtentOfGrid; i++) {
        glVertex2f(0, i);
        glVertex2f(horizontalExtentOfGrid, i);
    }
    glEnd();
    
    // Draw the shapes
    for (int i = 0; i < numberOfShapesToRender; i++) {
        if (cull.at(i)) {
            continue;
        }
        
        float x = position.at(3*i);
        float y = position.at((3*i)+1);
        float z = position.at((3*i)+2);
        float size = infoForShape.at(i);
        
        // Set color based on z-depth for visual distinction
        float colorIntensity = 0.3f + 0.7f * (z / 10.0f);
        glColor4f(colorIntensity, 0.2f, 1.0f - colorIntensity, 0.7f);
        
        if (drawCircles) {
            drawCircle(x, y, size, z);
        } else {
            drawSquare(x, y, size, z);
        }
    }
    
    // Draw the filled grid cells
    glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
    for (int y = 0; y < verticalExtentOfGrid; y++) {
        for (int x = 0; x < horizontalExtentOfGrid; x++) {
            if (isFilled[y][x]) {
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x+1, y);
                glVertex2f(x+1, y+1);
                glVertex2f(x, y+1);
                glEnd();
            }
        }
    }
    
    glutSwapBuffers();
}

// Reshape callback function
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set the coordinate system to match grid units
    double aspectRatio = (double)w / (double)h;
    if (w >= h) {
        double xExtent = aspectRatio * verticalExtentOfGrid;
        gluOrtho2D(-1.0, xExtent, -1.0, verticalExtentOfGrid + 1);
    } else {
        double yExtent = horizontalExtentOfGrid / aspectRatio;
        gluOrtho2D(-1.0, horizontalExtentOfGrid + 1, -1.0, yExtent);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Keyboard callback function
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'q':
        case 'Q':
        case 27: // ESC key
            exit(0);
            break;
        case 'c':
        case 'C':
            drawCircles = true;
            cout << "Switched to Circle mode" << endl;
            // Re-run algorithms with circles
            performCulling();
            fillGrid();
            break;
        case 's':
        case 'S':
            drawCircles = false;
            cout << "Switched to Square mode" << endl;
            // Re-run algorithms with squares
            performCulling();
            fillGrid();
            break;
        case 'r':
        case 'R':
            cout << "Recalculating..." << endl;
            // Re-run culling and filling algorithms
            performCulling();
            fillGrid();
            break;
        case '+':
            numberThreads = min(32, numberThreads + 1);
            cout << "Increased thread count to: " << numberThreads << endl;
            break;
        case '-':
            numberThreads = max(1, numberThreads - 1);
            cout << "Decreased thread count to: " << numberThreads << endl;
            break;
    }
    glutPostRedisplay();
}

// Initialize OpenGL settings
void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Function to generate random shapes for testing
void generateRandomShapes() {
    numberOfShapesToRender = 20; // Change this for more or fewer shapes
    
    // Resize vectors
    infoForShape.resize(numberOfShapesToRender);
    position.resize(numberOfShapesToRender * 3);
    cull.assign(numberOfShapesToRender, false);
    
    // Create random shapes
    for (int i = 0; i < numberOfShapesToRender; i++) {
        infoForShape[i] = 1.0f + (rand() % 5); // Random size between 1 and 5
        position[3*i] = rand() % horizontalExtentOfGrid;  // Random x
        position[3*i+1] = rand() % verticalExtentOfGrid;  // Random y
        position[3*i+2] = rand() % 10;  // Random z between 0 and 9
    }
}

// Main function
int main(int argc, char** argv) {
    // Initialize grid size
    verticalExtentOfGrid = 20;    // Default values - can be changed
    horizontalExtentOfGrid = 20;  // Default values - can be changed
    
    // Set default number of threads
    numberThreads = thread::hardware_concurrency();
    if (numberThreads == 0) numberThreads = 4; // Default if hardware_concurrency fails
    
    cout << "Using " << numberThreads << " threads" << endl;
    
    // Initialize isFilled array
    isFilled.assign(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));
    
    // Generate random shapes or read from input
    if (argc > 1 && string(argv[1]) == "-i") {
        // Read input from stdin
        cin >> verticalExtentOfGrid;
        cin >> horizontalExtentOfGrid;
        isFilled.assign(verticalExtentOfGrid, vector<bool>(horizontalExtentOfGrid, false));
        cin >> numberOfShapesToRender;
        
        // Read number of threads
        cin >> numberThreads;
        cout << "Using " << numberThreads << " threads as specified" << endl;
        
        infoForShape.resize(numberOfShapesToRender);
        position.resize(numberOfShapesToRender * 3);
        cull.assign(numberOfShapesToRender, false);
        
        for (int i = 0; i < numberOfShapesToRender; i++) {
            cin >> infoForShape.at(i);     // Read in radius or side length
            cin >> position.at(3*i);       // Read in X coordinate
            cin >> position.at((3*i)+1);   // Read in Y coordinate
            cin >> position.at((3*i)+2);   // Read in Z coordinate
        }
    } else {
        generateRandomShapes();
    }
    
    // Perform culling and fill grid using parallel algorithms
    performCulling();
    fillGrid();
    
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Parallel Shape Renderer");
    
    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    // Initialize OpenGL
    init();
    
    cout << "Controls:" << endl;
    cout << "  'c' - Switch to Circle mode" << endl;
    cout << "  's' - Switch to Square mode" << endl;
    cout << "  'r' - Recalculate culling and grid" << endl;
    cout << "  '+' - Increase thread count" << endl;
    cout << "  '-' - Decrease thread count" << endl;
    cout << "  'q' or ESC - Quit" << endl;
    
    // Start main loop
    glutMainLoop();
    
    return 0;
}