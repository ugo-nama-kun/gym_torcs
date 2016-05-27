/***************************************************************************
                        guilabel.cpp -- labels management                           
                             -------------------                                         
    created              : Fri Aug 13 22:22:12 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: guilabel.cpp,v 1.2.2.1 2008/11/09 17:50:22 berniw Exp $                                  
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
    		GUI labels management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: guilabel.cpp,v 1.2.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	gui
*/

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include "gui.h"
#include "guifont.h"

void
gfuiLabelInit(void)
{
}

/** Create a new label (extended version).
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	text	Text of the label
    @param	fgColor	Pointer on color static array (RGBA)
    @param	font	Font id
    @param	x	Position of the label on the screen
    @param	y	Position of the label on the screen
    @param	align	Alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical top
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    			<br>0 for the text length.
    @return	label Id
    @see	GfuiSetLabelText
 */
int 
GfuiLabelCreateEx(void *scr, char *text, float *fgColor, int font, int x, int y, int align, int maxlen)
{
    tGfuiLabel	*label;
    tGfuiObject	*object;
    int 	width;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_LABEL;
    object->focusMode = GFUI_FOCUS_NONE;
    object->visible = 1;
    object->id = screen->curId++;
    
    if (maxlen == 0) maxlen = strlen(text);
    label = &(object->u.label);
    label->text = (char*)calloc(maxlen+1, 1);
    strncpy(label->text, text, maxlen);
    label->maxlen = maxlen;
    
    label->bgColor = screen->bgColor;
    label->fgColor = fgColor;

    label->font = gfuiFont[font];
    width = gfuiFont[font]->getWidth((const char *)text);
    label->align = align;
    switch(align&0xF0) {
    case 0x00 /* LEFT */:
	label->x = object->xmin = x;
	label->y = y - gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x + width;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    case 0x10 /* CENTER */:
	label->x =  object->xmin = x - width / 2;
	label->y = y - gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x + width / 2;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    case 0x20 /* RIGHT */:
	label->x = object->xmin = x - width;
	label->y = y - gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    }

    gfuiAddObject(screen, object);

    return object->id;
}

/** Add a label to a screen.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	text	Text of the label
    @param	font	Font id
    @param	x	Position of the label on the screen
    @param	y	Position of the label on the screen
    @param	align	Alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical top
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    			<br>0 for the text length.
    @return	label Id
    @see	GfuiSetLabelText
 */
int
GfuiLabelCreate(void *scr, char *text, int font, int x, int y, int align, int maxlen)
{
    return GfuiLabelCreateEx(scr, text, &(GfuiColor[GFUI_LABELCOLOR][0]), font, x, y, align, maxlen);
}

/** Add a Tip (generally associated with a button).
    @param	scr	Screen where to add the label
    @param	text	Text of the label
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    @return	label Id
    @see	GfuiSetLabelText
 */
int
GfuiTipCreate(void *scr, char *text, int maxlen)
{
    return GfuiLabelCreateEx(scr, text, &(GfuiColor[GFUI_TIPCOLOR][0]), GFUI_FONT_SMALL, 320, 15, GFUI_ALIGN_HC_VB, maxlen);
}

/** Add a Title to the screen.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	text	Text of the title
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    			<br>0 for the text length.
    @return	label Id
    @see	GfuiSetLabelText
 */
int
GfuiTitleCreate(void *scr, char *text, int maxlen)
{
    return GfuiLabelCreateEx(scr, text, &(GfuiColor[GFUI_TITLECOLOR][0]), GFUI_FONT_BIG, 320, 440, GFUI_ALIGN_HC_VB, maxlen);
}

void
gfuiSetLabelText(tGfuiObject *curObject, tGfuiLabel *label, char *text)
{
    int		pw, w;

    if (!text) {
	return;
    }
    pw = label->font->getWidth((const char *)label->text);
    strncpy(label->text, text, label->maxlen);
    w = label->font->getWidth((const char *)text);
    switch(label->align&0xF0) {
    case 0x00 /* LEFT */:
	curObject->xmax = label->x + w;
	break;
    case 0x10 /* CENTER */:
	label->x = curObject->xmin = label->x + pw / 2 - w / 2;
	curObject->xmax = curObject->xmax - pw / 2 + w / 2;
	break;
    case 0x20 /* RIGHT */:
	label->x = curObject->xmin = curObject->xmax - w;
	break;
    }
}

/** Change the text of a label.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	id	Id of the label
    @param	text	Text of the label
    @attention	The maximum length is set at the label creation
    @see	GfuiAddLabel
 */
void
GfuiLabelSetText(void *scr, int id, char *text)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_LABEL) {
		      gfuiSetLabelText(curObject, &(curObject->u.label), text);
		}
		return;
	    }
	} while (curObject != screen->objects);
    }
}

/** Change the color of a label.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	id	Id of the label
    @param	color	an array of 4 floats (RGBA)
    @see	GfuiAddLabel
 */
void
GfuiLabelSetColor(void *scr, int id, float *color)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_LABEL) {
		     curObject->u.label.fgColor = color;
		}
		return;
	    }
	} while (curObject != screen->objects);
    }
}


void
gfuiDrawLabel(tGfuiObject *obj)
{
    tGfuiLabel	*label;

    label = &(obj->u.label);
    if (label->bgColor[3] != 0.0) {
	glColor4fv(label->bgColor);
	glBegin(GL_QUADS);
	glVertex2i(obj->xmin, obj->ymin);
	glVertex2i(obj->xmin, obj->ymax);
	glVertex2i(obj->xmax, obj->ymax);
	glVertex2i(obj->xmax, obj->ymin);
	glEnd();
    }
    glColor4fv(label->fgColor);
    gfuiPrintString(label->x, label->y, label->font, label->text);

}

void
gfuiReleaseLabel(tGfuiObject *obj)
{
    tGfuiLabel	*label;

    label = &(obj->u.label);

    free(label->text);
    free(obj);
}
