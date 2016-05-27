/* -*- Mode: C++; -*- */
/* VER: $Id: List.h,v 1.2 2005/07/19 23:50:02 olethros Exp $ */
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/// A list item
typedef struct ListItem {
	void* obj; ///< data
	void (*free_obj) (void* obj); ///< free hook
	struct ListItem* prev; ///< previous item
	struct ListItem* next; ///< next item
} LISTITEM;


/**
   \brief A very simple list structure
   
   The structure is initialised empty. The search function pointer is
   ListLinearSearchRetrieve.
   \return NULL if nothing could be
   created.
   \see ListLinearSearchRetrieve
*/
typedef struct List {
	LISTITEM* curr; ///< current item
	LISTITEM* head; ///< head item
	LISTITEM* tail; ///< tail item
	int n; ///< number of items
	/// Method by which to search objects
	LISTITEM* (*retrieve) (struct List* list, void* ptr);
} LIST;


/// Create a new list
LIST* List(void);
/// Get the size of the list
int ListSize(LIST* list);
/// Append an item to the list
LISTITEM* ListAppend(LIST* list, void* p);
/// Append an item to the list with free hook
LISTITEM* ListAppend(LIST* list, void* p, void (*free_obj) (void* obj));
/// Move to the first list item
LISTITEM* FirstListItem(LIST* list);
/// Move to the last list item
LISTITEM* LastListItem(LIST* list);
/// Advance one item
LISTITEM* NextListItem(LIST* list);
/// Remove the topmost item of the list (also frees obj memory)
int PopItem(LIST* list);
/// Clear the list
int ClearList(LIST* list);
/// Finds the LISTITEM pointer corresponding to the data
LISTITEM* FindItem (LIST* list, void* ptr);
/// Get the nth item of the list
LISTITEM* GetItem (LIST* list, int n);

LISTITEM* ListItem(void* ptr, void (*free_obj) (void* obj));
LISTITEM* GetNextItem(LISTITEM* ptr);
LISTITEM* GetPrevItem(LISTITEM* ptr);
LISTITEM* LinkNext(LISTITEM* src, void* ptr, void (*free_obj) (void* obj));
LISTITEM* LinkPrev(LISTITEM* src, void* ptr, void (*free_obj) (void* obj));
int FreeListItem(LIST* list, LISTITEM* ptr);
int RemoveListItem(LIST* list, LISTITEM* ptr);

LISTITEM* ListLinearSearchRetrieve (struct List* list, void* ptr);

#endif
