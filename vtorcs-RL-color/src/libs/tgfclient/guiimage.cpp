/***************************************************************************

    file                 : guiimage.cpp
    created              : Wed May  1 10:29:28 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: guiimage.cpp,v 1.3 2005/02/01 15:55:55 berniw Exp $

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
    		GUI Images management
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: guiimage.cpp,v 1.3 2005/02/01 15:55:55 berniw Exp $
    @ingroup	gui
*/

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include "gui.h"


/** Create a new static image.
    This kind of image is not clickable.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	x	Position of the left of the image on the screen
    @param	y	Position of the bottom of the image on the screen
    @param	w	Width of the image on the screen
    @param	h	Height of the image on the screen
    @param	name	Filename on the image (png)
    @return	Image Id
		<br>-1 Error
    @warning	the image must be sqare and its size must be a power of 2.
*/
int GfuiStaticImageCreate(void *scr, int x, int y, int w, int h, char *name)
{
	tGfuiImage *image;
	tGfuiObject *object;
	tGfuiScreen *screen = (tGfuiScreen*)scr;

	object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
	object->widget = GFUI_IMAGE;
	object->focusMode = GFUI_FOCUS_NONE;
	object->visible = 1;
	object->id = screen->curId++;

	image = &(object->u.image);
	image->texture = GfImgReadTex(name);

	if (!image->texture) {
		free(object);
		return -1;
	}

	object->xmin = x;
	object->xmax = x + w;
	object->ymin = y;
	object->ymax = y + h;

	gfuiAddObject(screen, object);

	return object->id;
}

/** Replace an image by another one.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	id	Image Id
    @param	name	Filename on the image (png)
    @return	none
    @warning	the image must be sqare and its size must be a power of 2.
*/
void GfuiStaticImageSet(void *scr, int id, char *name)
{
	tGfuiObject *curObject;
	tGfuiScreen *screen = (tGfuiScreen*)scr;
	tGfuiImage *image;

	curObject = screen->objects;
	if (curObject != NULL) {
		do {
			curObject = curObject->next;
			if (curObject->id == id) {
				if (curObject->widget == GFUI_IMAGE) {
					image = &(curObject->u.image);
					GfImgFreeTex(image->texture);
					image->texture = GfImgReadTex(name);
				}
			return;
			}
		} while (curObject != screen->objects);
	}
}

void
gfuiReleaseImage(tGfuiObject *obj)
{
	tGfuiImage	*image;

	image = &(obj->u.image);
	GfImgFreeTex(image->texture);
	free(obj);
}

void
gfuiDrawImage(tGfuiObject *obj)
{
    tGfuiImage	*image;

    image = &(obj->u.image);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, image->texture);

    glBegin(GL_TRIANGLE_STRIP);
    {
	glColor4f(1.0, 1.0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex2f(obj->xmin, obj->ymin);
	glTexCoord2f(0.0, 1.0); glVertex2f(obj->xmin, obj->ymax);
	glTexCoord2f(1.0, 0.0); glVertex2f(obj->xmax, obj->ymin);
	glTexCoord2f(1.0, 1.0); glVertex2f(obj->xmax, obj->ymax);
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}
