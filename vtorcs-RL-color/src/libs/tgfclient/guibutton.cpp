/***************************************************************************
                             guibutton.cpp                             
                             -------------------                                         
    created              : Fri Aug 13 22:18:21 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: guibutton.cpp,v 1.3.2.1 2008/11/09 17:50:22 berniw Exp $                                  
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
    		GUI Buttons Management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: guibutton.cpp,v 1.3.2.1 2008/11/09 17:50:22 berniw Exp $
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
gfuiButtonInit(void)
{
}

/** Add a graphical button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	disabled	filename of the image when the button is disabled
    @param	enabled		filename of the image when the button is enabled
    @param	focused		filename of the image when the button is focused
    @param	pushed		filename of the image when the button is pushed
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	align		Button alignment
    @param	mouse		Mouse behavior:
    				<br>GFUI_MOUSE_UP Action performed when the mouse right button is released
				<br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
int
GfuiGrButtonCreate(void *scr, char *disabled, char *enabled, char *focused, char *pushed,
		   int x, int y, int align, int mouse,
		   void *userDataOnPush, tfuiCallback onPush, 
		   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiGrButton	*button;
    tGfuiObject		*object;
    tGfuiScreen		*screen = (tGfuiScreen*)scr;
    int			width, height;
    
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_GRBUTTON;
    object->focusMode = GFUI_FOCUS_MOUSE_MOVE;
    object->id = screen->curId++;
    object->visible = 1;
    
    button = &(object->u.grbutton);
    button->state = GFUI_BTN_RELEASED;
    button->userDataOnPush = userDataOnPush;
    button->onPush = onPush;
    button->userDataOnFocus = userDataOnFocus;
    button->onFocus = onFocus;
    button->onFocusLost = onFocusLost;
    button->mouseBehaviour = mouse;

    button->disabled = GfImgReadPng(disabled, &width, &height, 1.0);
    button->enabled = GfImgReadPng(enabled, &width, &height, 1.0);
    button->focused = GfImgReadPng(focused, &width, &height, 1.0);
    button->pushed = GfImgReadPng(pushed, &width, &height, 1.0);

    switch (align) {
    case GFUI_ALIGN_HR_VB:
	object->xmin = x - width;
	object->xmax = x;
	object->ymin = y;
	object->ymax = y + height;
	break;
    case GFUI_ALIGN_HR_VC:
	object->xmin = x - width;
	object->xmax = x;
	object->ymin = y - height / 2;
	object->ymax = y + height / 2;
	break;
    case GFUI_ALIGN_HR_VT:
	object->xmin = x - width;
	object->xmax = x;
	object->ymin = y - height;
	object->ymax = y;
	break;
    case GFUI_ALIGN_HC_VB:
	object->xmin = x - width / 2;
	object->xmax = x + width / 2;
	object->ymin = y;
	object->ymax = y + height;
	break;
    case GFUI_ALIGN_HC_VC:
	object->xmin = x - width / 2;
	object->xmax = x + width / 2;
	object->ymin = y - height / 2;
	object->ymax = y + height / 2;
	break;
    case GFUI_ALIGN_HC_VT:
	object->xmin = x - width / 2;
	object->xmax = x + width / 2;
	object->ymin = y - height;
	object->ymax = y;
	break;
    case GFUI_ALIGN_HL_VB:
	object->xmin = x;
	object->xmax = x + width;
	object->ymin = y;
	object->ymax = y + height;
	break;
    case GFUI_ALIGN_HL_VC:
	object->xmin = x;
	object->xmax = x + width;
	object->ymin = y - height / 2;
	object->ymax = y + height / 2;
	break;
    case GFUI_ALIGN_HL_VT:
	object->xmin = x;
	object->xmax = x + width;
	object->ymin = y - height;
	object->ymax = y;
	break;
    default:
	break;
    }

    button->width = width;
    button->height = height;

    gfuiAddObject(screen, object);
    return object->id;
}

/** Add a state button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	text		Button label
    @param	font		Font id
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	width		width of the button (0 = text size)
    @param	align		Button alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VC	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VT	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VC	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VT	horizontal left, vertical top
    @param	mouse		Mouse behavior:
    				<br>GFUI_MOUSE_UP Action performed when the mouse right button is released
				<br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
int
GfuiButtonStateCreate(void *scr, char *text, int font, int x, int y, int width, int align, int mouse,
		      void *userDataOnPush, tfuiCallback onPush, 
		      void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    int		id;
    tGfuiButton	*button;
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    id = GfuiButtonCreate(scr, text, font, x, y, width, align, mouse, userDataOnPush, onPush, userDataOnFocus,
			  onFocus, onFocusLost);

    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_BUTTON) {
		    button = &(curObject->u.button);
		    button->buttonType = GFUI_BTN_STATE;
		}
		return id;
	    }
	} while (curObject != screen->objects);
    }
    return id;
}


/** Add a button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	text		Button label
    @param	font		Font id
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	width		width of the button (0 = text size)
    @param	align		Button alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VC	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VT	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VC	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VT	horizontal left, vertical top
    @param	mouse		Mouse behavior:
    				<br>GFUI_MOUSE_UP Action performed when the mouse right button is released
				<br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
int
GfuiButtonCreate(void *scr, char *text, int font, int x, int y, int width, int align, int mouse,
		 void *userDataOnPush, tfuiCallback onPush, 
		 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiButton	*button;
    tGfuiLabel	*label;
    tGfuiObject	*object;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;


    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_BUTTON;
    object->focusMode = GFUI_FOCUS_MOUSE_MOVE;
    object->id = screen->curId++;
    object->visible = 1;
    
    button = &(object->u.button);
    button->state = GFUI_BTN_RELEASED;
    button->userDataOnPush = userDataOnPush;
    button->onPush = onPush;
    button->userDataOnFocus = userDataOnFocus;
    button->onFocus = onFocus;
    button->onFocusLost = onFocusLost;
    button->mouseBehaviour = mouse;
    button->buttonType = GFUI_BTN_PUSH;

    button->bgColor[0] = &(GfuiColor[GFUI_BGBTNDISABLED][0]);
    button->bgColor[1] = &(GfuiColor[GFUI_BGBTNENABLED][0]);
    button->bgColor[2] = &(GfuiColor[GFUI_BGBTNCLICK][0]);
    button->bgFocusColor[0] = &(GfuiColor[GFUI_BGBTNDISABLED][0]);
    button->bgFocusColor[1] = &(GfuiColor[GFUI_BGBTNFOCUS][0]);
    button->bgFocusColor[2] = &(GfuiColor[GFUI_BGBTNCLICK][0]);
    button->fgColor[0] = &(GfuiColor[GFUI_BTNDISABLED][0]);
    button->fgColor[1] = &(GfuiColor[GFUI_BTNENABLED][0]);
    button->fgColor[2] = &(GfuiColor[GFUI_BTNCLICK][0]);
    button->fgFocusColor[0] = &(GfuiColor[GFUI_BTNDISABLED][0]);
    button->fgFocusColor[1] = &(GfuiColor[GFUI_BTNFOCUS][0]);
    button->fgFocusColor[2] = &(GfuiColor[GFUI_BTNCLICK][0]);

    label = &(button->label);
    label->text = (char*)calloc(1, 100);
    strncpy(label->text, text, 100);
    label->font = gfuiFont[font];
    label->maxlen = 99;
    if (width == 0) {
	width = gfuiFont[font]->getWidth((const char *)text);
    }
    label->align = align;
    switch(align&0xF0) {
    case 0x00 /* LEFT */:
	label->x = object->xmin = x;
	label->y = y - 2 * gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x + width;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    case 0x10 /* CENTER */:
	object->xmin = x - width / 2;
	label->x = x - gfuiFont[font]->getWidth((const char *)text) / 2;
	label->y = y - 2 * gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x + width / 2;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    case 0x20 /* RIGHT */:
	label->x = object->xmin = x - width;
	label->y = y - 2 * gfuiFont[font]->getDescender();
	object->ymin = y;
	object->xmax = x;
	object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	break;
    }
