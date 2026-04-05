#include "gridComp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int DELTA_ROW[NUM_DIRECTIONS] = {-1, 1, 0, 0};
static const int DELTA_COL[NUM_DIRECTIONS] = {0, 0, -1, 1};

typedef struct StartPoint_st
{
    Point_st point;
    int score;
} StartPoint_st;

static int compareStartPoints(const void *a, const void *b)
{
    const StartPoint_st *sa = (const StartPoint_st *)a;
    const StartPoint_st *sb = (const StartPoint_st *)b;
    if (sb->score > sa->score)
        return 1;
    if (sb->score < sa->score)
        return -1;
    return 0;
}

Grid_st *makeGrid(int numRows, int numCols,
                  const Point_st *blockers, int blockerCount)
{
    if (numCols <= 0 || numRows <= 0)
        return NULL;

    Grid_st *grid = malloc(sizeof(Grid_st));
    if (!grid)
        return NULL;

    grid->numCols = numCols;
    grid->numRows = numRows;
    grid->grid = calloc((size_t)numCols, sizeof(bool *));
    grid->cellStorage = calloc((size_t)numRows * numCols, sizeof(bool));

    if (!grid->grid || !grid->cellStorage)
    {
        free(grid->grid);
        free(grid->cellStorage);
        free(grid);
        return NULL;
    }

    for (int i = 0; i < numCols; i++)
    {
        grid->grid[i] = grid->cellStorage + i * numRows;
    }

    if (blockers)
    {
        for (int j = 0; j < blockerCount; j++)
        {
            int bx = blockers[j].x;
            int by = blockers[j].y;
            if (bx < 0 || bx >= numCols || by < 0 || by >= numRows)
            {
                fprintf(stderr, "Warning: blocker (%d,%d) out of bounds, skipped\n",
                        bx, by);
                continue;
            }
            grid->grid[bx][by] = true;
        }
    }

    return grid;
}

void printGrid(const Grid_st *grid)
{
    if (!grid)
        return;

    for (int row = 0; row < grid->numRows; row++)
    {
        for (int col = 0; col < grid->numCols; col++)
        {
            printf("%d", grid->grid[col][row] ? 1 : 0);
        }
        printf("\n");
    }
}

void freeGrid(Grid_st *grid)
{
    if (!grid)
        return;
    free(grid->cellStorage);
    free(grid->grid);
    free(grid);
}

static StartPoint_st *findSeedPoints(BeamCtx_st *ctx, const Grid_st *grid,
                                     int maxSteps, int *outCount)
{
    int numCols = grid->numCols;
    int numRows = grid->numRows;

    int maxPoints;
    if (numCols == 1 && numRows == 1)
    {
        maxPoints = 1;
    }
    else
    {
        maxPoints = numCols * numRows;
    }

    if (maxPoints <= 0)
    {
        *outCount = 0;
        return NULL;
    }

    StartPoint_st *seeds = malloc(sizeof(StartPoint_st) * (size_t)maxPoints);
    if (!seeds)
    {
        *outCount = 0;
        return NULL;
    }

    int count = 0;

    for (int y = 0; y < numRows; y++)
    {
        for (int x = 0; x < numCols; x++)
        {
            if (!grid->grid[x][y])
            {
                seeds[count].point = (Point_st){x, y};
                seeds[count].score = beamLookaheadBFS(ctx, &seeds[count].point,
                                                      maxSteps, grid->grid);
                count++;
            }
        }
    }

    qsort(seeds, (size_t)count, sizeof(StartPoint_st), compareStartPoints);
    *outCount = count;
    return seeds;
}

