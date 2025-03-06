#ifndef SQUARE_RENDERER_H
#define SQUARE_RENDERER_H

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

// Function to render squares on the grid with the given parameters
void renderSquares(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);

#endif // SQUARE_RENDERER_H