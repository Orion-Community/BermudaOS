/*
 *  BermudaOS - XOR linked list library
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

/*
 * Taken from https://github.com/bietje/libXORlist/
 * Maintained by bietje AKA Michel Megens.
 */

#include <stdlib.h>
#include <bermuda.h>

#include <lib/list/xorlist.h>
#include <lib/list/xorlist_error.h>

/**
 * \fn BermudaXorllGetNext(NODE *prev, NODE *this)
 * \return The next node
 * \brief Calculates the next node.
 *
 * Uses the XOR functionality to calculate the next node from the previous and
 * the next pointer.
 */
NODE *BermudaXorllGetNext(NODE *prev, NODE *this)
{
        if(this == NULL)
                return this;
        ulong uprev = (ulong)prev;
        ulong uthis = (ulong)((this) ? this->pointer : NULL);

        ulong next = uprev ^ uthis;

        return (NODE*)next;
}

/**
 * \fn BermudaXorllInsert(NODE *prev, NODE *this, NODE *new)
 * \brief Adds <b>node</b> to <b>list</b>.
 * \param prev The previous node
 * \param this The node will be after this node.
 * \param new The list node to add to the list.
 * \warning <i>prev</i> and <i>this</i> MUST be consecutive nodes.
 *
 * This function will insert the xornode node between alpha and beta.
 */
int BermudaXorllInsert(NODE *prev, NODE *this, NODE *new)
{
        ulong uprev = (ulong)prev;
        ulong uthis = (ulong)this;
        ulong unew  = (ulong)new;
        NODE *next  = BermudaXorllGetNext(prev, this);
        ulong unext = (ulong)next;

        ulong pNext_next = (next) ? (ulong)next->pointer ^ uthis : 0 ^ uthis;
        
        if(NULL == next)
        {
                this->pointer = (void*)(uprev ^ unew);
                new->pointer = (NODE*)(uthis ^ 0);
                return OK;
        }

        /* set the node pointer of this */
        this->pointer = (NODE*)(uprev ^ unew);
        new->pointer = (NODE*)(uthis ^ unext);
        next->pointer = (NODE*)(pNext_next ? unew ^ pNext_next : unew ^ 0);
        return OK;
}

/**
 * \fn BermudaXorllRemoveNode(NODE *prev, NODE *this)
 * \param prev Previous node of <i>this</i>
 * \param this Node which has to be removed.
 * \brief Remove node <i>this</i> from the list.
 * 
 * BermudaXorllRemoveNode removes node <i>this</i> from the linked list.
 */
int BermudaXorllRemoveNode(NODE *prev, NODE *this)
{
        if(NULL == this)
                return NULL_PTR;
        
        NODE *next = BermudaXorllGetNext(prev, this);
        ulong uprev_prev = (ulong) ((prev) ? BermudaXorllGetPrev(prev, this) : NULL);
        
        ulong unext = (ulong)next;
        ulong uprev = (ulong)prev;
        ulong uthis = (ulong)this;
        ulong unext_next = (ulong) ((next) ? (ulong)next->pointer ^ uthis : 0);
        
        if(NULL != prev)
                prev->pointer = (void*)(uprev_prev ^ unext);
        
        if(NULL != next)
                next->pointer = (void*)(unext_next ^ uprev);
        this->pointer = NULL;
//         printf("%x\n", next->pointer);
        return OK;
}

/**
 * \fn BermudaXorllAddNode(NODE *list, NODE *node, NODE *new)
 * \brief Add the node <i>new</i> to <i>list</i>.
 * \param list The list head.
 * \param node The node to add the new node after.
 * \param new The node to add after <i>node</i>.
 * \return ERROR code.
 *
 * The xornode <i>new</i> will be added after <i>node</i> in the list
 * <i>list</i>.
 */
int BermudaXorllAddNode(NODE *listHead, NODE *node, NODE *new)
{
        NODE *prev = NULL,*carriage = listHead, *tmp;

        if(!new)
                return NULL_PTR;
        while(carriage)
        {
                if(carriage == node)
                {
                        BermudaXorllInsert(prev, carriage, new);
                        break;
                }
                tmp = carriage;
                carriage = BermudaXorllGetNext(prev, tmp);                
                prev = tmp;
                
                if(!carriage && !node && new)
                {
                        carriage = tmp;
                        prev = BermudaXorllGetPrev(carriage, NULL);
                        BermudaXorllInsert(prev, carriage, new);
                        break;
                }
        }
        
        return OK;
}

/**
 * \fn BermudaXorllIterateList(NODE *prev, NODE *head, xor_list_iterator_t hook)
 * \param prev Previous node of the starting point <i>head</i>
 * \param head Iterate starting point.
 * \param hook Will be called every iteration.
 * \return An error code.
 * \brief Iterates trough a XOR linked list.
 *
 * This function returns trough a XOR-linkedlist and it will call hook on every
 * iteration.
 */
int
BermudaXorllIterateList(NODE *prev, NODE *head, xor_list_iterator_t hook)
{
        NODE *carriage = head, *tmp;
        int result = -1;

        if(!hook)
                return NULL_PTR;
        while(carriage)
        {
                tmp = carriage; // save to set prev later
                carriage = BermudaXorllGetNext(prev, tmp); // get next one..
                if(prev)
                        if(HOOK_DONE == (result = hook(prev)))
                                break;
                prev = tmp;
        }
        result = hook(prev);

        return result;
}
