#ifndef CANDIDATE_H
#define CANDIDATE_H
#include <stdint.h>
#include <stdbool.h>

#define B 20

#define VISITED_SIZE(rows, cols)        (((rows) * (cols) + 7) / 8)

#define CELL_IDX(row, col, cols)        ((row) * (cols) + (col))

#define BIT_SET(arr, row, col, cols)    ((arr)[CELL_IDX(row, col, cols) / 8] |=  (1 << (CELL_IDX(row, col, cols) % 8)))
#define BIT_CHECK(arr, row, col, cols)  ((arr)[CELL_IDX(row, col, cols) / 8] &   (1 << (CELL_IDX(row, col, cols) % 8)))
#define BIT_CLEAR(arr, row, col, cols)  ((arr)[CELL_IDX(row, col, cols) / 8] &= ~(1 << (CELL_IDX(row, col, cols) % 8)))



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
    uint8_t* visited;
    int stepsRemaining;
    int score;
    int scoreH;
    struct Candidate_st* parent;
}Candidate_st;


extern Candidate_st* currBeam[B];
extern Candidate_st* nextBeam[B * 4];
extern int currBeamSize;
extern int nextBeamSize;

void newCandidate(int x, int y, int maxSteps, bool** grid);

Candidate_st* copyCandidate(Candidate_st* cand, int x, int y, bool** grid);

void freeCandidate(Candidate_st* cand);

int lookaheadBFS_ext(Point_st* pos, int stepsLeft, bool** grid);

void sortBeam(bool curr);

void candidatesInit(int maxX, int maxY, int maxStep);
void candidatesUninit();

#endif //CANDIDATE_H