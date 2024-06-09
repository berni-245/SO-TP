#include <memoryManager.h>

#ifdef BUDDY

#include <stdint.h>
#include <utils.h>

typedef enum { false = 0, true = 1 } boolean;
typedef enum { LEFT = 'L', RIGHT = 'R' } blockAlignment;

#define ORDER_COUNT 27
// 2^(ORDER_COUNT-1) bytes
#define MAX_MEMORY_AVAILABLE (1 << (ORDER_COUNT - 1))

static const uint64_t addressByteSize = sizeof(void*);

typedef struct Block {
    struct Block* next;
    uint32_t size;
    boolean isFree;
} Block;

Block* freeList[ORDER_COUNT];
void* iniAddress;

void memoryInit(void* endOfModules) {
    iniAddress = endOfModules;
    for (int i = 0; i < ORDER_COUNT; i++) {
        freeList[i] = NULL;
    }
    Block* initialBlock = (Block*) (((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    initialBlock->size = MAX_MEMORY_AVAILABLE;
    initialBlock->isFree = true;
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
    uint32_t newBlockSize = block->size / 2;
    block->size = newBlockSize;

    Block* buddy = (Block*)((char*)block + newBlockSize);
    buddy->size = newBlockSize;
    buddy->isFree = true;
    return buddy;
}

void* malloc(uint64_t size) {
    // the block will also be allocated in the physical address
    int order = getOrder(size + sizeof(Block));

    for (int currentOrder = order; currentOrder < ORDER_COUNT; currentOrder++) {
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
            return (void*)(block + 1);
        }
    }

    return NULL;
}

static void removeFromFreeList(Block* toRemove, uint32_t order) {
    toRemove->isFree = false;
    if (freeList[order] == toRemove) {
        freeList[order] = toRemove->next;
        return;
    }

    Block* currentBlock = freeList[order];
    while (currentBlock != NULL && currentBlock->next != toRemove) {
        currentBlock = currentBlock->next;
    }
    if (currentBlock != NULL) {
        currentBlock->next = toRemove->next;
    }
    toRemove->next = NULL;
}

static blockAlignment getAlignment(Block * block) {
    if((((uint32_t) ((uint8_t *) block - (uint8_t *) iniAddress) / block->size) % 2) == 0)
        return LEFT;
    return RIGHT;
}

static void mergeBlock(Block* block, uint32_t order) {
    if (block->size == MAX_MEMORY_AVAILABLE){
        freeList[order] = block;
        return;
    }

    blockAlignment blockAlignment = getAlignment(block);
    
    Block* buddy = (blockAlignment == LEFT)
        ? (Block*)((char*)block + block->size)
        : (Block*)((char*)block - block->size);

    if (buddy == NULL || !buddy->isFree || buddy->size != block->size) {
        block->next = freeList[order];
        freeList[order] = block;
        return;
    }

    removeFromFreeList(block, order);
    removeFromFreeList(buddy, order);
    
    if(blockAlignment == RIGHT)
        block = buddy;

    block->size *= 2;
    block->isFree = true;
    mergeBlock(block, order + 1);
}

void free(void* ptr) {
    if (ptr == NULL) return;

    Block* block = (Block*)ptr - 1;
    block->isFree = true;
    int order = getOrder(block->size);

    mergeBlock(block, order);
}

#define MAX_STRING_SIZE 200

char * getMemoryState() {
    char * toReturn = malloc(MAX_STRING_SIZE);
    int i = strcpy(toReturn, "Total: ");
    i += uintToBase(MAX_MEMORY_AVAILABLE, toReturn + i, 10);
    i += strcpy(toReturn + i, " bytes ");

    uint32_t totalFreeMemory = 0;
    Block * currentBlock;
    for(int j = 0; j < ORDER_COUNT; j++) {
        currentBlock = freeList[j];
        while(currentBlock != NULL){
            totalFreeMemory += currentBlock->size;
            currentBlock = currentBlock->next;
        }
    }
    i += strcpy(toReturn + i, "| Used: ");
    i += uintToBase(MAX_MEMORY_AVAILABLE - totalFreeMemory, toReturn + i, 10);
    i += strcpy(toReturn + i, " bytes ");

    i += strcpy(toReturn + i, "| Unused: ");
    i += uintToBase(totalFreeMemory, toReturn + i, 10);
    i += strcpy(toReturn + i, " bytes ");
    toReturn[i] = 0;
    
    return toReturn;
}

#endif