static void expandBeam(BeamCtx_st *ctx, const Grid_st *grid)
{
    ctx->nextBeamSize = 0;

    for (int i = 0; i < ctx->currBeamSize; i++)
    {
        Candidate_st *cand = ctx->currBeam[i];

        for (int dir = 0; dir < NUM_DIRECTIONS; dir++)
        {
            int nx = cand->position.x + DELTA_COL[dir];
            int ny = cand->position.y + DELTA_ROW[dir];

            if (ny < 0 || ny >= grid->numRows)
                continue;
            if (nx < 0 || nx >= grid->numCols)
                continue;
            if (grid->grid[nx][ny])
                continue;
            if (bitCheck(cand->visited, ny, nx, grid->numCols))
                continue;
            if (ctx->nextBeamSize >= BEAM_WIDTH * NUM_DIRECTIONS)
                continue;

            Candidate_st *child = beamCopyCandidate(ctx, cand, nx, ny,
                                                    grid->grid);
            if (child)
            {
                ctx->nextBeam[ctx->nextBeamSize++] = child;
            }
        }
    }
}

static Candidate_st *selectWinner(const BeamCtx_st *ctx)
{
    if (ctx->currBeamSize <= 0)
        return NULL;

    Candidate_st *winner = ctx->currBeam[0];
    for (int i = 1; i < ctx->currBeamSize; i++)
    {
        if (ctx->currBeam[i]->score > winner->score)
        {
            winner = ctx->currBeam[i];
        }
    }
    return winner;
}

void findWinner(Grid_st *grid, int maxSteps)
{
    if (!grid || maxSteps <= 0)
    {
        printf("No path found\n");
        return;
    }

    BeamCtx_st *ctx = beamCtxCreate(grid->numCols, grid->numRows, maxSteps);
    if (!ctx)
        return;

    int seedCount = 0;
    StartPoint_st *seeds = findSeedPoints(ctx, grid, maxSteps, &seedCount);

    if (seedCount == 0)
    {
        printf("No path found\n");
        free(seeds);
        beamCtxDestroy(ctx);
        return;
    }

    int initialCount = seedCount < 20 ? seedCount : 20;
    for (int i = 0; i < initialCount; i++)
    {
        Candidate_st *cand = beamNewCandidate(ctx, seeds[i].point.x,
                                              seeds[i].point.y, maxSteps,
                                              grid->grid);
        if (cand && ctx->currBeamSize < BEAM_WIDTH)
        {
            ctx->currBeam[ctx->currBeamSize++] = cand;
        }
    }
    free(seeds);

    if (ctx->currBeamSize == 0)
    {
        printf("No path found\n");
        beamCtxDestroy(ctx);
        return;
    }

    while (ctx->currBeamSize > 0 && ctx->currBeam[0]->stepsRemaining > 0)
    {
        expandBeam(ctx, grid);

        if (ctx->nextBeamSize == 0)
            break;

        beamSort(ctx, false);

        for (int j = BEAM_WIDTH; j < ctx->nextBeamSize; j++)
        {
            candidateReleaseVisited(ctx->nextBeam[j]);
        }

        int newSize = ctx->nextBeamSize < BEAM_WIDTH
                          ? ctx->nextBeamSize
                          : BEAM_WIDTH;
        for (int k = 0; k < newSize; k++)
        {
            ctx->currBeam[k] = ctx->nextBeam[k];
        }
        ctx->currBeamSize = newSize;
    }

    Candidate_st *winner = selectWinner(ctx);
    if (!winner)
    {
        printf("No path found\n");
        beamCtxDestroy(ctx);
        return;
    }

    /* Reconstruct and print path */
    Point_st *path = malloc(sizeof(Point_st) * (size_t)(maxSteps + 1));
    if (!path)
    {
        beamCtxDestroy(ctx);
        return;
    }

    int pathLen = 0;
    int idx = maxSteps;
    const Candidate_st *current = winner;
    while (current != NULL)
    {
        path[idx] = current->position;
        current = current->parent;
        idx--;
        pathLen++;
    }
    memmove(path, &path[idx + 1], sizeof(Point_st) * (size_t)pathLen);

    for (int i = 0; i < pathLen; i++)
    {
        printf("(%d,%d), ", path[i].x + 1, path[i].y + 1);
    }
    printf("%d\n", winner->score);

    free(path);
    beamCtxDestroy(ctx);
}

