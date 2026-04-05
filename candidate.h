#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <stdint.h>
#include <stdbool.h>

#define BEAM_WIDTH 500
#define NUM_DIRECTIONS 4
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define VISITED_SIZE(rows, cols) (((rows) * (cols) + 7) / 8)

static inline int cellIdx(int row, int col, int cols)
{
    return row * cols + col;
}

static inline void bitSet(uint8_t *arr, int row, int col, int cols)
{
    int idx = cellIdx(row, col, cols);
    arr[idx / 8] |= (uint8_t)(1 << (idx % 8));
}

static inline int bitCheck(const uint8_t *arr, int row, int col, int cols)
{
    int idx = cellIdx(row, col, cols);
    return arr[idx / 8] & (1 << (idx % 8));
}

static inline void bitClear(uint8_t *arr, int row, int col, int cols)
{
    int idx = cellIdx(row, col, cols);
    arr[idx / 8] &= (uint8_t)~(1 << (idx % 8));
}

/* x = column, y = row */
typedef struct Point_st
{
    int x;
    int y;
} Point_st;

typedef struct BFSNode_st
{
    int row;
    int col;
    int depth;
} BFSNode_st;

typedef struct Candidate_st
{
    Point_st position;
    uint8_t *visited;
    int stepsRemaining;
    int score;
    int heuristicScore;
    const struct Candidate_st *parent;
} Candidate_st;

typedef struct BeamCtx_st
{
    Candidate_st *currBeam[BEAM_WIDTH];
    Candidate_st *nextBeam[BEAM_WIDTH * NUM_DIRECTIONS];
    int currBeamSize;
    int nextBeamSize;

    Candidate_st *pool;
    int poolIndex;
    int poolCapacity;

    BFSNode_st *bfsQueue;
    uint8_t *bfsSeen;

    int numCols;
    int numRows;
} BeamCtx_st;

BeamCtx_st *beamCtxCreate(int numCols, int numRows, int maxSteps);
void beamCtxDestroy(BeamCtx_st *ctx);

Candidate_st *beamNewCandidate(BeamCtx_st *ctx, int x, int y,
                               int maxSteps, bool **grid);
Candidate_st *beamCopyCandidate(BeamCtx_st *ctx, const Candidate_st *cand,
                                int x, int y, bool **grid);
void candidateReleaseVisited(Candidate_st *cand);
int beamLookaheadBFS(BeamCtx_st *ctx, const Point_st *pos,
                     int stepsLeft, bool **grid);
void beamSort(BeamCtx_st *ctx, bool curr);

#endif /* CANDIDATE_H */
