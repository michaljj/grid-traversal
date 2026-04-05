#include "candidate.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int DELTA_ROW[NUM_DIRECTIONS] = {-1, 1, 0, 0};
static const int DELTA_COL[NUM_DIRECTIONS] = {0, 0, -1, 1};

static int compareCandidates(const void *a, const void *b)
{
    const Candidate_st *ca = *(const Candidate_st *const *)a;
    const Candidate_st *cb = *(const Candidate_st *const *)b;
    if (cb->heuristicScore > ca->heuristicScore) return  1;
    if (cb->heuristicScore < ca->heuristicScore) return -1;
    return 0;
}

static int lookaheadBFS(BeamCtx_st *ctx, const Point_st *pos,
                        const uint8_t *vis, int stepsLeft, bool **grid)
{
    int numCols = ctx->numCols;
    int numRows = ctx->numRows;

    memset(ctx->bfsSeen, 0, VISITED_SIZE(numRows, numCols));
    int head = 0;
    int tail = 0;
    int bonus = 0;

    ctx->bfsQueue[tail++] = (BFSNode_st){pos->y, pos->x, 0};
    bitSet(ctx->bfsSeen, pos->y, pos->x, numCols);

    while (head != tail)
    {
        BFSNode_st curr = ctx->bfsQueue[head++];
        if (curr.depth >= stepsLeft)
        {
            continue;
        }

        for (int d = 0; d < NUM_DIRECTIONS; d++)
        {
            int row = curr.row + DELTA_ROW[d];
            int col = curr.col + DELTA_COL[d];

            if (row < 0 || row >= numRows)
                continue;
            if (col < 0 || col >= numCols)
                continue;
            if (grid[col][row])
                continue;
            if (bitCheck(ctx->bfsSeen, row, col, numCols))
                continue;

            bitSet(ctx->bfsSeen, row, col, numCols);

            if (!bitCheck(vis, row, col, numCols))
            {
                bonus++;
            }

            ctx->bfsQueue[tail++] = (BFSNode_st){row, col, curr.depth + 1};
        }
    }
    return bonus;
}

static Candidate_st *poolAlloc(BeamCtx_st *ctx)
{
    if (ctx->poolIndex >= ctx->poolCapacity)
    {
        fprintf(stderr, "Fatal: candidate pool exhausted (%d/%d)\n",
                ctx->poolIndex, ctx->poolCapacity);
        return NULL;
    }
    return &ctx->pool[ctx->poolIndex++];
}

BeamCtx_st *beamCtxCreate(int numCols, int numRows, int maxSteps)
{
    BeamCtx_st *ctx = calloc(1, sizeof(BeamCtx_st));
    if (!ctx)
    {
        fprintf(stderr, "Fatal: failed to allocate beam context\n");
        return NULL;
    }

    ctx->numCols = numCols;
    ctx->numRows = numRows;
    ctx->poolCapacity = BEAM_WIDTH + BEAM_WIDTH * NUM_DIRECTIONS * maxSteps;
    ctx->poolIndex = 0;
    ctx->currBeamSize = 0;
    ctx->nextBeamSize = 0;

    ctx->pool = malloc(sizeof(Candidate_st) * (size_t)ctx->poolCapacity);
    ctx->bfsQueue = malloc((size_t)numCols * numRows * sizeof(BFSNode_st));
    ctx->bfsSeen = malloc(VISITED_SIZE(numRows, numCols));

    if (!ctx->pool || !ctx->bfsQueue || !ctx->bfsSeen)
    {
        fprintf(stderr, "Fatal: failed to allocate beam search buffers\n");
        free(ctx->pool);
        free(ctx->bfsQueue);
        free(ctx->bfsSeen);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void beamCtxDestroy(BeamCtx_st *ctx)
{
    if (!ctx)
        return;

    for (int i = 0; i < ctx->poolIndex; i++)
    {
        if (ctx->pool[i].visited != NULL)
        {
            free(ctx->pool[i].visited);
            ctx->pool[i].visited = NULL;
        }
    }
    free(ctx->pool);
    free(ctx->bfsQueue);
    free(ctx->bfsSeen);
    free(ctx);
}

Candidate_st *beamNewCandidate(BeamCtx_st *ctx, int x, int y,
                               int maxSteps, bool **grid)
{
    Candidate_st *cand = poolAlloc(ctx);
    if (!cand)
        return NULL;

    cand->position.x = x;
    cand->position.y = y;
    cand->stepsRemaining = maxSteps - 1;
    cand->score = 1; 
    cand->parent = NULL;

    cand->visited = calloc(VISITED_SIZE(ctx->numRows, ctx->numCols),
                           sizeof(uint8_t));
    if (!cand->visited)
    {
        fprintf(stderr, "Fatal: failed to allocate visited bitmap\n");
        return NULL;
    }

    bitSet(cand->visited, y, x, ctx->numCols);
    cand->heuristicScore = lookaheadBFS(ctx, &cand->position, cand->visited,
                                        cand->stepsRemaining, grid);
    return cand;
}

Candidate_st *beamCopyCandidate(BeamCtx_st *ctx, const Candidate_st *cand,
                                int x, int y, bool **grid)
{
    Candidate_st *copy = poolAlloc(ctx);
    if (!copy)
        return NULL;

    copy->position.x = x;
    copy->position.y = y;
    copy->stepsRemaining = cand->stepsRemaining - 1;

    int visSize = VISITED_SIZE(ctx->numRows, ctx->numCols);
    copy->visited = malloc(visSize);
    if (!copy->visited)
    {
        fprintf(stderr, "Fatal: failed to allocate visited bitmap\n");
        return NULL;
    }
    memcpy(copy->visited, cand->visited, visSize);

    if (!bitCheck(copy->visited, y, x, ctx->numCols))
    {
        copy->score = cand->score + 1;
    }
    else
    {
        copy->score = cand->score;
    }
    bitSet(copy->visited, y, x, ctx->numCols);

    copy->parent = cand;
    copy->heuristicScore = lookaheadBFS(ctx, &copy->position, copy->visited,
                                        copy->stepsRemaining, grid);
    return copy;
}

void candidateReleaseVisited(Candidate_st *cand)
{
    if (cand)
    {
        free(cand->visited);
        cand->visited = NULL;
    }
}

int beamLookaheadBFS(BeamCtx_st *ctx, const Point_st *pos,
                     int stepsLeft, bool **grid)
{
    uint8_t *emptyVis = calloc(VISITED_SIZE(ctx->numRows, ctx->numCols),
                               sizeof(uint8_t));
    if (!emptyVis)
        return 0;

    int score = lookaheadBFS(ctx, pos, emptyVis, stepsLeft, grid);
    free(emptyVis);
    return score;
}

void beamSort(BeamCtx_st *ctx, bool curr)
{
    if (curr)
    {
        qsort(ctx->currBeam, ctx->currBeamSize,
              sizeof(Candidate_st *), compareCandidates);
    }
    else
    {
        qsort(ctx->nextBeam, ctx->nextBeamSize,
              sizeof(Candidate_st *), compareCandidates);
    }

}