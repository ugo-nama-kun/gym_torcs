/***************************************************************************

    file                 : util.cpp
    created              : Wed May 29 22:20:24 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: util.cpp,v 1.3.2.1 2008/11/09 17:50:23 berniw Exp $

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
    @version	$Id: util.cpp,v 1.3.2.1 2008/11/09 17:50:23 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <plib/ul.h>
#include <tgfclient.h>

#include "util.h"

int
GetFilename(char *filename, char *filepath, char *buf)
{
    char	*c1, *c2;
    int		found = 0;
    int		lg;
    
    if (filepath) {
	c1 = filepath;
	c2 = c1;
	while ((!found) && (c2 != NULL)) {
	    c2 = strchr(c1, ';');
	    if (c2 == NULL) {
		sprintf(buf, "%s/%s", c1, filename);
	    } else {
		lg = c2 - c1;
		strncpy(buf, c1, lg);
		buf[lg] = '/';
		strcpy(buf + lg + 1, filename);
	    }
	    if (ulFileExists(buf)) {
		found = 1;
	    }
	    c1 = c2 + 1;
	}
    } else {
	strcpy(buf, filename);
	if (ulFileExists(buf)) {
	    found = 1;
	}
    }
    if (!found) {
	printf("File %s not found\n", filename);
	printf("File Path was %s\n", filepath);
	return 0;
    }

    return 1;
}



float
getHOT(ssgRoot *root, float x, float y)
{
  sgVec3 test_vec;
  sgMat4 invmat;
  sgMakeIdentMat4(invmat);

  invmat[3][0] = -x;
  invmat[3][1] = -y;
  invmat[3][2] =  0.0f         ;

  test_vec [0] = 0.0f;
  test_vec [1] = 0.0f;
  test_vec [2] = 100000.0f;

  ssgHit *results;
  int num_hits = ssgHOT (root, test_vec, invmat, &results);

  float hot = -1000000.0f;

  for (int i = 0; i < num_hits; i++)
  {
    ssgHit *h = &results[i];

    float hgt = - h->plane[3] / h->plane[2];

    if (hgt >= hot)
      hot = hgt;
  }

  return hot;
}
