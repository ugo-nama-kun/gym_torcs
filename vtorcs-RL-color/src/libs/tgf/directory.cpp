/***************************************************************************
                        directory.cpp -- directory management                       
                             -------------------                                         
    created              : Fri Aug 13 21:58:55 CEST 1999
    copyright            : (C) 1999 by                          
    email                : torcs@free.fr   
    version              : $Id: directory.cpp,v 1.7 2005/02/01 15:55:54 berniw Exp $                                  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
    		This is used for directory manipulation.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: directory.cpp,v 1.7 2005/02/01 15:55:54 berniw Exp $
    @ingroup	dir
*/

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgf.h>
#include "os.h"

void
gfDirInit(void)
{
}


/** Get the list of files of a given directory
    @ingroup	dir
    @param	dir	directory name
    @return	The list of files
 */
tFList * GfDirGetList(char *dir)
{
	if (GfOs.dirGetList) {
		return GfOs.dirGetList(dir);
	} else {
		return (tFList*)NULL;
	}
}


/** Get the list of files of a given directory
    @ingroup	dir
    @param	dir	directory name
    @return	The list of files
 */
tFList * GfDirGetListFiltered(char *dir, char *suffix)
{
	if (GfOs.dirGetListFiltered) {
		return GfOs.dirGetListFiltered(dir, suffix);
	} else {
		return (tFList*)NULL;
	}
}

/** Free a directory list
    @ingroup	dir
    @param	list	List of files
    @param	freeUserData	User function used to free the user data
    @return	none
*/
void GfDirFreeList(tFList *list, tfDirfreeUserData freeUserData, bool freename, bool freedispname)
{
	//tFList *cur;

	if (list) {
		// The list contains at least one element, checked above.
		tFList *rl = list;
		do {
			tFList *tmp = rl;
			rl = rl->next;
			if ((freeUserData) && (tmp->userData)) {
				freeUserData(tmp->userData);
			}
			if (freename) {
				freez(tmp->name);
			}
			if (freedispname) {
				freez(tmp->dispName);
			}
			free(tmp);
		} while (rl != list);
	}

	list = NULL;


/*
	while (list) {
		if (list->next == list) {
			if ((freeUserData) && (list->userData)) {
				freeUserData(list->userData);
			}
			free(list);
			list = NULL;
		} else {
			cur = list->next;
			list->next = cur->next;
			cur->next->prev = list;
			if ((freeUserData) && (cur->userData)) {
				freeUserData(cur->userData);
			}
			free(cur);
		}
	}
*/
}

