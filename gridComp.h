#ifndef GRIDCOMP_H
#define GRIDCOMP_H

#include <stdbool.h>
#include "candidate.h"

typedef struct Grid_st
{
    bool **grid;
    bool *cellStorage;
    int numRows;
    int numCols;
} Grid_st;

Grid_st *makeGrid(int numRows, int numCols,
                  const Point_st *blockers, int blockerCount);
void freeGrid(Grid_st *grid);
void printGrid(const Grid_st *grid);
void findWinner(Grid_st *grid, int maxSteps);

#endif /* GRIDCOMP_H */
