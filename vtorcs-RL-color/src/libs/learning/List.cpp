/* -*- Mode: C++;  -*- */
/* VER: $Id: List.cpp,v 1.3 2005/08/05 09:02:58 berniw Exp $ */
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <learning/List.h>
#include <learning/learn_debug.h>

LIST* List(void)
{
	LIST* list = NULL;

	if ((list = (LIST*) malloc(sizeof(LIST)))==NULL) {
		Serror("Failed to create list structure\n");
		return NULL;
	}

	list->head = NULL;
	list->tail = NULL;
	list->curr = NULL;
	list->n = 0;
	list->retrieve = &ListLinearSearchRetrieve;

	return list;
}

LISTITEM* ListAppend(LIST* list, void* p) {
	return ListAppend (list, p, NULL);
}

LISTITEM* ListAppend(LIST* list, void* p, void (*free_obj) (void* obj))
{
	LISTITEM* tmp = NULL;
	assert(list);

	if (!p) {
		Swarning("NULL pointer given for new list item data\n");
	}
  
	if (!list->head) {
		tmp = ListItem(p, free_obj);
		list->head = tmp;
		list->curr = tmp;
	} else {
		tmp = LinkNext(list->tail, p, free_obj);
	}

	list->tail = tmp;
  
	list->n++;
  
	assert(list->head);
	assert(list->curr);
	assert(list->tail);
  
	return tmp;
}


LISTITEM* NextListItem(LIST* list)
{
	LISTITEM* t;
	assert(list);
  
	if (!list->curr) 
		return NULL;

	t = GetNextItem(list->curr);
	if (t)
		list->curr =t;
	return t;
}

LISTITEM* FirstListItem(LIST* list)
{
	LISTITEM* t;
	assert(list);
  
	t = list->head;

	if (!t) {
		//    Swarning("No First Item\n");
		return NULL;
	}
  
	list->curr = t;
	return t;
}


LISTITEM* LastListItem(LIST* list)
{
	LISTITEM* t;
	assert(list);
  
	t = list->tail;

	if (!t) {
		//    Swarning("No Last Item\n");
		return NULL;
	}
  
	list->curr = t;
	return t;
}


LISTITEM* GetNextItem(LISTITEM* ptr)
{
	if (ptr)
		return ptr->next;
	else {
		Serror("Null pointer given to GetNextItem()\n");
		return NULL;
	}

}

LISTITEM* GetPrevItem(LISTITEM* ptr)
{
	if (ptr)
		return ptr->prev;
	else {
		Serror("Null pointer given to GetPrevItem()\n");
		return NULL;
	}
}

LISTITEM* LinkNext(LISTITEM* src, void* ptr, void (*free_obj) (void* obj))
{
	LISTITEM* tmp;
	LISTITEM* dst = NULL;

	assert(ptr);
	assert(src);
  
	if ((dst = ListItem(ptr, free_obj))==NULL) {
		return NULL;
	}

	if ((tmp = GetNextItem(src))) {
		tmp->prev = dst;
	}
	dst->next = tmp;
	dst->prev = src;
	src->next = dst;  

	return dst;
}

LISTITEM* LinkPrev(LISTITEM* src, void* ptr, void (*free_obj) (void* obj))
{
	Serror("Not implemented\n");
	return NULL;
}

LISTITEM* ListItem(void* ptr, void (*free_obj) (void* obj)) 
{
	LISTITEM* item = NULL;

	assert(ptr);

	if ((item = (LISTITEM*) malloc(sizeof(LISTITEM)))==NULL) {
		Serror("Failed to allocate new listitem\n");
		return NULL;
	}
  
	item->prev = NULL;
	item->next = NULL;
	item->obj = ptr;
	item->free_obj = free_obj;
	return item;
}

int FreeListItem(LIST* list, LISTITEM* ptr)
{
	if (ptr==NULL) {
		Serror("Null value for LISTITEM\n");
		return -1;
	}

	if (ptr->obj) {
		if (ptr->free_obj) {
			ptr->free_obj(ptr->obj);
		} else {
			free(ptr->obj);
		}
	}

	return RemoveListItem(list, ptr);
}


int RemoveListItem(LIST* list, LISTITEM* ptr) {
	LISTITEM* prev;
	LISTITEM* next;

	assert(ptr);

	prev = GetPrevItem(ptr);
	next = GetNextItem(ptr);
  
	if (prev) {
		if (prev->next != ptr) {
			Swarning("prev->next Sanity check failed on list\n");
		}
		prev->next = next;
		if (next==NULL) {
			assert (list->tail == ptr);
			list->tail = prev;
			if (list->curr == ptr) {
				list->curr = prev;
			}
		}
	}
  
	if (next) {
		if (next->prev != ptr) {
			Swarning("next->prev Sanity check failed on list\n");
		}
		next->prev = prev;
		if (prev==NULL) {
			assert (list->head == ptr);
			list->head = next;
			if (list->curr == ptr) {
				list->curr = next;
			}
		}
	}

	if ((next==NULL)&&(prev==NULL)) {
		assert (list->tail==list->head);
		list->tail = NULL;
		list->head = NULL;
		list->curr = NULL;
	}

	free(ptr);
	return 0;
 
}



int PopItem(LIST* list) {

	LISTITEM* head = list->head;

	if (list->head==NULL) {
		Swarning("List already empty\n");
		return -1;
	}

	if (FreeListItem(list, head))
		return -1;

	list->n--;

	if (list->head==NULL) {
		if (list->n) {
			Swarning("List seems empty (%d items remaining?)",list->n);  
		}
	} else {
		assert(list->curr);
		assert(list->tail);
		if (list->head==NULL) {
			Serror ("List already empty\n");
		}
		/* set tail to head if only one item is remaining */
		if (list->head->next==NULL) {
			assert(list->n==1);
			list->tail = list->head;
		}
		if (list->n<=0) {
			Serror("Counter at %d, yet least not empty?\n",list->n);
			return -1;
		}
	}

	return 0;

}

int ClearList(LIST* list)
{
	int i;
	while (list->head) {
		PopItem(list);
	}
	i = list->n;

	if (i==0) {
		if (list->head) {
			Serror("List still has a head after clearing\n");
		}
		if (list->curr) {
			Serror("List still points somewhere after clearing\n");
		}
		if (list->tail) {
			Serror("List still has a tail after clearing\n");
		}
	} else {
		Serror("List size not zero after clearing\n");
	}

	free (list);

	return i;
}

LISTITEM* FindItem (LIST* list, void* ptr)
{
	return list->retrieve (list, ptr);
}


LISTITEM* ListLinearSearchRetrieve (struct List* list, void* ptr)
{
	LISTITEM* item;

	item = FirstListItem (list);
	while (item) {
		if (item->obj == ptr) {
			return item;
		}
		item = NextListItem (list);
	}

	return NULL;
}


/* Get the size of the list */
int ListSize(LIST* list) {
	return list->n;
}

LISTITEM* GetItem (LIST* list, int n)
{
	LISTITEM* item;

	if (n>=ListSize (list)) {
		return NULL;
	}
	item = FirstListItem (list);
	for (int i=0; i<n; i++) {
		item = NextListItem (list);
	}

	return item;
}
