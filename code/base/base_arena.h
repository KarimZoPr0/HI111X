#ifndef BASE_ARENA_H
#define BASE_ARENA_H

#include <stdlib.h>
#include "base_core.h"

C_LINKAGE_BEGIN

// Default arena block data size in bytes.
#define ARENA_DEFAULT_SIZE 1024

// Header size reserved at the beginning of each block.
#define ARENA_HEADER_SIZE 128

//----------------------------------------------------------------------
// Types

typedef struct ArenaBlock {
    struct ArenaBlock *prev; // Pointer to previous block (or NULL for base block)
    size_t size;             // Total size (bytes available for data)
    size_t used;             // Number of bytes already allocated from this block
    char data[];             // Block data (usable memory)
} ArenaBlock;

typedef struct Arena {
    ArenaBlock *current;
} Arena;

typedef struct Temp {
    Arena *arena;         // Arena in which temp scope began
    ArenaBlock *block;    // Block in which the allocation started
    size_t used;          // The used offset of that block when temp_begin was called
} Temp;

Arena *ArenaAlloc(void);
void ArenaRelease(Arena *arena);
void *ArenaPush(Arena *arena, size_t size, size_t align);
size_t ArenaGetPos(Arena *arena);
void ArenaPopTo(Arena *arena, size_t pos);
void ArenaClear(Arena *arena);
void ArenaPop(Arena *arena, size_t amt);

Temp TempBegin(Arena *arena);
void TempEnd(Temp temp);

//----------------------------------------------------------------------
// Helper Macros for Type‑Safe Allocations

#define push_array_no_zero_aligned(a, T, c, align) ( (T *)ArenaPush((a), sizeof(T)*(c), (align)) )
#define push_array_aligned(a, T, c, align) ( (T *)MemoryZero(push_array_no_zero_aligned(a, T, c, align), sizeof(T)*(c)) )
#define push_array_no_zero(a, T, c) push_array_no_zero_aligned(a, T, c, Max(8, AlignOf(T)))
#define push_array(a, T, c) push_array_aligned(a, T, c, Max(8, AlignOf(T)))

C_LINKAGE_END
#endif //BASE_ARENA_H
