#ifndef CIRCLE_RENDERER_H
#define CIRCLE_RENDERER_H

#include <vector>
#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>

// Function to render circles on the grid with the given parameters
void renderCircles(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);

#endif // CIRCLE_RENDERER_H
