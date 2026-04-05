#include <stdio.h>
#include <stdlib.h>

#include "testParams.h"
#include "gridComp.h"

int main(void)
{
    Grid_st *grid = makeGrid(gridRows, gridCols, blockers, blockerCount);
    if (!grid)
    {
        fprintf(stderr, "Error: failed to create grid\n");
        return EXIT_FAILURE;
    }

    findWinner(grid, maxSteps);

    freeGrid(grid);
    return 0;
}
