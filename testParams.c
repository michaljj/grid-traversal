#include "testParams.h"

const Point_st blockers[] = {
    {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {7, 2}, {3, 3}, {3, 4}, {5, 5}, {6, 6}};

const int blockerCount = sizeof(blockers) / sizeof(blockers[0]);
const int gridRows = 8;
const int gridCols = 8;
const int maxSteps = 25;