#define HORIZ_MARGIN 10
    object->xmin -= HORIZ_MARGIN;
    object->xmax += HORIZ_MARGIN;

    gfuiAddObject(screen, object);
    return object->id;
}

/** Change the label of a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	text	New label of the button
 */
void
GfuiButtonSetText(void *scr, int id, char *text)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    int oldmin, oldmax;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_BUTTON) {
		    oldmax = curObject->xmax;
		    oldmin = curObject->xmin;
		    gfuiSetLabelText(curObject, &(curObject->u.button.label), text);
		    curObject->xmax = oldmax;
		    curObject->xmin = oldmin;
		}
		return;
	    }
	} while (curObject != screen->objects);
    }    
}

/** Get the Id of the button focused in the current screen.
    @ingroup	gui
    @return	Button Id
		<br>-1 if no button or no screen or the focus is not on a button
 */
int
GfuiButtonGetFocused(void)
{
    tGfuiObject *curObject;

    if (GfuiScreen != NULL) {
	curObject = GfuiScreen->objects;
	if (curObject != NULL) {
	    do {
		curObject = curObject->next;
		if (curObject->focus) {
		    if (curObject->widget == GFUI_BUTTON) {
			return curObject->id;
		    }
		    return -1;
		}
	    } while (curObject != GfuiScreen->objects);
	}
    }
    return -1;
}

