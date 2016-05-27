/***************************************************************************

    file                 : glfeatures.cpp
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: glfeatures.cpp,v 1.2 2005/08/05 09:26:39 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*
	Functions to check if features seems to be available and requested by the
	user. The isAvailable functions should return if a feature is working on
	the system, the isEnabled feature should check if the user wants to enable
	it as well.
	It should NOT check if the features are really working, that is subject
	to another part eventually.
*/

#include <glfeatures.h>

/*
	----------------------- Texture Compression
*/


static bool compressARBAvailable;
static bool compressARBEnabled;

// Feature checks, GL_ARB_texture_compression.
void checkCompressARBAvailable(bool &result)
{
	// Query if the extension is avaiable at the runtime system (true, if > 0).
	int compressARB = GfuiGlutExtensionSupported("GL_ARB_texture_compression");
	// Check if at least one internal format is vailable. This is a workaround for
	// driver problems and not a bugfix. According to the specification OpenGL should
	// choose an uncompressed alternate format if it can't provide the requested
	// compressed one... but it does not on all cards/drivers.
	if (compressARB) {
		int numformats;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &numformats);
		if (numformats == 0) {
			compressARB = 0;
		}
	}

	result = (compressARB > 0) ? true : false;
}


void checkCompressARBEnabled(bool &result)
{
	if (!isCompressARBAvailable()) {
		// Feature not available, do not use it.
		result = false;
	} else {
		// Feature available, check if the user wants to use it.
		// TODO: put this enabled/disable stuff in one function (it is used in grsound.cpp as well).
		char *tcEnabledStr = GR_ATT_TEXTURECOMPRESSION_ENABLED;
		char fnbuf[1024];
		sprintf(fnbuf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
		char *optionName = GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION, GR_ATT_TEXTURECOMPRESSION_DISABLED);

		if (strcmp(optionName, tcEnabledStr) != 0) {
			result = false;
		} else {
			result = true;
		}
		GfParmReleaseHandle(paramHandle);
	}
}


void updateCompressARBEnabled(void)
{
	checkCompressARBEnabled(compressARBEnabled);
}


// GL_ARB_texture_compression
bool isCompressARBAvailable(void)
{
	return compressARBAvailable;
}


bool isCompressARBEnabled(void) {
	return compressARBEnabled;
}


/*
	----------------------- Texture downsizing.
*/
static int glTextureMaxSize;
static int userTextureMaxSize;


void getGLTextureMaxSize(int &result)
{
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	if (result > 16384) {
		result = 16384;
	}
}


void getUserTextureMaxSize(int &result)
{
	char fnbuf[1024];
	sprintf(fnbuf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
	void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	result = (int) GfParmGetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURESIZE, (char*)NULL, (tdble) glTextureMaxSize);
	if (result > glTextureMaxSize) {
		result = glTextureMaxSize;
	}
	GfParmReleaseHandle(paramHandle);
}


void updateUserTextureMaxSize(void)
{
	getUserTextureMaxSize(userTextureMaxSize);
}

int getGLTextureMaxSize(void)
{
	return glTextureMaxSize;
}


int getUserTextureMaxSize(void)
{
	return userTextureMaxSize;
}


// Initialize
void checkGLFeatures(void) {
	checkCompressARBAvailable(compressARBAvailable);
	checkCompressARBEnabled(compressARBEnabled);
	getGLTextureMaxSize(glTextureMaxSize);
	getUserTextureMaxSize(userTextureMaxSize);
}



