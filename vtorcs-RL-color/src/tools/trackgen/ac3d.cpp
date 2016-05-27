/***************************************************************************

    file                 : ac3d.cpp
    created              : Wed May 29 22:11:21 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: ac3d.cpp,v 1.1.6.1 2008/11/09 17:50:23 berniw Exp $

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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: ac3d.cpp,v 1.1.6.1 2008/11/09 17:50:23 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <tgf.h>

#include "ac3d.h"



FILE *
Ac3dOpen(char *filename, int nbObjects)
{
    FILE *save_fd;

    save_fd = fopen(filename, "wa");

    if (save_fd == NULL) {
	    GfOut("Failed to open '%s' for writing", filename);
	    return NULL;
    }

    fprintf(save_fd, "AC3Db\n");
    fprintf(save_fd, "MATERIAL \"\" rgb 0.4 0.4 0.4  amb 0.8 0.8 0.8  emis 0.4 0.4 0.4  spec 0.5 0.5 0.5  shi 50  trans 0\n");
    
    fprintf(save_fd, "OBJECT world\n");
    fprintf(save_fd, "kids %d\n", nbObjects);

    return save_fd;
}


int
Ac3dGroup(FILE *save_fd, char *name, int nbObjects)
{
    fprintf(save_fd, "OBJECT group\n");
    fprintf(save_fd, "name \"%s\"\n", name);
    fprintf(save_fd, "kids %d\n", nbObjects);

    return 0;
}

void
Ac3dClose(FILE *save_fd)
{
    fclose(save_fd);
}
