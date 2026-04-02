#include "candidate.h"
#include <stdlib.h>
#include <string.h>

static int maxX_g = 0;
static int maxY_g = 0;

static Candidate_st* pool;
static int pool_index = 0;

static Candidate_st* currBeam[B];
static Candidate_st* nextBeam[B * 4];
static int currBeamSize = 0;
static int nextBeamSize = 0;

static BFSNode_st *bfs_queue = NULL;
static uint8_t    *bfs_seen  = NULL;

void candidatesInit(int maxX, int maxY, int maxStep)
{
    maxX_g = maxX;
    maxY_g = maxY;
    pool = malloc(sizeof(Candidate_st) * (B + B * 4 * maxStep));

    bfs_queue = malloc(maxX * maxY * sizeof(BFSNode_st));
    bfs_seen  = malloc(VISITED_SIZE(maxY, maxX));
}

Candidate_st* newCandidate(int x, int y, int maxSteps, bool** grid)
{
    Candidate_st* ret = &pool[pool_index++];
    ret->position.x = x;
    ret->position.y = y;
    ret->stepsRemaining = maxSteps;
    ret->score = 1;
    ret->parent = NULL;
    ret->visited = calloc(VISITED_SIZE(maxY_g, maxX_g), sizeof(uint8_t));
    BIT_SET(ret->visited, y, x, maxX_g);
    ret->scoreH = lookaheadBFS(&ret->position, ret->visited, ret->stepsRemaining, grid);
    currBeam[currBeamSize++] = ret;
    return ret;
}

Candidate_st* copyCandidate(Candidate_st* cand, int x, int y, bool** grid)
{
    Candidate_st* ret = &pool[pool_index++];
    ret->position.x = x;
    ret->position.y = y;
    ret->stepsRemaining = cand->stepsRemaining - 1;
    int visSize = VISITED_SIZE(maxY_g, maxX_g);
    ret->visited = calloc(visSize, sizeof(uint8_t));
    memcpy(ret->visited, cand->visited, (visSize * sizeof(uint8_t)));
    if (!BIT_CHECK(ret->visited, ret->position.y, ret->position.x, maxX_g))
    {
        ret->score = cand->score + 1;
        BIT_SET(ret->visited, ret->position.y, ret->position.x, maxX_g);
    }else
    {
        ret->score = cand->score;
    }
    ret->parent = cand;
    ret->scoreH = lookaheadBFS(&ret->position, ret->visited, ret->stepsRemaining, grid);
    return ret;
}

void freeCandidate(Candidate_st* cand)
{
    free(cand->visited);   
}

int lookaheadBFS(Point_st* pos, uint8_t* vis, int stepsLeft, bool** grid)
{
    memset(bfs_seen, 0, VISITED_SIZE(maxY_g, maxX_g));
    int head = 0;
    int tail = 0;
    int bonusRet = 0;
    int dr[] = {-1,  1,  0,  0};
    int dc[] = { 0,  0, -1,  1};

    bfs_queue[tail++] = (BFSNode_st){pos->y, pos->x, 0};
    BIT_SET(bfs_seen, pos->y, pos->x, maxX_g);

    while (head != tail)
    {
        BFSNode_st curr = bfs_queue[head++];
        if (curr.depth >= stepsLeft)
        {
            continue;
        }

        for (int d = 0; d < 4; d++) {
            int rowNum = curr.row + dr[d];
            int colNum = curr.col + dc[d];

            if (rowNum < 0 || rowNum >= maxY_g) continue;
            if (colNum < 0 || colNum >= maxX_g) continue;

            if (grid[colNum][rowNum] == true) continue;

            if (BIT_CHECK(bfs_seen, rowNum, colNum, maxX_g)) continue;

            BIT_SET(bfs_seen, rowNum, colNum, maxX_g);

            if (!BIT_CHECK(vis, rowNum, colNum, maxX_g))
                bonusRet++;

            bfs_queue[tail++] = (BFSNode_st){rowNum, colNum, curr.depth + 1};
        }
    }
    return bonusRet;
}
