#include <stdint.h>
#include <stdio.h>

typedef enum { false = 0, true = 1 } boolean;

#define ORDER_COUNT 20
#define MAX_MEMORY_SIZE (1 << (ORDER_COUNT)) // 2^(ORDER_COUNT) bytes
#define NULL (void *) 0

static const uint64_t addressByteSize = sizeof(void*);

typedef struct Block {
    struct Block* next;
    unsigned int size;
    boolean isFree;
} Block;

Block* freeList[ORDER_COUNT];

void initBuddySystem(void* endOfModules) {
    for (int i = 0; i < ORDER_COUNT; i++) {
        freeList[i] = NULL;
    }

    Block* initialBlock = (Block*) (((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    initialBlock->size = MAX_MEMORY_SIZE;
    initialBlock->isFree = true;
    initialBlock->next = NULL;
    freeList[ORDER_COUNT - 1] = initialBlock;
}

static int getOrder(unsigned int size) {
    // Incluir el tamaño de la estructura Block en el cálculo
    size += sizeof(Block);
    int order = 0;
    uint64_t iniBlockSize = 1;
    while (iniBlockSize < size) {
        iniBlockSize <<= 1;
        order++;
    }
    return order;
}

static Block* splitBlock(Block* block, int order) {
    unsigned int blockSize = block->size;
    block->size = blockSize / 2;

    // Calcular la dirección del buddy
    Block* buddy = (Block*)((char*)block + block->size);
    buddy->size = block->size;
    buddy->isFree = true;
    buddy->next = NULL;

    block->next = buddy;
    return buddy;
}

void* buddyAlloc(unsigned int size) {
    int order = getOrder(size);

    for (int currentOrder = order; currentOrder < ORDER_COUNT; currentOrder++) {
        if (freeList[currentOrder] != NULL) {
            Block* block = freeList[currentOrder];
            freeList[currentOrder] = block->next;

            while (currentOrder > order) {
                currentOrder--;
                Block* buddy = splitBlock(block, currentOrder);
                buddy->next = freeList[currentOrder];
                freeList[currentOrder] = buddy;
            }

            block->isFree = false;
            return (void*)(block + 1);  // La memoria útil comienza después de la estructura Block
        }
    }

    return NULL;
}

static void mergeBlock(Block* block, int order) {
    unsigned int blockSize = block->size;
    uintptr_t buddyAddress = (uintptr_t)block ^ blockSize;
    Block* buddy = (Block*)buddyAddress;

    if ((char*)buddy < (char*)freeList[order]) return;
    if (!buddy->isFree) return;
    if (buddy->size != blockSize) return;

    if (buddy < block) {
        Block* temp = block;
        block = buddy;
        buddy = temp;
    }

    block->size *= 2;
    block->next = buddy->next;
    freeList[order] = block;

    mergeBlock(block, order + 1);
}

void buddyFree(void* ptr) {
    if (ptr == NULL) return;

    Block* block = (Block*)ptr - 1;  // La estructura Block está justo antes de la memoria útil
    block->isFree = true;
    int order = getOrder(block->size);

    block->next = freeList[order];
    freeList[order] = block;

    mergeBlock(block, order);
}

void getMemoryState(char* buffer) {
    int offset = 0;
    for (int i = 0; i < ORDER_COUNT; i++) {
        offset += sprintf(buffer + offset, "Order %d: ", i);
        Block* block = freeList[i];
        while (block != NULL) {
            offset += sprintf(buffer + offset, "[Block at %p, size %u] -> ", block, block->size);
            block = block->next;
        }
        offset += sprintf(buffer + offset, "NULL\n");
    }
}
