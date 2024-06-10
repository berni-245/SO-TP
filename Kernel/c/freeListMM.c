#include <memoryManager.h>

#ifndef BUDDY

#include <stdint.h>

#define heapBITS_PER_BYTE ((uint64_t) 8)
// Marks the most significant bit in 64 bits as 1, and the rest as 0
#define heapBLOCK_ALLOCATED_BITMASK (((uint64_t)1) << ((sizeof(uint64_t) * heapBITS_PER_BYTE) - 1))
#define heapBLOCK_SIZE_IS_VALID(blockSize) (((blockSize) & heapBLOCK_ALLOCATED_BITMASK) == 0)
#define heapBLOCK_IS_ALLOCATED(block) (((block->blockSize) & heapBLOCK_ALLOCATED_BITMASK) != 0)
#define heapALLOCATE_BLOCK(block) ((block->blockSize) |= heapBLOCK_ALLOCATED_BITMASK)
#define heapFREE_BLOCK(block) ((block->blockSize) &= ~heapBLOCK_ALLOCATED_BITMASK)
#define NULL (void *) 0

typedef struct blockLink_t{
  struct blockLink_t * nextFreeBlock;
  uint64_t blockSize;
} blockLink_t;

// 64MB max heap size
#define MAX_MEMORY_AVAILABLE ((1 << 20) * 64)

static blockLink_t listStart;
static blockLink_t * listEnd = NULL;

static const uint64_t addressByteSize = sizeof(void*);
static const uint64_t heapStructSize = sizeof(blockLink_t); // should be 16 bytes

static uint64_t freeBytesRemaining;

// Used so it doesn't split into too small blocks, it's value should be 32 bytes
#define MINIMUM_BLOCK_SIZE_FOR_SPLIT ((uint64_t) (heapStructSize << 1)) 

void memoryInit(void* endOfModules) {
  void * alinedHeapStart = (void*)(((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
  listStart.nextFreeBlock = (blockLink_t *) alinedHeapStart;
  listStart.blockSize = 0;

  void * alinedHeapEnd = (void*)((uint64_t) (alinedHeapStart + MAX_MEMORY_AVAILABLE - heapStructSize));
  listEnd = (blockLink_t *) alinedHeapEnd;
  listEnd->blockSize = 0;
  listEnd->nextFreeBlock = NULL;

  blockLink_t * firstFreeBlock = (blockLink_t *) alinedHeapStart;
  firstFreeBlock->blockSize = (uint64_t) (alinedHeapEnd - alinedHeapStart);
  firstFreeBlock->nextFreeBlock = listEnd;

  freeBytesRemaining = firstFreeBlock->blockSize;
}

static void insertBlockIntoFreeList(blockLink_t * blockToInsert){
  blockLink_t * blockIterator;
  uint8_t *aux;

  // Iterate through the list until a block is found that has a higher address than the block being inserted. 
  for (blockIterator = &listStart; blockIterator->nextFreeBlock < blockToInsert; blockIterator = blockIterator->nextFreeBlock) {}

  // Check if block inserted is after blockIterator
  aux = (uint8_t *) blockIterator;

  if ((aux + blockIterator->blockSize) == (uint8_t *)blockToInsert) {
      blockIterator->blockSize += blockToInsert->blockSize;
      blockToInsert = blockIterator;
  }
  // Check if block inserted is right before blockIterator->next
  aux = (uint8_t *)blockToInsert;

  if ((aux + blockToInsert->blockSize) == (uint8_t *) blockIterator->nextFreeBlock) {
      if (blockIterator->nextFreeBlock != listEnd) {
          /* Form one big block from the two blocks. */
          blockToInsert->blockSize += blockIterator->nextFreeBlock->blockSize;
          blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock->nextFreeBlock;
      }
      else {
          blockToInsert->nextFreeBlock = listEnd;
      }
  }
  else {
      blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock;
  }

  // Check so it doesn't point to itself
  if (blockIterator != blockToInsert) {
      blockIterator->nextFreeBlock = blockToInsert;
  }
}

void* mallocFreeList(uint64_t size) {
  blockLink_t * block;
  blockLink_t * previousBlock;
  blockLink_t * newBlockLink;
  void * toReturn = NULL;
  uint64_t alinedRequiredSize = 0;

  if(size > 0){
    // We need to align it since the user can request any size
    alinedRequiredSize = (size + heapStructSize + addressByteSize - 1) & ~(addressByteSize - 1);
  }

  if(heapBLOCK_SIZE_IS_VALID(alinedRequiredSize) && alinedRequiredSize > 0 && alinedRequiredSize <= freeBytesRemaining){
    previousBlock = &listStart;
    block = listStart.nextFreeBlock;

    while ((block->blockSize < alinedRequiredSize) && (block->nextFreeBlock != NULL)){
      previousBlock = block;
      block = block->nextFreeBlock;
    }

    // If we reached the end marker, then there's no block of the size needed
    if(block != listEnd){
      // The useful memory starts after the block structure
      toReturn = (void *)(previousBlock->nextFreeBlock + 1);

      previousBlock->nextFreeBlock = block->nextFreeBlock;

      if((block->blockSize - alinedRequiredSize) > MINIMUM_BLOCK_SIZE_FOR_SPLIT){
        newBlockLink = (blockLink_t *) ((uint8_t *)block + alinedRequiredSize);
        newBlockLink->blockSize = block->blockSize - alinedRequiredSize;
        block->blockSize = alinedRequiredSize;

        insertBlockIntoFreeList(newBlockLink);
      }

      freeBytesRemaining -= block->blockSize;

      heapALLOCATE_BLOCK(block);
      block->nextFreeBlock = NULL;
    }
    else{
      return NULL;
    }
  }
  else{
    return NULL;
  }

  return toReturn;
}

void freeFreeList(void* ptr) {
  if( ptr == NULL) return;
  // The block structure is before the useful memory
  blockLink_t *freeBlock = (blockLink_t *) ptr - 1;

  if (heapBLOCK_IS_ALLOCATED(freeBlock) != 0 && freeBlock->nextFreeBlock == NULL) {
      heapFREE_BLOCK(freeBlock);
      freeBytesRemaining += freeBlock->blockSize;
      insertBlockIntoFreeList(((blockLink_t *)freeBlock));
  }
}

#endif
