#include <stdint.h>
#include <stdio.h>

typedef enum { false = 0, true = 1 } boolean;
// When a block is split, it will be on the left or on the right, the alignment is important for merging
typedef enum { LEFT = 'L', RIGHT = 'R'} blockAlignment;

#define ORDER_COUNT 21
// 2^(ORDER_COUNT-1) bytes
#define MAX_MEMORY_AVAILABLE (1 << (ORDER_COUNT - 1))
#define NULL (void *) 0

static const uint64_t addressByteSize = sizeof(void*);

typedef struct Block {
    struct Block* next;
    uint32_t size;
    boolean isFree;
    blockAlignment align;
} Block;

Block* freeList[ORDER_COUNT];

void initBuddySystem(void* endOfModules) {
    for (int i = 0; i < ORDER_COUNT; i++) {
        freeList[i] = NULL;
    }
    Block* initialBlock = (Block*) (((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    initialBlock->size = MAX_MEMORY_AVAILABLE;
    initialBlock->isFree = true;
    initialBlock->align = LEFT;
    initialBlock->next = NULL;
    freeList[ORDER_COUNT - 1] = initialBlock;
}

static int getOrder(uint32_t size) {
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

    buddy->align = RIGHT;
    return buddy;
}

void* buddyAlloc(uint32_t size) {
    // the block will also be allocated in the physical address
    int order = getOrder(size + sizeof(Block));

    for (int currentOrder = order; currentOrder < ORDER_COUNT; currentOrder++) {
        // TODO: maybe remove isFree
        if (freeList[currentOrder] != NULL && freeList[currentOrder]->isFree) { 
            Block* block = freeList[currentOrder];
            freeList[currentOrder] = block->next;

            while (order < currentOrder) {
                currentOrder--;
                Block* buddy = splitBlock(block);
                buddy->next = freeList[currentOrder];
                freeList[currentOrder] = buddy;
            }

            block->isFree = false;
            // The useful memory will come after the block
            return (void*)(block + 1);
        }
    }

    return NULL;
}

static void removeFromFreeList(Block* toRemove, uint32_t order) {
    toRemove->isFree = false;
    Block* previousBlock = freeList[order];
    if(previousBlock == toRemove) {
        freeList[order] = previousBlock->next;
        return;
    }

    Block* currentBlock = previousBlock->next;
    while(currentBlock != toRemove) { 
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;        
    }
    previousBlock->next = currentBlock->next;
}

static void mergeBlock(Block* block, uint32_t order) {
    // this always needs to happen, in case there's no merging blocks
    block->next = freeList[order]; 
    freeList[order] = block;

    if(block->size == MAX_MEMORY_AVAILABLE) return;

    Block* buddy;
    if(block->align == LEFT)
        buddy = (Block*)((char*)block + block->size);
    else
        buddy = (Block*)((char*)block - block->size);

    if (buddy == NULL) return;
    if (!buddy->isFree) return;
    if (buddy->size != block->size) return;
    

    // remove the block and buddy from the free list
    removeFromFreeList(block, order);
    removeFromFreeList(buddy, order);
    if(block->align == RIGHT)
        block = buddy;

    block->size *= 2;
    block->isFree = true;
    mergeBlock(block, order + 1);
}

void buddyFree(void* ptr) {
    if (ptr == NULL) return;

    // The block structure will be before the address returned to the user
    Block* block = (Block*)ptr - 1; 
    block->isFree = true;
    int order = getOrder(block->size); 

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
