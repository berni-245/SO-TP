#include <stdint.h>
#include <stdio.h>

typedef enum { false = 0, true = 1 } boolean;

#define MAX_MEMORY_SIZE (1 << 20) // 1 MB
#define MIN_BLOCK_SIZE 1
#define ORDER_COUNT 20 // from order 0 to order 19
#define NULL (void *) 0

static const uint64_t addressByteSize = sizeof(void*);

typedef struct Block {
    struct Block* next;
    unsigned int size;
    boolean is_free;
} Block;

Block* free_list[ORDER_COUNT];

void init_buddy_system(void* endOfModules) {
    for (int i = 0; i < ORDER_COUNT; i++) {
        free_list[i] = NULL;
    }

    Block* initial_block = (Block*) (((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    initial_block->size = MAX_MEMORY_SIZE;
    initial_block->is_free = true;
    initial_block->next = NULL;
    free_list[ORDER_COUNT - 1] = initial_block;
}

int get_order(unsigned int size) {
    int order = 0;
    unsigned int block_size = MIN_BLOCK_SIZE;
    // Incluir el tamaño de la estructura Block en el cálculo
    size += sizeof(Block);
    while (block_size < size) {
        block_size <<= 1;
        order++;
    }
    return order;
}

Block* split_block(Block* block, int order) {
    unsigned int block_size = block->size;
    block->size = block_size / 2;

    // Calcular la dirección del buddy
    Block* buddy = (Block*)((char*)block + block->size);
    buddy->size = block->size;
    buddy->is_free = true;
    buddy->next = NULL;

    block->next = buddy;
    return buddy;
}

void* buddy_alloc(unsigned int size) {
    int order = get_order(size);

    for (int current_order = order; current_order < ORDER_COUNT; current_order++) {
        if (free_list[current_order] != NULL) {
            Block* block = free_list[current_order];
            free_list[current_order] = block->next;

            while (current_order > order) {
                current_order--;
                Block* buddy = split_block(block, current_order);
                buddy->next = free_list[current_order];
                free_list[current_order] = buddy;
            }

            block->is_free = false;
            return (void*)(block + 1);  // La memoria útil comienza después de la estructura Block
        }
    }

    return NULL;
}

void merge_block(Block* block, int order) {
    unsigned int block_size = block->size;
    uintptr_t buddy_address = (uintptr_t)block ^ block_size;
    Block* buddy = (Block*)buddy_address;

    if ((char*)buddy < (char*)free_list[order]) return;
    if (!buddy->is_free) return;
    if (buddy->size != block_size) return;

    if (buddy < block) {
        Block* temp = block;
        block = buddy;
        buddy = temp;
    }

    block->size *= 2;
    block->next = buddy->next;
    free_list[order] = block;

    merge_block(block, order + 1);
}

void buddy_free(void* ptr) {
    if (ptr == NULL) return;

    Block* block = (Block*)ptr - 1;  // La estructura Block está justo antes de la memoria útil
    block->is_free = true;
    int order = get_order(block->size);

    block->next = free_list[order];
    free_list[order] = block;

    merge_block(block, order);
}

void get_memory_state(char* buffer) {
    int offset = 0;
    for (int i = 0; i < ORDER_COUNT; i++) {
        offset += sprintf(buffer + offset, "Order %d: ", i);
        Block* block = free_list[i];
        while (block != NULL) {
            offset += sprintf(buffer + offset, "[Block at %p, size %u] -> ", block, block->size);
            block = block->next;
        }
        offset += sprintf(buffer + offset, "NULL\n");
    }
}
