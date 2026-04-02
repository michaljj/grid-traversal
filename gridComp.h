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


Grid_st* makeGrid(int GC, int GR, Point_st* BL, int BLn);

void printGrid(Grid_st* grid);

void cleanUp(Grid_st* grid);

#endif //GRIDCOMP_H