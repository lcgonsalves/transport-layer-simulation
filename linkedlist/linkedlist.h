#ifndef LINKEDLIST_H
#define LINKEDLIST_H

/****************************************************
*													*
*													*
*	A LinkedList implementation by Leo G.			*
*													*
****************************************************/

// change this to modify the type of the payload
#define PAYLOAD_TYPE struct msg*

// definition of a node with a payload pointer
typedef struct node {
	PAYLOAD_TYPE payload;
	struct node* next;
} node;

// 
typedef struct linkedlist {
	node* head;
	node* tail;
} linkedlist;

/**
*	Initializes empty linked list by dyn allocation of memory.
*	@returns the pointer to the newly allocated linkedlist
*/
linkedlist* init_linkedlist();

/** Terminates linked list by iterating through items and freeing each item, then
* freeing original pointer.
*
*  WARNING: does not free payload pointers.
*/
void term_linkedlist(linkedlist* list);

/**
*	Inserts an object of type PAYLOAD_TYPE defined above into the end of the list.
*/
void insert_end(linkedlist* list, PAYLOAD_TYPE payload);

/**
*	Returns payload on HEAD of list, removes node, and moves HEAD to next.
*/
PAYLOAD_TYPE pop(linkedlist* list);

#endif /* LINKEDLIST_H */
