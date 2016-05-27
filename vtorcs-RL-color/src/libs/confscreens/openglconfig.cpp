/***************************************************************************

    file        : openglconfig.cpp
    created     : Fri Jun 3 12:52:07 CET 2004
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch
    version     : $Id: openglconfig.cpp,v 1.2.2.2 2008/11/09 17:50:21 berniw Exp $

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

    @version	$Id: openglconfig.cpp,v 1.2.2.2 2008/11/09 17:50:21 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <tgfclient.h>
#include <raceinit.h>
#include <graphic.h>
#include <glfeatures.h>
#include "openglconfig.h"

static float LabelColor[] = {1.0, 0.0, 1.0, 1.0};

// Texture compression.
static char *textureCompressOptionList[] = {GR_ATT_TEXTURECOMPRESSION_DISABLED, GR_ATT_TEXTURECOMPRESSION_ENABLED};
static const int nbOptionsTextComp = sizeof(textureCompressOptionList) / sizeof(textureCompressOptionList[0]);
static int curOptionTextComp = 0;
static int TextureCompressOptionId;

// Texture sizing, order of list is important, do not change.
static int textureSizeOptionList[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
static int nbOptionsTextSize = sizeof(textureSizeOptionList) / sizeof(textureSizeOptionList[0]);
static int curOptionTextSize = 0;
static int TextureSizeOptionId;
static const int defaultTextSize = 64; // In case everything goes wrong.
static char valuebuf[10];

// gui screen handles.
static void	*scrHandle = NULL;
static void	*prevHandle = NULL;


// Read OpenGL configuration.
static void readOpenGLCfg(void)
{
	int	i;
	char buf[1024];

	sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	// Read texture compression parameters.
	char *optionName = GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION, textureCompressOptionList[0]);
	for (i = 0; i < nbOptionsTextComp; i++) {
		if (strcmp(optionName, textureCompressOptionList[i]) == 0) {
			curOptionTextComp = i;
			break;
		}
	}
	if (isCompressARBAvailable()) {
		GfuiLabelSetText(scrHandle, TextureCompressOptionId, textureCompressOptionList[curOptionTextComp]);
	}

	// Read texture sizing parameters.
	int maxsizenb = 0;
	int sizelimit = getGLTextureMaxSize();
	int tsize = (int) GfParmGetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURESIZE, (char*)NULL, (tdble) sizelimit);
	bool found = false;

	for (i = 0; i < nbOptionsTextSize; i++) {
		if (textureSizeOptionList[i] <= sizelimit) {
			maxsizenb = i;
		} else {
			break;
		}
	}

	// Limit to available sizes.
	nbOptionsTextSize = maxsizenb+1;

	for (i = 0; i < nbOptionsTextSize; i++) {
		if (textureSizeOptionList[i] == tsize) {
			curOptionTextSize = i;
			found = true;
			break;
		}
	}

	if (!found) {
		// Should never come here if there is no bug in OpenGL.
		tsize = defaultTextSize;
		for (i = 0; i < nbOptionsTextSize; i++) {
			if (textureSizeOptionList[i] == tsize) {
				curOptionTextSize = i;
				break;
			}
		}
	}
	sprintf(valuebuf, "%d", textureSizeOptionList[curOptionTextSize]);
	GfuiLabelSetText(scrHandle, TextureSizeOptionId, valuebuf);

	GfParmReleaseHandle(paramHandle);
}


// Save the choosen values in the corresponding parameter file.
static void saveOpenGLOption(void *)
{
	char buf[1024];
	sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	// Texture compression.
	GfParmSetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION, textureCompressOptionList[curOptionTextComp]);
	// Texture sizing.
	GfParmSetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURESIZE, (char*)NULL, (tdble) textureSizeOptionList[curOptionTextSize]);

	GfParmWriteFile(NULL, paramHandle, "graph");
	GfParmReleaseHandle(paramHandle);

	// Return to previous screen.
	GfuiScreenActivate(prevHandle);
	updateCompressARBEnabled();
	updateUserTextureMaxSize();
	return;
}


// Toggle texture compression state enabled/disabled.
static void changeTextureCompressState(void *vp)
{
	if (vp == 0) {
		curOptionTextComp--;
		if (curOptionTextComp < 0) {
	    	curOptionTextComp = nbOptionsTextComp - 1;
		}
	} else {
		curOptionTextComp++;
		if (curOptionTextComp == nbOptionsTextComp) {
	    	curOptionTextComp = 0;
		}
	}
	GfuiLabelSetText(scrHandle, TextureCompressOptionId, textureCompressOptionList[curOptionTextComp]);
}


// Scroll through texture sizes smaller or equal the system limit.
static void changeTextureSizeState(void *vp)
{
	long delta = (long)vp;
	curOptionTextSize += delta;
	if (curOptionTextSize < 0) {
		curOptionTextSize = nbOptionsTextSize - 1;
	} else if (curOptionTextSize >= nbOptionsTextSize) {
		curOptionTextSize= 0;
	}

	sprintf(valuebuf, "%d", textureSizeOptionList[curOptionTextSize]);
	GfuiLabelSetText(scrHandle, TextureSizeOptionId, valuebuf);
}


static void onActivate(void * /* dummy */)
{
	readOpenGLCfg();
}


