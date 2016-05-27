/***************************************************************************
                    tgfclient.h -- Interface file for The Gaming Framework                                    
                             -------------------                                         
    created              : Fri Aug 13 22:32:14 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: tgfclient.h,v 1.3.2.1 2008/08/16 14:12:08 berniw Exp $                                  
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
    	The Gaming Framework API (client part).
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: tgfclient.h,v 1.3.2.1 2008/08/16 14:12:08 berniw Exp $
*/



#ifndef __TGFCLIENT__H__
#define __TGFCLIENT__H__

#include <tgf.h>
#include <GL/glut.h>
#include <js.h>
#include <screen_properties.h>

extern void GfInitClient(void);

/******************** 
 * Screen Interface *
 ********************/

extern unsigned char *GfImgReadPng(const char *filename, int *widthp, int *heightp, float gamma);
extern int GfImgWritePng(unsigned char *img, const char *filename, int width, int height);
extern void GfImgFreeTex(GLuint tex);
extern GLuint GfImgReadTex(char *filename);

extern void GfScrInit(int argc, char *argv[]);
extern void GfScrShutdown(void);
extern void *GfScrMenuInit(void *precMenu);
extern char *GfTime2Str(tdble sec, int sgn);
extern void GfScrGetSize(int *ScrW, int *ScrH, int *ViewW, int *ViewH);
extern void GfScrReinit(void*);


/*****************************
 * GUI interface (low-level) *
 *****************************/

/* Widget type */
#define GFUI_LABEL	0
#define GFUI_BUTTON	1
#define GFUI_GRBUTTON	2
#define GFUI_SCROLLIST	3
#define GFUI_SCROLLBAR	4
#define GFUI_EDITBOX	5

/* Alignment */
#define GFUI_ALIGN_HL_VB	0x00
#define GFUI_ALIGN_HL_VC	0x01
#define GFUI_ALIGN_HL_VT	0x02
#define GFUI_ALIGN_HC_VB	0x10
#define GFUI_ALIGN_HC_VC	0x11
#define GFUI_ALIGN_HC_VT	0x12
#define GFUI_ALIGN_HR_VB	0x20
#define GFUI_ALIGN_HR_VC	0x21
#define GFUI_ALIGN_HR_VT	0x22

/* Mouse action */
#define GFUI_MOUSE_UP	0
#define GFUI_MOUSE_DOWN	1

/* Keyboard action */
#define GFUI_KEY_UP	0
#define GFUI_KEY_DOWN	1

/* Scroll Bar position */
#define GFUI_SB_NONE	0
#define GFUI_SB_RIGHT	1
#define GFUI_SB_LEFT	2
#define GFUI_SB_TOP	3
#define GFUI_SB_BOTTOM	4

/* Scroll bar orientation */
#define GFUI_HORI_SCROLLBAR	0
#define GFUI_VERT_SCROLLBAR	1

/** Scroll bar call-back information */
typedef struct ScrollBarInfo
{
    int		pos;		/**< Current scroll bar position */
    void	*userData;	/**< Associated user data */
} tScrollBarInfo;

typedef void (*tfuiCallback)(void * /* userdata */);
typedef void (*tfuiSBCallback)(tScrollBarInfo *);
typedef int (*tfuiKeyCallback)(unsigned char key, int modifier, int state); /**< return 1 to prevent normal key computing */
typedef int (*tfuiSKeyCallback)(int key, int modifier, int state);  /**< return 1 to prevent normal key computing */


/* GLUT Callback functions                  */
/* should be called explicitely if          */
/* the corresponding GLUT Func is overriden */
/* after a call to GfuiActivateScreen       */
extern void GfuiDisplay(void);
extern void GfuiDisplayNothing(void);
extern void GfuiIdle(void);

/* Screen management */
extern void *GfuiScreenCreate(void);
extern void *GfuiScreenCreateEx(float *bgColor, 
				void *userDataOnActivate, tfuiCallback onActivate, 
				void *userDataOnDeactivate, tfuiCallback onDeactivate, 
				int mouseAllowed);
