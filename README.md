# GridTraversal

A high-performance grid path optimizer written in C, implementing **beam search with BFS lookahead** to find the longest non-revisiting path through an obstacle-filled grid.

## Problem

Given an N×M grid with blocked cells, find the starting position and movement sequence that visits the maximum number of unique cells within a fixed step budget.

This is a constrained combinatorial optimization problem — the search space grows exponentially with grid size and step count.

## Approach

The solver combines two complementary algorithms:

**Beam Search** — maintains a fixed-width frontier (`BEAM_WIDTH = 500`) of the most promising paths, expanding each candidate in all four directions per step. This keeps memory bounded while exploring deeply.

**BFS Lookahead Heuristic** — before committing to a candidate, runs a bounded BFS from its current position to estimate how many new cells are still reachable. This guides the beam toward paths with the most future potential, not just the best score so far.

**Seed Selection** — instead of starting from one fixed point, all non-blocked cells are ranked by lookahead score and the top 20 are seeded into the initial beam, reducing sensitivity to starting position.

## Technical Highlights

| Technique | Details |
|---|---|
| Memory pool | Pre-allocated slab for `Candidate_st` objects avoids per-step `malloc` overhead |
| Bitmask visited tracking | `uint8_t` bitfield packs cell-visited state into `ceil(rows*cols/8)` bytes per candidate |
| Flat 2D grid storage | Single contiguous `calloc` for the grid avoids pointer-chasing |
| Path reconstruction | Parent pointer chain — no path array stored mid-search |
| Manual memory hygiene | Every allocation paired with explicit `free`; pruned beam candidates release their bitmaps immediately |

## Build & Run

**Requirements:** `gcc`, `make`

```bash
make
./main
```

Output format: comma-separated `(col,row)` coordinates followed by the final score.

```
(3,1), (4,1), (5,1), (5,2), ... 22
```

To change the grid, edit [testParams.c](testParams.c) — adjust `gridRows`, `gridCols`, `maxSteps`, and the `blockers` array.

## Project Structure

```
main.c          Entry point
gridComp.c/h    Grid construction, seed selection, beam loop, path output
candidate.c/h   Beam context, candidate lifecycle, BFS heuristic, sorting
testParams.c/h  Configurable grid parameters and obstacle layout
Makefile        gcc -Wall -Wextra -std=c11
```
## Skills Demonstrated

- Algorithm design: beam search, BFS, heuristic search
- Systems C (C11): manual memory management, pointer arithmetic, bitmask operations
- Performance-aware data structures: memory pools, flat array layouts, bit packing
- Clean modular design with clear separation of concerns

## See also

[greedy-nearest-unvisited](https://github.com/michaljj/greedy-nearest-unvisited) — a greedy
nearest-unvisited solver for the non-revisiting variant of the same grid
traversal problem.

---

## License

Source code is licensed under the **GNU Affero General Public License v3.0**
(see [`LICENSE.md`](LICENSE.md)). You may use, modify and redistribute it
freely; if you run a modified version as a network service, you must offer
its source to users. No warranty.
