/***************************************************************************
                         gui.h -- Interface file for GUI                          
                             -------------------                                         
    created              : Fri Aug 13 22:15:46 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: gui.h,v 1.2.2.1 2008/08/16 14:12:08 berniw Exp $                                  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _GUI_H__
#define _GUI_H__

#include "guifont.h"

#define GFUI_COLORNB	21
extern float	GfuiColor[GFUI_COLORNB][4];

#define GFUI_BGCOLOR		0
#define GFUI_TITLECOLOR		1
#define GFUI_BGBTNFOCUS		2
#define GFUI_BGBTNCLICK		3
#define GFUI_BGBTNENABLED	4
#define GFUI_BGBTNDISABLED	5
#define GFUI_BTNFOCUS		6
#define GFUI_BTNCLICK		7
#define GFUI_BTNENABLED		8
#define GFUI_BTNDISABLED	9
#define GFUI_LABELCOLOR		10
#define GFUI_TIPCOLOR		11
#define GFUI_MOUSECOLOR1	12
#define GFUI_MOUSECOLOR2	13
#define GFUI_HELPCOLOR1		14
#define GFUI_HELPCOLOR2		15
#define GFUI_BGSCROLLIST	16
#define GFUI_FGSCROLLIST	17
#define GFUI_BGSELSCROLLIST	18
#define GFUI_FGSELSCROLLIST	19
#define GFUI_EDITCURSORCLR	20
#define GFUI_IMAGE		21

typedef struct
{
    char	*text;		/* text */
    float	*bgColor;	/* RGBA */
    float	*fgColor;
    GfuiFontClass	*font;		/* ttf font */
    int		x, y;		/* label position */
    int		align;
    int		maxlen;
} tGfuiLabel;

/* button state */
#define GFUI_BTN_DISABLE	0
#define GFUI_BTN_RELEASED	1
#define GFUI_BTN_PUSHED		2

/* Button type */
#define GFUI_BTN_PUSH		0
#define GFUI_BTN_STATE		1

typedef struct
{
    tGfuiLabel		label;
    float		*bgColor[3];
    float		*fgColor[3];
    float		*bgFocusColor[3];
    float		*fgFocusColor[3];
    unsigned int	state;
    int			buttonType;
    int			mouseBehaviour;
    void		*userDataOnPush;
    tfuiCallback	onPush;
    void		*userDataOnFocus;
    tfuiCallback	onFocus;
    tfuiCallback	onFocusLost;
} tGfuiButton;

typedef struct
{
    unsigned int	state;
    unsigned char	*disabled;
    unsigned char	*enabled;
    unsigned char	*focused;
    unsigned char	*pushed;
    int			width, height;
    int			buttonType;
    int			mouseBehaviour;
    void		*userDataOnPush;
    tfuiCallback	onPush;
    void		*userDataOnFocus;
    tfuiCallback	onFocus;
    tfuiCallback	onFocusLost;
} tGfuiGrButton;

#define GFUI_FOCUS_NONE		0
#define GFUI_FOCUS_MOUSE_MOVE	1
#define GFUI_FOCUS_MOUSE_CLICK	2

typedef struct GfuiListElement
{
    char			*name;
    char			*label;
    void			*userData;
    int				selected;
    int				index;
    struct GfuiListElement	*next;
    struct GfuiListElement	*prev;
} tGfuiListElement;

typedef struct
{
    int			sbPos;
    float		*bgColor[3];
    float		*fgColor[3];
    float		*bgSelectColor[3];
    float		*fgSelectColor[3];
    GfuiFontClass	*font;
    tGfuiListElement	*elts;
    int			nbElts;
    int			firstVisible;
    int			nbVisible;
    int			selectedElt;
    int			scrollBar;
    tfuiCallback	onSelect;
    void		*userDataOnSelect;
} tGfuiScrollList;

typedef struct
{
    tScrollBarInfo	info;
    int			min, max, len, pos;
    int			orientation;
    void		*userData;
    tfuiSBCallback	onScroll;
} tGfuiScrollBar;

typedef struct
{
    tGfuiLabel		label;
    float		*cursorColor[3];
    float		*bgColor[3];
    float		*fgColor[3];
    float		*bgFocusColor[3];
    float		*fgFocusColor[3];
    int			state;
    int			cursorx;
    int			cursory1;
    int			cursory2;
    int			cursorIdx;
    void		*userDataOnFocus;
    tfuiCallback	onFocus;
    tfuiCallback	onFocusLost;    
} tGfuiEditbox;

