#include "gridComp.h"
#include <stdio.h>
#include <stdlib.h>


Grid_st* makeGrid(int GC, int GR, Point_st* BL, int BLn)
{
    if(GC == 0 || GR == 0  || BL == NULL) return NULL;
    
    Grid_st* grid_s = malloc(sizeof(Grid_st));
    grid_s->gridCol = GC;
    grid_s->gridRow = GR;
    grid_s->grid = calloc(grid_s->gridRow, sizeof(bool*));
    grid_s->block = calloc(grid_s->gridRow * grid_s->gridCol, sizeof(bool));

    for (int i = 0; i < grid_s->gridCol; i++)
    {
        grid_s->grid[i] = grid_s->block + i * grid_s->gridRow;
    }
    for (int j = 0; j < BLn; j++)
    {
        grid_s->grid[BL[j].x][BL[j].y] = true;
    }
    return grid_s;
}


void printGrid(Grid_st* grid)
{
    if (grid != NULL)
    {
        int GC = grid->gridCol;
        int GR = grid->gridRow;

        for (int i = 0; i < GR; i++)
        {
            for (int j = 0; j < GC; j++)
            {
                printf("%u", (int)grid->grid[j][i]);
            }
            printf("\n");
        }
    }
}

void cleanUp(Grid_st* grid)
{
    free(grid->block);
    free(grid->grid);
    free(grid);
}