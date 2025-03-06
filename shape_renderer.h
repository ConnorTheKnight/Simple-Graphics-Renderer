#ifndef SHAPE_RENDERER_H
#define SHAPE_RENDERER_H

#include "shapes/circle_renderer.h"
#include "shapes/triangle_renderer.h"
#include "shapes/square_renderer.h"

// Function declarations for rendering different shapes
void renderCircles(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);
void renderTriangles(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);
void renderSquares(const std::vector<float>& shapeValues, int verticalExtentOfGrid, int horizontalExtentOfGrid, int numberOfShapes);

#endif // SHAPE_RENDERER_H