// OpenGL menu
void * OpenGLMenuInit(void *prevMenu)
{
	int y, dy;

	// Has screen already been created?
	if (scrHandle) {
		return scrHandle;
	}

	prevHandle = prevMenu;

	scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
	GfuiTitleCreate(scrHandle, "OpenGL Options", 0);
	GfuiScreenAddBgImg(scrHandle, "data/img/splash-simucfg.png");

	y = 400;
	dy = 30;

	const int xleft = 160;	// Center of left elements.
	const int xright = 480;	// Center of right elements.
	const int width = 240;	// Width of elements.

	y -= dy;

	// Texture compression.
	GfuiLabelCreate(scrHandle, "Texture Compression", GFUI_FONT_LARGE, xleft, 400, GFUI_ALIGN_HC_VB, 0);

	if (isCompressARBAvailable()) {
		GfuiGrButtonCreate(scrHandle, "data/img/arrow-left.png", "data/img/arrow-left.png",
				"data/img/arrow-left.png", "data/img/arrow-left-pushed.png",
				xleft-width/2, y, GFUI_ALIGN_HC_VB, 1,
				(void*)-1, changeTextureCompressState,
				NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

		GfuiGrButtonCreate(scrHandle, "data/img/arrow-right.png", "data/img/arrow-right.png",
				"data/img/arrow-right.png", "data/img/arrow-right-pushed.png",
				xleft+width/2, y, GFUI_ALIGN_HC_VB, 1,
				(void*)1, changeTextureCompressState,
				NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

		TextureCompressOptionId = GfuiLabelCreate(scrHandle, "", GFUI_FONT_LARGE_C, xleft, y, GFUI_ALIGN_HC_VB, 32);
		GfuiLabelSetColor(scrHandle, TextureCompressOptionId, LabelColor);
	} else {
		GfuiLabelCreate(scrHandle, "not available", GFUI_FONT_LARGE_C, xleft, y, GFUI_ALIGN_HC_VB, 0);
	}


	// Texture sizing.
	GfuiLabelCreate(scrHandle, "Texture Size Limit", GFUI_FONT_LARGE, xright, 400, GFUI_ALIGN_HC_VB, 0);

	GfuiGrButtonCreate(scrHandle, "data/img/arrow-left.png", "data/img/arrow-left.png",
			"data/img/arrow-left.png", "data/img/arrow-left-pushed.png",
			xright-width/2, y, GFUI_ALIGN_HC_VB, 0,
			(void*)-1, changeTextureSizeState,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	GfuiGrButtonCreate(scrHandle, "data/img/arrow-right.png", "data/img/arrow-right.png",
			"data/img/arrow-right.png", "data/img/arrow-right-pushed.png",
			xright+width/2, y, GFUI_ALIGN_HC_VB, 0,
			(void*)1, changeTextureSizeState,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	TextureSizeOptionId = GfuiLabelCreate(scrHandle, "", GFUI_FONT_LARGE_C, xright, y, GFUI_ALIGN_HC_VB, 32);
	GfuiLabelSetColor(scrHandle, TextureSizeOptionId, LabelColor);


	GfuiButtonCreate(scrHandle, "Accept", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
	NULL, saveOpenGLOption, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	GfuiButtonCreate(scrHandle, "Cancel", GFUI_FONT_LARGE, 430, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
	prevMenu, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	GfuiAddKey(scrHandle, 13, "Save", NULL, saveOpenGLOption, NULL);
	GfuiAddKey(scrHandle, 27, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_LEFT, "Previous Option in list", (void*)0, changeTextureCompressState, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_RIGHT, "Next Option in list", (void*)1, changeTextureCompressState, NULL);

	readOpenGLCfg();

	return scrHandle;
}
