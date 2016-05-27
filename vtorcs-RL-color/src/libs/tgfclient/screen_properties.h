/***************************************************************************

    file        : screen_properties.h
    created     : Sat Apr 19 23:37:41 CEST 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: screen_properties.h,v 1.4 2005/02/01 15:55:55 berniw Exp $                                  

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: screen_properties.h,v 1.4 2005/02/01 15:55:55 berniw Exp $
*/

#ifndef _SCREEN_PROPERTIES_H_
#define _SCREEN_PROPERTIES_H_

#define GFSCR_CONF_FILE		"config/screen.xml"

#define GFSCR_SECT_PROP		"Screen Properties"

#define GFSCR_ATT_X		"x"
#define GFSCR_ATT_Y		"y"
#define GFSCR_ATT_BPP		"bpp"
#define GFSCR_ATT_WIN_X		"window width"
#define GFSCR_ATT_WIN_Y		"window height"
#define GFSCR_ATT_MAXREFRESH	"maximum refresh frequency"
#define GFSCR_ATT_FSCR		"fullscreen"
#define GFSCR_VAL_YES		"yes"
#define GFSCR_VAL_NO		"no"
#define GFSCR_ATT_GAMMA		"gamma"

#define GFSCR_ATT_VINIT				"video mode init"
#define GFSCR_VAL_VINIT_COMPATIBLE	"compatible"
#define GFSCR_VAL_VINIT_BEST		"best"


#define GFSCR_SECT_MENUCOL	"Menu Colors"

#define GFSCR_LIST_COLORS	"colors"

#define GFSCR_ELT_BGCOLOR	"background"
#define GFSCR_ELT_TITLECOLOR	"title"
#define GFSCR_ELT_BGBTNFOCUS	"background focused button"
#define GFSCR_ELT_BGBTNCLICK	"background pushed button"
#define GFSCR_ELT_BGBTNENABLED	"background enabled button"
#define GFSCR_ELT_BGBTNDISABLED	"background disabled button"
#define GFSCR_ELT_BTNFOCUS	"focused button"
#define GFSCR_ELT_BTNCLICK	"pushed button"
#define GFSCR_ELT_BTNENABLED	"enabled button"
#define GFSCR_ELT_BTNDISABLED	"disabled button"
#define GFSCR_ELT_LABELCOLOR	"label"
#define GFSCR_ELT_TIPCOLOR	"tip"
#define GFSCR_ELT_MOUSECOLOR1	"mouse 1"
#define GFSCR_ELT_MOUSECOLOR2	"mouse 2"
#define GFSCR_ELT_HELPCOLOR1	"help key"
#define GFSCR_ELT_HELPCOLOR2	"help description"
#define GFSCR_ELT_BGSCROLLIST	"background scroll list"
#define GFSCR_ELT_SCROLLIST	"scroll list"
#define GFSCR_ELT_BGSELSCROLLIST "background selected scroll list"
#define GFSCR_ELT_SELSCROLLIST	"selected scroll list"
#define GFSCR_ELT_EDITCURSORCLR	"edit box cursor color"

#define GFSCR_ATTR_RED		"red"
#define GFSCR_ATTR_GREEN	"green"
#define GFSCR_ATTR_BLUE		"blue"
#define GFSCR_ATTR_ALPHA	"alpha"


#endif /* _SCREEN_PROPERTIES_H_ */ 



