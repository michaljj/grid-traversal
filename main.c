#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "testParams.h"
#include "gridComp.h"


int main()
{
    Grid_st* grid = makeGrid(gridC, gridR, blockers, (sizeof(blockers)/sizeof(Point_st)));
    printGrid(grid);
    cleanUp(grid);
    return 0;
}