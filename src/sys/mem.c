/*
 *  BermudaOS - Memory module
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file mem.c */

#include <stddef.h>
#include <bermuda.h>
#include <lib/binary.h>

#include <sys/mem.h>

#include <arch/io.h>

PRIVATE WEAK volatile HEAPNODE *BermudaHeapHead = NULL;

static inline HEAPNODE *BermudaHeapInitHeader(HEAPNODE *node, size_t size);

/**
 * \fn void *BermudaHeapAlloc(size_t size)
 * \brief Allocated a given amout of memory.
 * \param size Requested memory size.
 * \return Pointer to the start of the memory block.
 * 
 * This function will search for a fitting block of memory. If no fitting block
 * is found it will return <i><b>NULL</b></i>.
 */
__attribute__ ((malloc)) void *BermudaHeapAlloc(size_t size) 
{
        if(size > MEM)
                return NULL;
        void *ret = NULL;
        volatile HEAPNODE *c = BermudaHeapHead, *prev = NULL;
        while(c)
        {
                if(c->size == size)
                { // requested block fits perfectly
                        break;
                }
                
                if(c->size > size)
                { // block is to large
                        if(c->size < size+sizeof(*c)+4)
                                break; // block size is to small to split
                        
                        // split the node
                        BermudaHeapSplitNode(c, size);
                        break;
                }
                prev = c;
                c = c->next;
        }
        
        if(c == NULL)
                return NULL;
        
        BermudaHeapUseBlock(c, prev);
        ret = ((void*)c)+sizeof(*c);
        return ret;
}

/**
 * \fn BermudaHeapUseBlock(volatile HEAPNODE *node, volatile HEAPNODE *prev)
 * \brief Use a memory block from the heap.
 * \param node Block which is going to be used.
 * \param prev Block in the list before <i>node</i>.
 * 
 * This block will set the magic attribute to used and remove the block from the
 * heap list.
 */
PRIVATE WEAK void BermudaHeapUseBlock(volatile HEAPNODE *node,
                                      volatile HEAPNODE *prev)
{
        if(node->magic != BERMUDA_MM_FREE_MAGIC)
        {
#ifdef __VERBAL__
                printf("BermudaUseHeapBlock failure!\n");
#endif
                return;
        }
        
        node->magic = BERMUDA_MM_ALLOC_MAGIC;
        if(prev != NULL && node->next != NULL)
        { // somewhere in the middle of the list
                prev->next = node->next;
                return;
        }
        if(prev == NULL)
        { // at the start of the list
                BermudaHeapHead = node->next;
                return;
        }
        if(node->next == NULL)
        {
                prev->next = NULL;
                return;
        }
}

/**
 * \fn BermudaHeapInitBlock(void *start, size_t size)
 * \brief Initialise a new heap block.
 * \param start Start of the new block.
 * \param size Size of the block.
 * 
 * This function will initialise a new heap block, and add it to to heap list.
 */
void BermudaHeapInitBlock(volatile void *start, size_t size)
{        
        if(!BermudaHeapHead)
        { // if the MM is not yet initialised
                BermudaHeapHead = (HEAPNODE*)start;
                BermudaHeapHead->magic = BERMUDA_MM_FREE_MAGIC;
                BermudaHeapHead->size = size;
        }
        
        return;
}

/**
 * \fn BermudaNodeReturn(HEAPNODE *block)
 * \brief Return a block to the list.
 * \param block Block to return.
 * 
 * This will put the given block back in the linked heap list.
 */
PRIVATE WEAK char BermudaNodeReturn(volatile HEAPNODE *block)
{
        if(block->magic != BERMUDA_MM_ALLOC_MAGIC || block->size == 0)
                return -1; // don't accept nonsense blocks
        
        block->magic = BERMUDA_MM_FREE_MAGIC;
        
        if(block < BermudaHeapHead)
        {// we're at the start of the list
                block->next = (void*)BermudaHeapHead;
                BermudaHeapHead = block;
                return 0;
        }
        
        volatile HEAPNODE *node = BermudaHeapHead;
        volatile HEAPNODE *prev = NULL;
        while(node)
        {
                if(block > node && block < node->next)
                {
                        prev->next = block;
                        block->next = node;
                        return 0;
                }
                
                if(NULL == node->next)
                { // the block shall fit here
                        node->next = block;
                        block->next = NULL;
                        return 0;
                }
                
                prev = node;
                node = node->next;
        }
        return -1;
}

/**
 * \fn BermudaHeapMergeNode(volatile HEAPNODE *alpha, volatile HEAPNODE *beta)
 * \brief Merge two nodes if possible.
 * \param alpha Node 1.
 * \param beta Node 2.
 * \return Error code.
 * 
 * This function will try to merge node <i>alpha</i> and <i>beta</i>. If it is
 * not possible, -1 is returned.
 */
PRIVATE WEAK char BermudaHeapMergeNode(alpha, beta)
volatile HEAPNODE *alpha;
volatile HEAPNODE *beta;
{
        if(alpha->magic != BERMUDA_MM_FREE_MAGIC || beta->magic !=
        /* no crap blocks */                              BERMUDA_MM_FREE_MAGIC)
        {
#ifdef __VERBAL__
                printf("Heap merge failed!\n");
#endif
                return -1;
        }
        
        if((((void*)alpha) + sizeof(HEAPNODE)+ alpha->size != beta) &&
                (((void*)beta) + sizeof(HEAPNODE) + beta->size != alpha))
                return -1;
        
        if(((void*)beta) + sizeof(HEAPNODE) + beta->size == alpha)
        { // alpa and beta are given reversed.. swap
                volatile HEAPNODE *tmp = alpha;
                alpha = beta;
                beta = tmp;
        }
        
        alpha->size = beta->size + sizeof(HEAPNODE); // calc new size
        alpha->next = beta->next;
        
        return 0;
}

/**
 * \fn BermudaHeapSplitNode(volatile HEAPNODE *node, size_t req)
 * \brief Split the given memory node.
 * \param node The memory node to split.
 * \param req Requested size of node.
 * 
 * This function will split the memory node <i>node</i> to the given size <i>
 * size</i>.
 */
PRIVATE WEAK void BermudaHeapSplitNode(volatile HEAPNODE *node, size_t req)
{
        if(node->magic != BERMUDA_MM_FREE_MAGIC)
        {
#ifdef __VERBAL__
                printf("Node merge failed: %p - Size: %x\n", node, node->size);
#endif
                return;
        }
        
        volatile HEAPNODE *next = BermudaHeapInitHeader(((void*)node)+req+
                                        sizeof(*node),
                                             node->size - req - sizeof(*next));
        next->next = node->next;
        node->next = next;
        node->size = req;
}

/**
 * \fn BermudaHeapInitHeader(HEAPNODE *node, size_t size)
 * \brief Initialise a heap node.
 * \param node Memory address to add the header to.
 * \param size Size of <i>node</i>.
 * \return The memory address of the given node.
 * 
 * At the given memory address a heap node header will be initialised. The
 * HEAPNODE.magic attribute will be set to <i>BERMUDA_MM_FREE_MAGIC</i>.
 */
static inline HEAPNODE *BermudaHeapInitHeader(HEAPNODE *node, size_t size)
{
        node->magic = BERMUDA_MM_FREE_MAGIC;
        node->size = size;
        node->next = NULL;
        return node;
}
