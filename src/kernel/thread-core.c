/*
 *  BermudaOS - Thread core
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

/**
 * \file src/kernel/thread-core.c Thread core module.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/thread.h>
#include <kernel/stack.h>
#include <kernel/ostimer.h>

#include <dev/dev.h>
#include <dev/error.h>

/**
 * \brief Currently running threads.
 * 
 * Each CPU core has its own running thread.
 */
static struct thread *current_thread[CPU_CORES];

/**
 * \brief Threads which are currently not runnable.
 *
 * This list contains all threads which are sleeping, waiting for an event, etc.
 */
static struct thread *waiting_queue;

/**
 * \brief Run queue tree head.
 */
static struct thread_root thread_ready_tree;

/**
 * \brief The idle thread definition.
 */
static struct thread idle_thread;
/**
 * \brief The main thread definition.
 */
static struct thread main_thread;

/**
 * \brief Thread id counter.
 */
static uint64_t thread_id_counter = 0;

/* support */
static void thread_rotate_swap_parent(struct thread_root *root, struct thread *parent, 
									struct thread *current);
static struct thread *__thread_search(struct thread *tree, uint64_t key);
static struct thread *thread_find_successor(struct thread *tree);
static struct thread *thread_find_predecessor(struct thread *tree);
static void thread_replace_node(struct thread_root *root, struct thread *orig, 
								struct thread *replacement);
static struct thread *thread_find_leftmost(struct thread *tree);
static struct thread *thread_find_rightmost(struct thread *tree);

/* insertion */
static struct thread *__thread_insert(struct thread_root *root, struct thread *node);
static void thread_validate_insertion(struct thread_root *root, struct thread *node);

/* deletion */
static int __thread_delete_node(struct thread_root *tree, struct thread *node);
static struct thread *thread_find_replacement(struct thread *tree);
static void thread_sub_deletion(struct thread_root *root, struct thread *current);

/* dbg */
#ifdef HAVE_SCHED_DBG
static void thread_dump_node(struct thread *tree, FILE *stream);
#endif

/**
 * \brief Generate a new thread ID.
 */
static inline uint64_t thread_generate_thread_id()
{
	uint64_t i = thread_id_counter;
	
	thread_id_counter += 1;
	return i;
}

/**
 * \brief Initialize the scheduler (thread core).
 * \param mstack Pointer to the main thread stack.
 * \param mstack_size Size of the main thread stack.
 * \note If \p mstack is zero the top of the RAM will be used as stack for the main thread.
 * \return Error code.
 * \retval 0 on success.
 */
PUBLIC int thread_core_init(void *mstack, size_t mstack_size)
{
	return -1;
}

/**
 * \brief Initialize a new thread information structure and its stack.
 * \param t Thread to initialize.
 * \param stack Stack pointer.
 * \param size Size of \p stack.
 * \note If \p stack is \p NULL, a stack will be allocated.
 */
PUBLIC int thread_add_new(struct thread *t, void *stack, size_t stack_size)
{
	if(stack == NULL) {
		stack = malloc(stack_size);
		return -DEV_NULL;
	}
	
	stack_init(t, stack_size, stack);
	t->id = thread_generate_thread_id();
	
	return thread_insert(&thread_ready_tree, t);
}

static void thread_sched_tick()
{
	return; /* not yet implemented */
}

/**
 * \brief Insert a thread in the given tree.
 * \param root Tree root.
 * \param node Thread to insert.
 */
PUBLIC int thread_insert(struct thread_root *root, struct thread *node)
{
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
	node->color = THREAD_RED;
	thread_validate_insertion(root, __thread_insert(root, node));
	
	return -DEV_OK;
}

/**
 * \brief Search for a thread.
 * \param root Root of the tree to search in.
 * \param key Key to search for (i.e. CPU time).
 */
PUBLIC struct thread *thread_search(struct thread_root *root, uint64_t key)
{
	return __thread_search(root->tree, key);
}

#if HAVE_RECURSION
/**
 * \brief Actual search (recursive).
 * \param tree Tree to search in.
 * \param key The key to look for.
 */
static struct thread *__thread_search(struct thread *tree, uint64_t key)
{
	if(tree == NULL) {
		return NULL;
	}
	
	if(tree->cpu_time == key) {
		return tree;
	}
	
	if(key < tree->cpu_time) {
		return __thread_search(tree->left, key);
	} else {
		return __thread_search(tree->right, key);
	}
}
#else