void
gfuiDrawButton(tGfuiObject *obj)
{
    tGfuiLabel	*label;
    tGfuiButton	*button;
    float	*fgColor;
    float	*bgColor;

    button = &(obj->u.button);
    if (obj->state == GFUI_DISABLE) {
	button->state = GFUI_BTN_DISABLE;
    }
    if (obj->focus) {
	fgColor = button->fgFocusColor[button->state];
	bgColor = button->bgFocusColor[button->state];
    } else {
	fgColor = button->fgColor[button->state];
	bgColor = button->bgColor[button->state];
    }
    if (bgColor[3] != 0.0) {
	glColor4fv(bgColor);
	glBegin(GL_QUADS);
	glVertex2i(obj->xmin, obj->ymin);
	glVertex2i(obj->xmin, obj->ymax);
	glVertex2i(obj->xmax, obj->ymax);
	glVertex2i(obj->xmax, obj->ymin);
	glEnd();
	glColor4fv(fgColor);
	glBegin(GL_LINE_STRIP);
	glVertex2i(obj->xmin, obj->ymin);
	glVertex2i(obj->xmin, obj->ymax);
	glVertex2i(obj->xmax, obj->ymax);
	glVertex2i(obj->xmax, obj->ymin);
	glVertex2i(obj->xmin, obj->ymin);
	glEnd();	
    }
    label = &(button->label);
    glColor4fv(fgColor);
    gfuiPrintString(label->x, label->y, label->font, label->text);
}

void
gfuiDrawGrButton(tGfuiObject *obj)
{
    int sw, sh, vw, vh;
    tGfuiGrButton	*button;
    unsigned char	*img;

    button = &(obj->u.grbutton);
    if (obj->state == GFUI_DISABLE) {
	img = button->disabled;
    } else if (button->state == GFUI_BTN_PUSHED) {
	img = button->pushed;
    } else if (obj->focus) {
	img = button->focused;
    } else {
	img = button->enabled;
    }
    GfScrGetSize(&sw, &sh, &vw, &vh);
    glRasterPos2i(obj->xmin, obj->ymin);
    glPixelZoom((float)vw / (float)GfuiScreen->width, (float)vh / (float)GfuiScreen->height);
    glDrawPixels(button->width, button->height, GL_RGBA, GL_UNSIGNED_BYTE, img);
}

