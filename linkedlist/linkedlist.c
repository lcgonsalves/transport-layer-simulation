#include <stdlib.h>
#include "linkedlist.h"

///////// SEE HEADER FOR INFO ON EACH METHOD \\\\\\\\\\\\\

linkedlist* init_linkedlist()
{
	linkedlist* list = (linkedlist*) malloc(sizeof(linkedlist));
	list.head = NULL;
	list.tail = NULL;
	return list;
}

void term_linkedlist(linkedlist* list)
{
	// if both head and tail are empty, free and return
	if (!list.head && !list.tail)
	{
		free(list);
		return;
	}

	// iterate through list and free each node
	node* current = list.head;
	node* rm;

	while (current)
	{
		rm = current;
		current = current.next;
		free(rm);
	}

	// at this point, all nodes have been freed
	free(list);
}

PAYLOAD_TYPE pop(linkedlist* list)
{
	PAYLOAD_TYPE result;

	if (!list.head)
	{	// no head means list is empty
		return NULL;
	}
	else if (list.head == list.tail)
	{	// head and tail are the same, only one item in list
		result = list.head.payload;
		free(list.head);
		list.head = list.tail = NULL;
		return result;
	}
	else
	{
		// it's arbitrarily long
		result = list.head.payload;
		node* temp = list.head;
		list.head = list.head.next;
		free(temp);
		return result;
	}
}

void insert_end(linkedlist* list, PAYLOAD_TYPE payload)
{
	node* ins = (node*) malloc(sizeof(node));
	ins.payload = payload;
	ins.next = NULL;

	// case 1 empty list
	if (!list.head)
	{
		// both tail and head will be the node
		list.head = list.tail = ins;
	} // case 2 non empty list
	else
	{
		list.tail.next = ins;
		list.tail = ins;
	}
}