typedef struct
{
    GLuint		texture;
} tGfuiImage;

typedef struct GfuiObject
{
    int		widget;
    int		id;
    int		visible;
    int		focusMode;
    int		focus;
    int		state;		/* enable / disable */
    int		xmin, ymin;	/* bounding box for focus */
    int		xmax, ymax;
    union
    {
	tGfuiLabel	label;
	tGfuiButton	button;
	tGfuiGrButton	grbutton;
	tGfuiScrollList scrollist;
	tGfuiScrollBar	scrollbar;
	tGfuiEditbox	editbox;
	tGfuiImage	image;
    } u;
    struct GfuiObject	*next;
    struct GfuiObject	*prev;
} tGfuiObject;

/* Keyboard key assignment */
typedef struct GfuiKey
{
    unsigned char	key;
    char		*name;
    char		*descr;
    int			specialkey;
    int			modifier;
    void		*userData;
    tfuiCallback	onPress;
    tfuiCallback	onRelease;
    struct GfuiKey	*next;
} tGfuiKey;

/* screen definition */
typedef struct 
{
    float		width, height;
    float		*bgColor;	/* RGBA */
    GLuint		bgImage;

    /* sub-objects */
    tGfuiObject		*objects;
    tGfuiObject		*hasFocus;	/* in order to speed up focus management */
    int			curId;

    /* users keys definition */
    tGfuiKey		*userKeys;
    tGfuiKey		*userSpecKeys;
    void		*userActData;
    tfuiCallback	onActivate;
    void		*userDeactData;
    tfuiCallback	onDeactivate;

    /* key callback functions */
    tfuiKeyCallback	onKeyAction;
    tfuiSKeyCallback	onSKeyAction;

    /* mouse handling */
    int			mouse;
    int			mouseAllowed;
    float		*mouseColor[2];

    /* menu specific */
    int			nbItems;

    /* Screen type */
    int			onlyCallback;
} tGfuiScreen;


extern tGfuiScreen	*GfuiScreen;
extern tMouseInfo	 GfuiMouse;
extern int		 GfuiMouseHW;

extern void gfuiReleaseObject(tGfuiObject *curObject);

extern void GfuiDrawCursor();
extern void GfuiDraw(tGfuiObject *obj);;
extern void gfuiUpdateFocus();
extern void gfuiPrintString(int x, int y, GfuiFontClass *font, const char *string);
extern void gfuiMouseAction(void *action);
extern void gfuiSelectNext(void *);
extern void gfuiSelectPrev(void *);
extern void gfuiSelectId(void *scr, int id);
extern void gfuiAddObject(tGfuiScreen *screen, tGfuiObject *object);
extern tGfuiObject *gfuiGetObject(void *scr, int id);

extern void gfuiSetLabelText(tGfuiObject *object, tGfuiLabel *label, char *text);

extern void gfuiDrawLabel(tGfuiObject *obj);
extern void gfuiDrawButton(tGfuiObject *obj);
extern void gfuiButtonAction(int action);
extern void gfuiDrawGrButton(tGfuiObject *obj);
extern void gfuiGrButtonAction(int action);
extern void gfuiDrawScrollist(tGfuiObject *obj);
extern void gfuiScrollListAction(int mouse);
extern void gfuiDrawEditbox(tGfuiObject *obj);
extern void gfuiEditboxAction(int action);

extern void gfuiInit(void);
extern void gfuiButtonInit(void);
extern void gfuiHelpInit(void);
extern void gfuiLabelInit(void);
extern void gfuiObjectInit(void);
extern void gfuiEditboxInit(void);

extern void gfuiReleaseLabel(tGfuiObject *obj);
extern void gfuiReleaseButton(tGfuiObject *obj);
extern void gfuiReleaseGrButton(tGfuiObject *obj);
extern void gfuiReleaseScrollist(tGfuiObject *curObject);
extern void gfuiReleaseScrollbar(tGfuiObject *curObject);
extern void gfuiReleaseEditbox(tGfuiObject *curObject);

extern void gfuiLoadFonts(void);

extern void gfuiEditboxKey(tGfuiObject *obj, int key, int modifier);


extern void gfuiScrollListNextElt (tGfuiObject *object);
extern void gfuiScrollListPrevElt (tGfuiObject *object);

extern void gfuiReleaseImage(tGfuiObject *obj);
extern void gfuiDrawImage(tGfuiObject *obj);


#endif /* _GUI_H__ */ 



