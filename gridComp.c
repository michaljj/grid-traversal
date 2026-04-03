#include "gridComp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int compareStartPoints(const void* a, const void* b)
{
    const StartPoint_st* sa = (const StartPoint_st*)a;
    const StartPoint_st* sb = (const StartPoint_st*)b;
    if (sb->score > sa->score) return  1;
    if (sb->score < sa->score) return -1;
    return 0;
}

Grid_st* makeGrid(int GC, int GR, Point_st* BL, int BLn)
{
    if(GC == 0 || GR == 0  || BL == NULL) return NULL;
    
    Grid_st* grid_s = malloc(sizeof(Grid_st));
    grid_s->gridCol = GC;
    grid_s->gridRow = GR;
    grid_s->grid = calloc(grid_s->gridCol, sizeof(bool*));
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


StartPoint_st* findSeedPoints(Grid_st* grid, int maxStep, int* outCount)
{
    int maxPoints = 2 * grid->gridCol + 2 * grid->gridRow - 4;
    StartPoint_st* startPointArr = malloc(sizeof(StartPoint_st) * maxPoints);
    int startPointIdx = 0;

    for (int x = 0; x < grid->gridCol; x++)
    {
        if (grid->grid[x][0] == false)
        {
            startPointArr[startPointIdx].point.x = x;
            startPointArr[startPointIdx].point.y = 0;
            startPointArr[startPointIdx].score = lookaheadBFS_ext(&(Point_st){x, 0}, maxStep, grid->grid);
            startPointIdx++;
        }
    }

    for (int x = 0; x < grid->gridCol; x++)
    {
        if (grid->grid[x][grid->gridRow - 1] == false)
        {
            startPointArr[startPointIdx].point.x = x;
            startPointArr[startPointIdx].point.y = grid->gridRow - 1;
            startPointArr[startPointIdx].score = lookaheadBFS_ext(&(Point_st){x, grid->gridRow - 1}, maxStep, grid->grid);
            startPointIdx++;
        }
    }

    for (int y = 1; y < grid->gridRow - 1; y++)
    {
        if (grid->grid[0][y] == false)
        {
            startPointArr[startPointIdx].point.x = 0;
            startPointArr[startPointIdx].point.y = y;
            startPointArr[startPointIdx].score = lookaheadBFS_ext(&(Point_st){0, y}, maxStep, grid->grid);
            startPointIdx++;
        }
    }

    for (int y = 1; y < grid->gridRow - 1; y++)
    {
        if (grid->grid[grid->gridCol - 1][y] == false)
        {
            startPointArr[startPointIdx].point.x = grid->gridCol - 1;
            startPointArr[startPointIdx].point.y = y;
            startPointArr[startPointIdx].score = lookaheadBFS_ext(&(Point_st){grid->gridCol - 1, y}, maxStep, grid->grid);
            startPointIdx++;
        }
    }

    qsort(startPointArr, startPointIdx, sizeof(StartPoint_st), compareStartPoints);
    *outCount = startPointIdx;
    return startPointArr;
}

void findWinner(Grid_st *grid, int maxSteps)
{
    candidatesInit(grid->gridCol, grid->gridRow, maxSteps);
    int pointsCnt = 0;
    StartPoint_st* points = findSeedPoints(grid, maxSteps, &pointsCnt);

    int dr[] = {-1,  1,  0,  0};
    int dc[] = { 0,  0, -1,  1};
    int newCands = B <= pointsCnt ? B : pointsCnt;
    for (int i = 0; i < newCands; i++)
    {
        newCandidate(points[i].point.x, points[i].point.y, maxSteps, grid->grid);
    }

    while (currBeam[0]->stepsRemaining > 0)
    {
        nextBeamSize = 0;

        for (int i = 0; i < currBeamSize; i++)
        {
            for (int dir = 0; dir < 4; dir++)
            {
                Point_st nextPoint = {currBeam[i]->position.x + dc[dir], currBeam[i]->position.y + dr[dir]};
                if (nextPoint.y < 0 || nextPoint.y >= grid->gridRow) continue;
                if (nextPoint.x < 0 || nextPoint.x >= grid->gridCol) continue;

                if (grid->grid[nextPoint.x][nextPoint.y] == true) continue;

                if (BIT_CHECK(currBeam[i]->visited, nextPoint.y, nextPoint.x, grid->gridCol)) continue;

                if (nextBeamSize >= B * 4) continue;
                nextBeam[nextBeamSize++] = copyCandidate(currBeam[i], nextPoint.x, nextPoint.y, grid->grid);
                
            }
            
        }

        sortBeam(false);
        for (int j = B; j < nextBeamSize; j++)
        {
            freeCandidate(nextBeam[j]);
        }

        for (int k = 0; k < B; k++)
        {
            currBeam[k] = nextBeam[k];
        }
        currBeamSize = nextBeamSize < B ? nextBeamSize : B;
        
    }
    
    Candidate_st* winner = currBeam[0];
    for (int i = 0; i <= currBeamSize; i++)
    {
        if (currBeam[i]->score > winner->score)
        {
            winner = currBeam[i];
        }
    }
    
    Point_st* path = malloc(sizeof(Point_st) * (maxSteps + 1));

    Candidate_st* current = winner;
    int pathLen = 0;
    int idx = maxSteps;
    while(current != NULL)
    {
        path[idx].x = current->position.x;
        path[idx].y = current->position.y;
        current = current->parent;
        idx--;
        pathLen++;
    }
    memmove(path, &path[idx + 1], sizeof(Point_st) * pathLen);
    
    for(int i = 0; i < pathLen; i++)
    {
        printf("(%i,%i)", path[i].x, path[i].y);
    }
    printf("\n");
    candidatesUninit();
    free(points);
    free(path);
}

void printPath(Point_st *path, int pathLen)
{
    for (int i = 0; i < pathLen; i++)
    {
        printf("(%i,%i)", path[i].x, path[i].y);
    }
    printf("\n");
    
}