/**
 * \brief Actual search (non-recursive).
 * \param tree Tree to search in.
 * \param key Key to look for.
 */
static struct thread *__thread_search(struct thread *tree, uint64_t key)
{
	for(;;) {
		if(tree == NULL) {
			return NULL;
		}
		if(tree->cpu_time == key) {
			return tree;
		}
		
		if(key < tree->cpu_time) {
			tree = tree->left;
		} else {
			tree = tree->right;
		}
	}
}
#endif

/**
 * \brief Action insertion function.
 * \param root Tree root to insert into.
 * \param node Node to insert.
 * 
 * This funciton, which is called by thread_insert, does the actual insertion work.
 */
static struct thread *__thread_insert(struct thread_root *root, struct thread *node)
{
	struct thread *tree = root->tree;
	
	if(tree == NULL) {
		root->tree = node;
		/* Set the color to black. The root is always black. */
		node->color = THREAD_BLACK;
		return root->tree;
	}
	
	for(;;) {
		if(node->cpu_time <= tree->cpu_time) {
			if(tree->left == NULL) {
				tree->left = node;
				node->parent = tree;
				break;
			}
			tree = tree->left;
		} else {
			if(tree->right == NULL) {
				tree->right = node;
				node->parent = tree;
				break;
			}
			tree = tree->right;
		}
	}
	
	return node;
}

/**
 * \brief Find the left-most node in the tree.
 * \param tree Tree node to find most left node of.
 * 
 * The left-most node is the node with the lowest key value.
 */
static struct thread *thread_find_leftmost(struct thread *tree)
{
	if(!tree) {
		return NULL;
	}
	
	for(;;) {
		if(tree->left == NULL) {
			return tree;
		}
		tree = tree->left;
	}
}

/**
 * \brief Find the right-most node in the tree.
 * \param tree Tree node to find most right node of.
 * 
 * The right-most node is the node with the lowest key value.
 */
static struct thread *thread_find_rightmost(struct thread *tree)
{
	if(!tree) {
		return NULL;
	}
	
	for(;;) {
		if(tree->right == NULL) {
			return tree;
		}
		tree = tree->right;
	}
}

/**
 * \brief Find the successor of the given node.
 * \param tree Tree node to find the successor of.
 * \retval NULL if \p tree does not have a successor.
 * 
 * The successor is the node with the next highest value.
 */
static struct thread *thread_find_successor(struct thread *tree)
{
	if(!tree) {
		return NULL;
	}
	
	if(!tree->right) {
		struct thread *tmp = tree, *carriage = thread_parent(tree);
		
		while (carriage != NULL && carriage->left != tmp) {
			tmp = carriage;
			carriage = carriage->parent;
		}
		return carriage;
	}
	
	return thread_find_leftmost(tree->right);
}

/**
 * \brief Find the predecessor of a given node.
 * \param tree Node to whose predecessor is wanted.
 * \retval NULL if \p tree does not have a predecessor.
 * 
 * The predecessor is the node with the next lowest value.
 */
static struct thread *thread_find_predecessor(struct thread *tree)
{
	if(!tree) {
		return NULL;
	}
	
	if(!tree->left) {
		struct thread *tmp = tree, *carriage = thread_parent(tree);
		
		while (carriage != NULL && carriage->right != tmp) {
			tmp = carriage;
			carriage = carriage->parent;
		}
		return carriage;
	}
	return thread_find_rightmost(tree->left);
}

/**
 * \brief Rotate the tree to the left.
 * \param root Root of the tree.
 * \param tree Node to rotate around.
 * 
 * Rotate the tree counter clockwise around \p tree.
 */
static struct thread *thread_rotate_left(struct thread_root *root, struct thread *tree)
{
	struct thread *right = tree->right, *parent = thread_parent(tree);
	
	tree->right = right->left;
	right->parent = parent;
	right->left = tree;
	tree->parent = right;
	
	if(tree->right) {
		tree->right->parent = tree;
	}
	
	if(parent) {
		if(tree == parent->left) {
			parent->left = right;
			
		} else {
			parent->right = right;
		}
	} else {
		root->tree = right;
	}
	
	
	
	return right;
}

/**
 * \brief Rotate the tree to the right.
 * \param root Root of the tree.
 * \param tree Node to rotate around.
 * 
 * Rotate the tree clockwise around \p tree.
 */