void
gfuiGrButtonAction(int action)
{
    tGfuiGrButton	*button;

    button = &(GfuiScreen->hasFocus->u.grbutton);

    switch (button->buttonType) {
    case GFUI_BTN_PUSH:
	if (action == 2) { /* enter key */
	    if (button->onPush != NULL) {
		button->onPush(button->userDataOnPush);
	    }
	} else if (action == 1) { /* mouse up */
	    if (button->state != GFUI_BTN_RELEASED) {
		button->state = GFUI_BTN_RELEASED;
		if (button->mouseBehaviour == GFUI_MOUSE_UP) {
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		}
	    }
	} else { /* mouse down */
	    if (button->state != GFUI_BTN_PUSHED) {
		button->state = GFUI_BTN_PUSHED;
		if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		}
	    }
	}
	break;
	
    case GFUI_BTN_STATE:
	if (action == 2) { /* enter key */
	    if (button->state == GFUI_BTN_RELEASED) {
		button->state = GFUI_BTN_PUSHED;
		if (button->onPush != NULL) {
		    button->onPush(button->userDataOnPush);
		}
	    } else {
		button->state = GFUI_BTN_RELEASED;
	    }
	} else if (action == 1) { /* mouse up */
	    if (button->mouseBehaviour == GFUI_MOUSE_UP) {
		if (button->state == GFUI_BTN_RELEASED) {
		    button->state = GFUI_BTN_PUSHED;
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		} else {
		    button->state = GFUI_BTN_RELEASED;
		}
	    }
	} else { /* mouse down */
	    if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
		if (button->state == GFUI_BTN_RELEASED) {
		    button->state = GFUI_BTN_PUSHED;
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		} else {
		    button->state = GFUI_BTN_RELEASED;
		}
	    }
	}
	break;
    }
}

void
gfuiButtonAction(int action)
{
    tGfuiButton	*button;

    button = &(GfuiScreen->hasFocus->u.button);

    switch (button->buttonType) {
    case GFUI_BTN_PUSH:
	if (action == 2) { /* enter key */
	    if (button->onPush != NULL) {
		button->onPush(button->userDataOnPush);
	    }
	} else if (action == 1) { /* mouse up */
	    button->state = GFUI_BTN_RELEASED;
	    if (button->mouseBehaviour == GFUI_MOUSE_UP) {
		if (button->onPush != NULL) {
		    button->onPush(button->userDataOnPush);
		}
	    }
	} else { /* mouse down */
	    button->state = GFUI_BTN_PUSHED;
	    if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
		if (button->onPush != NULL) {
		    button->onPush(button->userDataOnPush);
		}
	    }
	}
	break;
	
    case GFUI_BTN_STATE:
	if (action == 2) { /* enter key */
	    if (button->state == GFUI_BTN_RELEASED) {
		button->state = GFUI_BTN_PUSHED;
		if (button->onPush != NULL) {
		    button->onPush(button->userDataOnPush);
		}
	    } else {
		button->state = GFUI_BTN_RELEASED;
	    }
	} else if (action == 1) { /* mouse up */
	    if (button->mouseBehaviour == GFUI_MOUSE_UP) {
		if (button->state == GFUI_BTN_RELEASED) {
		    button->state = GFUI_BTN_PUSHED;
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		} else {
		    button->state = GFUI_BTN_RELEASED;
		}
	    }
	} else { /* mouse down */
	    if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
		if (button->state == GFUI_BTN_RELEASED) {
		    button->state = GFUI_BTN_PUSHED;
		    if (button->onPush != NULL) {
			button->onPush(button->userDataOnPush);
		    }
		} else {
		    button->state = GFUI_BTN_RELEASED;
		}
	    }
	}
	break;
    }
}

void
gfuiReleaseButton(tGfuiObject *obj)
{
    tGfuiButton	*button;
    tGfuiLabel	*label;

    button = &(obj->u.button);
    label = &(button->label);

	freez(button->userDataOnFocus);
    free(label->text);
    free(obj);
}

void
gfuiReleaseGrButton(tGfuiObject *obj)
{
    tGfuiGrButton	*button;
    
    button = &(obj->u.grbutton);
    
    free(button->disabled);
    free(button->enabled);
    free(button->focused);
    free(button->pushed);
    free(obj);
}
