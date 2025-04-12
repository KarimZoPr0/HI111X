//
// Created by Abdik on 2025-03-18.
//

#include "base_arena.h"
#include "base_core.h"

internal ArenaBlock* ArenaCreateBlock(size_t block_data_size)
{
    // Allocate enough memory for the header plus the block's data.
    ArenaBlock* block = (ArenaBlock*)malloc(sizeof(ArenaBlock) + block_data_size);
    if (!block)
    {
        fprintf(stderr, "ArenaCreateBlock: Failed to allocate block of size %zu\n", block_data_size);
        exit(1);
    }
    block->prev = NULL;
    block->size = block_data_size;
    block->used = 0;
    return block;
}

Arena* ArenaAlloc(void)
{
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    if (!arena)
    {
        fprintf(stderr, "ArenaAlloc: Failed to allocate Arena structure\n");
        exit(1);
    }
    // Allocate an initial block of default size.
    arena->current = ArenaCreateBlock(ARENA_DEFAULT_SIZE);
    // Reserve space for the header in each block.
    arena->current->used = ARENA_HEADER_SIZE;
    return arena;
}

void ArenaRelease(Arena* arena)
{
    if (!arena) return;
    ArenaBlock* block = arena->current;
    while (block)
    {
        ArenaBlock* prev = block->prev;
        free(block);
        block = prev;
    }
    free(arena);
}

void* ArenaPush(Arena* arena, size_t size, size_t align)
{
    ArenaBlock* block = arena->current;
    // Align the current used offset.
    size_t pos = block->used;
    size_t aligned_pos = (pos + align - 1) & ~(align - 1);

    // If there is not enough room in the current block, allocate a new block.
    if (aligned_pos + size > block->size)
    {
        // Choose new block size: either default size or enough for the new allocation.
        size_t new_block_size = ARENA_DEFAULT_SIZE;
        if (size + ARENA_HEADER_SIZE > new_block_size)
            new_block_size = size + ARENA_HEADER_SIZE;

        ArenaBlock* new_block = ArenaCreateBlock(new_block_size);
        new_block->used = ARENA_HEADER_SIZE; // Reserve header area.
        new_block->prev = block;
        arena->current = new_block;
        block = new_block;
        aligned_pos = (block->used + align - 1) & ~(align - 1);
    }

    void* ptr = block->data + aligned_pos;
    block->used = aligned_pos + size;
    return ptr;
}

size_t ArenaGetPos(Arena* arena)
{
    return arena->current->used;
}

void ArenaPopTo(Arena* arena, size_t pos)
{
    ArenaBlock* block = arena->current;
    // If pos is less than the header offset, it's invalid.
    if (pos < ARENA_HEADER_SIZE)
    {
        fprintf(stderr, "ArenaPopTo: Invalid position %zu (must be >= %d)\n", pos, ARENA_HEADER_SIZE);
        exit(1);
    }
    // If the current block doesn't have enough used bytes, pop blocks.
    while (block && block->used < pos)
    {
        ArenaBlock* prev = block->prev;
        free(block);
        if (!prev)
        {
            fprintf(stderr, "ArenaPopTo: Underflow, cannot pop to %zu\n", pos);
            exit(1);
        }
        arena->current = prev;
        block = prev;
    }
    if (pos > block->used)
    {
        fprintf(stderr, "ArenaPopTo: Invalid position %zu (current used %zu)\n", pos, block->used);
        exit(1);
    }
    block->used = pos;
}

void ArenaClear(Arena* arena)
{
    // Free all blocks except the base block.
    ArenaBlock* block = arena->current;
    while (block->prev)
    {
        ArenaBlock* prev = block->prev;
        free(block);
        block = prev;
    }
    block->used = ARENA_HEADER_SIZE;
    arena->current = block;
}

void ArenaPop(Arena* arena, size_t amt)
{
    size_t cur_pos = ArenaGetPos(arena);
    size_t new_pos = (amt > cur_pos) ? ARENA_HEADER_SIZE : cur_pos - amt;
    ArenaPopTo(arena, new_pos);
}

Temp TempBegin(Arena* arena)
{
    Temp temp;
    temp.arena = arena;
    temp.block = arena->current;
    temp.used = arena->current->used;
    return temp;
}

void TempEnd(Temp temp)
{
    while (temp.arena->current != temp.block)
    {
        ArenaBlock* cur = temp.arena->current;
        temp.arena->current = cur->prev;
        free(cur);
    }
    temp.block->used = temp.used;
}