static struct thread *thread_rotate_right(struct thread_root *root, struct thread *tree)
{
	struct thread *left = tree->left, *parent = thread_parent(tree);
	
	tree->left = left->right;
	left->parent = parent;
	tree->parent = left;
	left->right = tree;
	
	if(tree->left) {
		tree->left->parent = tree;
	}
	
	if(parent) {
		if(tree == parent->right) {
			parent->right = left;
		} else {
			parent->left = left;
		}
	} else {
		root->tree = left;
	}
	
	return left;
}

/**
 * \brief Validate the tree after an insertion.
 * \param root Root of the tree.
 * \param current Node which was inserted.
 * 
 * This function is called by thread_insert to fix any red-black tree violations.
 */
static void thread_validate_insertion(struct thread_root *root, struct thread *current)
{
	struct thread *x;
	
	if(current == root->tree) {
		return;
	}
	
	current = thread_parent(current);
	while(current != root->tree && current->color == THREAD_RED) {
		if((x = thread_node_has_sibling(current)) != NULL) {
			/* parent has a sibling, pull black down from the GP */
			if(x->color == THREAD_RED) {
				x->color = THREAD_BLACK;
				current->color = THREAD_BLACK;
				thread_parent(current)->color = THREAD_RED;
				current = thread_grandparent(current);
				if(!current) {
					break;
				}
				continue;
			} else {
				if(thread_parent_on_left(current) && current->left->color == THREAD_RED) {
					/* 
					* if current is the right child of its parent, rotate right. If current is on the left,
					* rotate left.
					*/
					thread_rotate_right(root, current);
					current = thread_parent(current);
				} else if(!thread_parent_on_left(current) && !current->left->color == THREAD_RED) {
					thread_rotate_left(root, current);
					current = thread_parent(current);
				}
				
				thread_rotate_swap_parent(root, thread_parent(current), current);
			}
		} else {
			thread_rotate_swap_parent(root, thread_parent(current), current);
		}
	}
	root->tree->color = THREAD_BLACK;
}

/**
 * \brief Rotate the tree in the direction that swaps the current node with its parent.
 * \param root Tree root.
 * \param parent Parent of \p current.
 * \param current Node to swap with \p parent.
 * 
 * When this function returns \p current and \p parent are swapped.
 */
static void thread_rotate_swap_parent(struct thread_root *root, struct thread *parent, 
									struct thread *current)
{
	thread_color_t tmp = parent->color;
	/* rotate in the direction that sets current as parent of pre-rotation parent */
	if(parent->right == current) {
		/* rotate to the left */
		thread_rotate_left(root, parent);
	} else {
		thread_rotate_right(root, parent);
	}
	
	parent->color = current->color;
	current->color = tmp;
}

/**
 * \brief Delete a node from the thread tree.
 * \param root Tree root.
 * \param node Node to delete.
 * \return Error code.
 */
PUBLIC int thread_delete_node(struct thread_root *root, struct thread *node)
{
	int rc = __thread_delete_node(root, node);
	if(root->tree) {
		root->tree->color = THREAD_BLACK;
	}
	return rc;
}

/**
 * \brief Deletion case enumerator.
 */
typedef enum
{
	TREE_DELETION_TERMINATE = 0,
	TREE_DELETION_CASE0, //!< Current is a red leaf.
	TREE_DELETION_CASE1, //!< Current is black with one red child.
	TREE_DELETION_CASE2, //!< Current is black and has no children.
} thread_deletion_case_t;

/**
 * \brief Sub deletion case enumerator.
 */
typedef enum
{
	TREE_SUB_DELETION_TERMINATE = 0,
	TREE_SUB_DELETION_CASE0, //!< Currents sibling is red.
	TREE_SUB_DELETION_CASE1, //!< Currents sibling is black with two black children.
	
	/**
	 * \brief Currents sibling is black with at least one red child.
	 * 
	 * Case two is executed if the far newphew is black.
	 */
	TREE_SUB_DELETION_CASE2,
	/**
	 * \brief Currents sibling is black with at least one red child.
	 * 
	 * This step is reached through step 2 or directly (i.e. the far nephew is red).
	 */
	TREE_SUB_DELETION_CASE3,
	
	/**
	 * \brief Currents sibling is black with at least one red child.
	 * 
	 * This step is reached through step 3 and this is the final rotation.
	 */
	TREE_SUB_DELETION_CASE4,
} thread_deletion_subcase_t;

