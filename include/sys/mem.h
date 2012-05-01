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

/** \file mem.h */

#ifndef __MEM_H__
#define __MEM_H__

#include <stddef.h>
#include <bermuda.h>

#include <lib/binary.h>

#include <sys/mem.h>

#define BERMUDA_MM_FREE_MAGIC 0x99
#define BERMUDA_MM_ALLOC_MAGIC 0x66

struct heap_node
{
        unsigned char magic;
        volatile struct heap_node *next;
        unsigned short size;
} __PACK__;
typedef struct heap_node HEAPNODE;

__DECL

/**
 * \fn extern BermudaHeapInitBlock(volatile void *start, size_t size)
 * \brief Initialise a new heap block.
 * \param start Start of the new block.
 * \param size Size of the block.
 * 
 * This function will initialise a new heap block, and add it to to heap list.
 */
void BermudaHeapInitBlock(volatile void *start, size_t size);

/**
 * \fn BermudaHeapSplitNode(volatile HEAPNODE *node, size_t req)
 * \brief Split the given memory node.
 * \param node The memory node to split.
 * \param req Requested size of node.
 * 
 * This function will split the memory node <i>node</i> to the given size <i>
 * size</i>.
 */
PRIVATE WEAK void BermudaHeapSplitNode(volatile HEAPNODE *node, size_t req);

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
PRIVATE WEAK char BermudaHeapMergeNode(volatile HEAPNODE *alpha, 
                                       volatile HEAPNODE *beta);

/**
 * \fn BermudaHeapNodeReturn(volatile HEAPNODE *block)
 * \brief Return a block to the list.
 * \param block Block to return.
 * \return error code
 * 
 * This will put the given block back in the linked heap list.
 */
PRIVATE WEAK char BermudaHeapNodeReturn(volatile HEAPNODE *block);

/**
 * \fn BermudaHeapUseBlock(HEAPNODE *node, HEAPNODE *prev)
 * \brief Use a memory block from the heap.
 * \param node Block which is going to be used.
 * \param prev Block in the list before <i>node</i>.
 * 
 * This block will set the magic attribute to used and remove the block from the
 * heap list.
 */
PRIVATE WEAK void BermudaHeapUseBlock(volatile HEAPNODE *node, 
                                      volatile HEAPNODE *prev);

/**
 * \fn void *BermudaHeapAlloc(size_t size)
 * \brief Allocated a given amout of memory.
 * \param size Requested memory size.
 * \return Pointer to the start of the memory block.
 * 
 * This function will search for a fitting block of memory. If no fitting block
 * is found it will return <i><b>NULL</b></i>.
 */
void *BermudaHeapAlloc(size_t size) __attribute__ ((malloc));

/**
 * \fn BermudaHeapFree(void *ptr)
 * \brief Free an allocated heap block.
 * \param ptr Block pointer to free.
 * 
 * The given pointer <i>ptr</i>, which points to <b>node+sizeof(*node)</b>, is
 * returned to the heap.
 */
void BermudaHeapFree(void *ptr);
__DECL_END

#endif /* __MEM_H__ */
