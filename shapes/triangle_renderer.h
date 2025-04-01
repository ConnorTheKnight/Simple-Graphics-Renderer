#ifndef TRIANGLE_RENDERER_H
#define TRIANGLE_RENDERER_H

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>

// Function to render triangles on the grid with the given parameters
void renderTriangles(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);

#endif // TRIANGLE_RENDERER_H