/**
 * \brief Actual deletion.
 * \param root Tree root.
 * \param node Node to delete.
 * 
 * This function is called by thread_delete_node.
 */
static int __thread_delete_node(struct thread_root *root, struct thread *node)
{
	struct thread *current = node, *parent, *replacement = NULL;
	int rc = 0;
	char replace = 0;
	thread_deletion_case_t _case = TREE_DELETION_TERMINATE;
	
	successor:
	if(!current->left && !current->right) {
		if(current->color == THREAD_RED) {
			_case = TREE_DELETION_CASE0;
		} else {
			_case = TREE_DELETION_CASE2;
		}
	} else if(!(current->left && current->right) && current->color == THREAD_BLACK) {
		/* current has at most one child (and current is black) */
		_case = TREE_DELETION_CASE1;
	} else if(current->left && current->right) {
		replacement = current;
		current = thread_find_replacement(replacement);
		replace = 1;
		goto successor;
	}
	
	while(_case) {
		switch(_case) {
			case TREE_DELETION_CASE1:
				parent = thread_parent(current);
				if(parent) {
					if(parent->left == current) {
						parent->left = (current->left) ? current->left : current->right;
						parent->left->parent = parent;
						parent->left->color = THREAD_BLACK;
						
					} else {
						/* current is the right child of its parent */
						parent->right = (current->left) ? current->left : current->right;
						parent->right->parent = parent;
						parent->right->color = THREAD_BLACK;
					}
				} else {
					/* current is root */
					root->tree = (current->left) ? current->left : current->right;
					root->tree->color = THREAD_BLACK;
					root->tree->parent = NULL;
				}
				_case = TREE_DELETION_TERMINATE;
				break;
				
			case TREE_DELETION_CASE2:
				if(root->tree == current) {
					root->tree = NULL;
					_case = TREE_DELETION_TERMINATE;
					break;
				}
				/* current is NOT the root */
				thread_sub_deletion(root, current);
				_case = TREE_DELETION_TERMINATE;
				break;
				
			case TREE_DELETION_CASE0:
			default:
				parent = thread_parent(current);
				if(parent) {
					if(parent->left == current) {
						parent->left = NULL;
					} else {
						parent->right = NULL;
					}
				}
				_case = TREE_DELETION_TERMINATE;
				break;
		}
	}

	if(replace) {
		thread_replace_node(root, replacement, current);
	}
	
	return rc;
}

/**
 * \brief Sub deletion routine.
 * \param root Tree root.
 * \param current Currently selected node.
 * 
 * This function is called on the hard cases (i.e. \p current is black with at most one child).
 */
static void thread_sub_deletion(struct thread_root *root, struct thread *current)
{
	thread_deletion_subcase_t _case = TREE_SUB_DELETION_TERMINATE;
	struct thread *sibling, *fn;
	thread_color_t tmp;
	
	sibling = thread_node_has_sibling(current);
	fn = thread_node_far_nephew(current);
	
	/* unhook current from the tree */
	if(current->parent->left == current) {
		current->parent->left = NULL;
	} else {
		current->parent->right = NULL;
	}
	
	do {
		if(sibling != NULL) {
			if(sibling->color == THREAD_RED) {
				_case = TREE_SUB_DELETION_CASE0;
			} else if((!sibling->left || sibling->left->color == THREAD_BLACK) && 
				(!sibling->right || sibling->right->color == THREAD_BLACK)) {
				_case = TREE_SUB_DELETION_CASE1;
			} else if(sibling->color == THREAD_BLACK) {
				if((sibling->left && sibling->left->color == THREAD_RED) || 
					(sibling->right && sibling->right->color == THREAD_RED)) {
					_case = (!fn || fn->color == THREAD_BLACK) ? TREE_SUB_DELETION_CASE2 : 
																		TREE_SUB_DELETION_CASE3;
				}
			}
		}
		
		switch(_case) {
			case TREE_SUB_DELETION_CASE0:
				/* xchn colors of sibling and its parent */
				tmp = sibling->parent->color;
				sibling->parent->color = sibling->color;
				sibling->color = tmp;
				
				/* rotate clock wise if sibling is on the left side of current */
				if(sibling->parent->left == sibling) {
					thread_rotate_right(root, sibling->parent);
					sibling = current->parent->left;
					fn = sibling->left;
				} else {
					thread_rotate_left(root, sibling->parent);
					sibling = current->parent->right;
					fn = sibling->right;
				}
				break;
				
			case TREE_SUB_DELETION_CASE1:
				sibling->color = THREAD_RED;
				current = sibling->parent;
				if(current->color == THREAD_BLACK) {
					sibling = thread_node_has_sibling(current);
					fn = thread_node_far_nephew(current);
					break;
				} else {
					current->color = THREAD_BLACK;
					_case = TREE_SUB_DELETION_TERMINATE;
					break;
				}
				
			case TREE_SUB_DELETION_CASE2:
				if(fn == sibling->left) {
					fn = sibling->right;
					thread_rotate_left(root, sibling);
					sibling = fn;
					fn = sibling->left;
				} else {
					fn = sibling->left;
					thread_rotate_right(root, sibling);
					sibling = fn;
					fn = sibling->right;
				}
				/* roll through */
			
			case TREE_SUB_DELETION_CASE3:
				fn->color = THREAD_BLACK;
				sibling->color = sibling->parent->color;
				sibling->parent->color = THREAD_BLACK;
				/* roll through */
			case TREE_SUB_DELETION_CASE4:
				if(current->parent->right == sibling) {
					thread_rotate_left(root, current->parent);
				} else {
					thread_rotate_right(root, current->parent);
				}
				_case = TREE_SUB_DELETION_TERMINATE;
				break;
				
			default:
				_case = TREE_SUB_DELETION_TERMINATE;
				break;
		}
	} while(_case);
}