extern void GfuiScreenRelease(void *screen);
extern void GfuiScreenActivate(void *screen);
extern int  GfuiScreenIsActive(void *screen);
extern void GfuiScreenReplace(void *screen);
extern void GfuiScreenDeactivate(void);
extern void *GfuiHookCreate(void *userDataOnActivate, tfuiCallback onActivate);
extern void GfuiHookRelease(void *hook);
extern void GfuiAddKey(void *scr, unsigned char key, char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiRegisterKey(unsigned char key, char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiAddSKey(void *scr, int key, char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiHelpScreen(void *prevScreen);
extern void GfuiScreenShot(void *notused);
extern void GfuiScreenAddBgImg(void *scr, char *filename);
extern void GfuiKeyEventRegister(void *scr, tfuiKeyCallback onKeyAction);
extern void GfuiSKeyEventRegister(void *scr, tfuiSKeyCallback onSKeyAction);
extern void GfuiKeyEventRegisterCurrent(tfuiKeyCallback onKeyAction);
extern void GfuiSKeyEventRegisterCurrent(tfuiSKeyCallback onSKeyAction);

/* mouse */
typedef struct MouseInfo
{
    int	X;
    int	Y;
    int	button[3];
} tMouseInfo;

extern tMouseInfo *GfuiMouseInfo(void);
extern void GfuiMouseSetPos(int x, int y);
extern void GfuiMouseHide(void);
extern void GfuiMouseShow(void);
extern void GfuiMouseSetHWPresent(void);

/* all widgets */
#define	GFUI_VISIBLE	1	/**< Object visibility flag  */
#define	GFUI_INVISIBLE	0	/**< Object invisibility flag  */
extern int GfuiVisibilitySet(void *scr, int id, int visible);
#define	GFUI_DISABLE	1
#define	GFUI_ENABLE	0
extern int GfuiEnable(void *scr, int id, int flag);
extern void GfuiUnSelectCurrent(void);

/* labels */
#define GFUI_FONT_BIG		0
#define GFUI_FONT_LARGE		1
#define GFUI_FONT_MEDIUM	2
#define GFUI_FONT_SMALL		3
#define GFUI_FONT_BIG_C		4
#define GFUI_FONT_LARGE_C	5
#define GFUI_FONT_MEDIUM_C	6
#define GFUI_FONT_SMALL_C	7
#define GFUI_FONT_DIGIT		8
extern int GfuiLabelCreate(void *scr, char *text, 
			int font, int x, int y, int align, int maxlen);
extern int GfuiLabelCreateEx(void *scr, char *text, float *fgColor, int font, int x, int y, int align, int maxlen);

extern int GfuiTipCreate(void *scr, char *text, int maxlen);
extern int GfuiTitleCreate(void *scr, char *text, int maxlen);

extern void GfuiLabelSetText(void *scr, int id, char *text);
extern void GfuiLabelSetColor(void *scr, int id, float *color);

extern void GfuiPrintString(const char *text, float *fgColor, int font, int x, int y, int align);
extern int  GfuiFontHeight(int font);
extern int  GfuiFontWidth(int font, const char *text);


/* buttons */
#define GFUI_BTNSZ	300
extern int GfuiButtonCreate(void *scr, char *text, int font,
			    int x, int y, int width, int align, int mouse,
			    void *userDataOnPush, tfuiCallback onPush, 
			    void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiButtonStateCreate(void *scr, char *text, int font, int x, int y, int width, int align, int mouse,
				 void *userDataOnPush, tfuiCallback onPush, 
				 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiGrButtonCreate(void *scr, char *disabled, char *enabled, char *focused, char *pushed,
			      int x, int y, int align, int mouse,
			      void *userDataOnPush, tfuiCallback onPush, 
			      void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);

extern void GfuiButtonSetText(void *scr, int id, char *text);
extern int GfuiButtonGetFocused(void);

/* Edit Box */
extern int GfuiEditboxCreate(void *scr, char *text, int font, int x, int y, int width, int maxlen,
			     void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiEditboxGetFocused(void);
extern char *GfuiEditboxGetString(void *scr, int id);
extern void GfuiEditboxSetString(void *scr, int id, char *text);

/* Scrolling lists */
extern int GfuiScrollListCreate(void *scr, int font, int x, int y, int align,
				int width, int height, int scrollbar, void *userDataOnSelect, tfuiCallback onSelect);
extern int GfuiScrollListInsertElement(void *scr, int Id, char *element, int index, void *userData);
extern int GfuiScrollListMoveSelectedElement(void *scr, int Id, int delta);
extern char *GfuiScrollListExtractSelectedElement(void *scr, int Id, void **userData);
extern char *GfuiScrollListExtractElement(void *scr, int Id, int index, void **userData);
extern char *GfuiScrollListGetSelectedElement(void *scr, int Id, void **userData);
extern char *GfuiScrollListGetElement(void *scr, int Id, int index, void **userData);

/* scroll bars */
extern int GfuiScrollBarCreate(void *scr, int x, int y, int align, int width, int orientation,
			       int min, int max, int len, int start, 
			       void *userData, tfuiSBCallback onScroll);
extern void GfuiScrollBarPosSet(void *scr, int id, int min, int max, int len, int start);
extern int GfuiScrollBarPosGet(void *scr, int id);

/* Images */
extern int GfuiStaticImageCreate(void *scr, int x, int y, int w, int h, char *name);
extern void GfuiStaticImageSet(void *scr, int id, char *name);

/*****************************
 * Menu Management Interface *
 *****************************/

extern void *GfuiMenuScreenCreate(char *title);
extern void  GfuiMenuDefaultKeysAdd(void *scr);
extern int   GfuiMenuButtonCreate(void *menu, char *text, char *tip, void *userdata, tfuiCallback onpush);
extern int   GfuiMenuBackQuitButtonCreate(void *menu, char *text, char *tip, void *userdata, tfuiCallback onpush);


/*********************
 * Control interface *
 *********************/

#define GFCTRL_TYPE_NOT_AFFECTED	0
#define GFCTRL_TYPE_JOY_AXIS		1
#define GFCTRL_TYPE_JOY_BUT		2
#define GFCTRL_TYPE_KEYBOARD		3
#define GFCTRL_TYPE_MOUSE_BUT		4
#define GFCTRL_TYPE_MOUSE_AXIS		5
#define GFCTRL_TYPE_SKEYBOARD		6

typedef struct
{
    int		index;
    int		type;
} tCtrlRef;


#define GFCTRL_JOY_UNTESTED	-1
#define GFCTRL_JOY_NONE		0
#define GFCTRL_JOY_PRESENT	1

#define GFCTRL_JOY_MAXBUTTON	32 /* Size of integer so don't change please */

/** Joystick Information Structure */
typedef struct
{
    int		oldb[NUM_JOY];
    float	ax[MAX_AXES * NUM_JOY];			/**< Axis values */
    int		edgeup[GFCTRL_JOY_MAXBUTTON * NUM_JOY];	/**< Button transition from down (pressed) to up */
    int		edgedn[GFCTRL_JOY_MAXBUTTON * NUM_JOY];	/**< Button transition from up to down */
    int		levelup[GFCTRL_JOY_MAXBUTTON * NUM_JOY];/**< Button state (1 = up) */
} tCtrlJoyInfo;

extern tCtrlJoyInfo *GfctrlJoyInit(void);
extern int GfctrlJoyIsPresent(void);
extern int GfctrlJoyGetCurrent(tCtrlJoyInfo *joyInfo);
extern void GfctrlJoyRelease(tCtrlJoyInfo *joyInfo);


/** Mouse information structure */
typedef struct
{
    int		edgeup[3];	/**< Button transition from down (pressed) to up */
    int		edgedn[3];	/**< Button transition from up to down */
    int		button[3];	/**< Button state (1 = up) */
    float	ax[4];		/**< mouse axis position (mouse considered as a joystick) */
} tCtrlMouseInfo;

extern tCtrlMouseInfo *GfctrlMouseInit(void);
extern int GfctrlMouseGetCurrent(tCtrlMouseInfo *mouseInfo);
extern void GfctrlMouseRelease(tCtrlMouseInfo *mouseInfo);
extern void GfctrlMouseCenter(void);
extern void GfctrlMouseInitCenter(void);
extern tCtrlRef *GfctrlGetRefByName(char *name);
extern char *GfctrlGetNameByRef(int type, int index);

extern int GfuiGlutExtensionSupported(char *str);


#endif /* __TGFCLIENT__H__ */


