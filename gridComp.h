#ifndef GRIDCOMP_H
#define GRIDCOMP_H

#include <stdbool.h>
#include "candidate.h"


typedef struct Grid_st
{
    bool** grid;
    bool* block;
    int gridRow;
    int gridCol;

}Grid_st;

typedef struct StartPoint_st
{
    Point_st point;
    int score;
}StartPoint_st;


Grid_st* makeGrid(int GC, int GR, Point_st* BL, int BLn);

void printGrid(Grid_st* grid);

void cleanUp(Grid_st* grid);

StartPoint_st* findSeedPoints(Grid_st* grid, int maxStep, int* outCount);

void findWinner(Grid_st *grid, int maxSteps);

void printPath(Point_st* path, int pathLen);

#endif //GRIDCOMP_H