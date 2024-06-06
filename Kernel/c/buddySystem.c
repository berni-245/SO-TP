#include <stdint.h>
#include <stdio.h>

typedef enum { false = 0, true = 1 } boolean;

#define ORDER_COUNT 21
#define MAX_MEMORY_AVAILABLE (1 << (ORDER_COUNT - 1)) // 2^(ORDER_COUNT-1) bytes
#define NULL (void *) 0

static const uint64_t addressByteSize = sizeof(void*);

typedef struct Block {
    struct Block* currentBuddy;
    struct Block* nextBuddy;
    struct Block* next;
    uint32_t size;
    boolean isFree;
} Block;

Block* freeList[ORDER_COUNT];

void initBuddySystem(void* endOfModules) {
    for (int i = 0; i < ORDER_COUNT; i++) {
        freeList[i] = NULL;
    }

    Block* initialBlock = (Block*) (((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    initialBlock->size = MAX_MEMORY_AVAILABLE;
    initialBlock->isFree = true;
    initialBlock->currentBuddy = NULL;
    initialBlock->nextBuddy = NULL;
    initialBlock->next = NULL;
    freeList[ORDER_COUNT - 1] = initialBlock;
}

static int getOrder(uint32_t size) {
    // Part of the memory of the block will contain the structure
    size += sizeof(Block); 
    uint32_t order = 0;
    uint32_t iniBlockSize = 1;
    while (iniBlockSize < size) {
        iniBlockSize <<= 1;
        order++;
    }
    return order;
}

static Block* splitBlock(Block* block) {
    uint32_t newBlockSize = (block->size)/2;
    block->size = newBlockSize;

    // Get the direction of the buddy
    Block* buddy = (Block*)((char*)block + newBlockSize);
    buddy->size = newBlockSize;
    buddy->isFree = true;

    buddy->nextBuddy = NULL;
    buddy->currentBuddy = block;

    block->nextBuddy = block->currentBuddy;
    block->currentBuddy = buddy;
    return buddy;
}

void* buddyAlloc(uint32_t size) {
    int order = getOrder(size);

    for (int currentOrder = order; currentOrder < ORDER_COUNT; currentOrder++) {
        if (freeList[currentOrder] != NULL && freeList[currentOrder]->isFree) { // TODO: maybe remove isFree
            Block* block = freeList[currentOrder];
            freeList[currentOrder] = block->next;

            while (order < currentOrder) {
                currentOrder--;
                Block* buddy = splitBlock(block);
                buddy->next = freeList[currentOrder];
                freeList[currentOrder] = buddy;
            }

            block->isFree = false;
            return (void*)(block + 1);  // The useful memory will come after the block
        }
    }

    return NULL;
}

static void removeFromFreeList(Block* toRemove, uint32_t order) {
    Block* previousBlock = freeList[order];
    if(previousBlock == toRemove) {
        freeList[order] = previousBlock->next;
        return;
    }

    Block* currentBlock = previousBlock->next;
    while(currentBlock != toRemove) { // if we ever get a NULL, then there was a programming error somewhere
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;        
    }
    previousBlock->next = currentBlock->next;
}

static void mergeBlock(Block* block, uint32_t order) {
    block->next = freeList[order];
    freeList[order] = block;

    if(block->size == MAX_MEMORY_AVAILABLE) return;

    Block* buddy = block->currentBuddy;

    if (!buddy->isFree) return;
    if (buddy->size != block->size) return;

    freeList[order] = block->next; // remove the block from the free list
    removeFromFreeList(buddy, order); // remove the buddy from the free list

    block->currentBuddy = block->nextBuddy;
    block->nextBuddy = block->currentBuddy->next;
    block->size *= 2;

    mergeBlock(block, order + 1);
}

void buddyFree(void* ptr) {
    if (ptr == NULL) return;

    Block* block = (Block*)ptr - 1;  // The block structure will be before the address returned to the user
    block->isFree = true;
    int order = getOrder(block->size - sizeof(Block)); // TODO: use a variable in block for requested size

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