/**
 * \brief Find a replacement for a node.
 * \param tree Node to find a replacer for.
 * \return Either the successor or the predecessor if has neither (i.e. \p tree is the root), \p NULL
 *         is returned.
 */
static struct thread *thread_find_replacement(struct thread *tree)
{
	struct thread *successor = thread_find_successor(tree);
	if(!successor) {
		return NULL;
	}
	
	if(successor->color == THREAD_RED || !(successor->color == THREAD_BLACK && successor->left == NULL 
		&& successor->right == NULL)) {
		return successor;
	} else {
		return thread_find_predecessor(tree);
	}
}

/**
 * \brief Replace a node with another node.
 * \param root Tree root.
 * \param orig Node to be replaced.
 * \param replacement The node replacing \p orig.
 */
static void thread_replace_node(root, orig, replacement)
struct thread_root *root;
struct thread *orig;
struct thread *replacement;
{
	replacement->left = orig->left;
	replacement->right = orig->right;
	replacement->parent = orig->parent;
	
	/* fix the parent pointers of origs children */
	if(orig->left) {
		orig->left->parent = replacement;
	}
	if(orig->right) {
		orig->right->parent = replacement;
	}
	/* fix the pointer of origs parent */
	if(orig->parent) {
		if(orig->parent->left == orig) {
			orig->parent->left = replacement;
		} else {
			orig->parent->right = replacement;
		}
	} else if(orig == root->tree) {
		root->tree = replacement;
	}
	replacement->color = orig->color;
}

#ifdef HAVE_SCHED_DBG
PUBLIC void thread_dump(struct thread *tree, FILE *stream)
{
	thread_dump_node(tree, stream);
	fputc('\n', stream);
}

static void thread_dump_node(struct thread *tree, FILE *stream)
{
	if (tree == NULL)
	{
		printf("null");
		return;
	}

	printf("d:[");
	printf("%u,%s,%u", tree->cpu_time, (tree->color == THREAD_RED) ? "RED" : "BLACK", 
		   (tree->parent != NULL) ? tree->parent->cpu_time : -1);
	
	printf("]");
	if (tree->left != NULL)
	{
		printf("l:[");
		thread_dump_node(tree->left, stream);
		printf("]");
	}
	if (tree->right != NULL)
	{
		printf("r:[");
		thread_dump_node(tree->right, stream);
		printf("]");
	}
}

PUBLIC void thread_cleanup(struct thread *root)
{
	struct thread *node = root->left, *tmp;
	
	for(;;) {
		if(node) {
			tmp = node->left;
			free(node);
			node = tmp;
		} else {
			break;
		}
	}
	
	/* repeat for right */
	node = root->right;
	for(;;) {
		if(node) {
			tmp = node->right;
			free(node);
			node = tmp;
		} else {
			break;
		}
	}
	
	free(root);
}
#endif
