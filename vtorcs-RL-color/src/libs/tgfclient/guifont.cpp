/***************************************************************************
                         guifont.cpp -- GLTT fonts management
                             -------------------
    created              : Fri Aug 13 22:19:09 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: guifont.cpp,v 1.4.2.1 2008/11/09 17:50:22 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* This font manipulation is based on Brad Fish's glFont format and code.  */
/* http://www.netxs.net/bfish/news.html                                    */

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#elif defined(__FreeBSD__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#include <tgfclient.h>

#include "guifont.h"

static char buf[1024];

#define FONT_NB	9
GfuiFontClass *gfuiFont[FONT_NB];
char *keySize[4] = { "size big", "size large", "size medium", "size small" };


#ifndef WIN32
#if BYTE_ORDER == BIG_ENDIAN
void swap32(unsigned int *p, unsigned int size)
{
	unsigned int i, t;
	for (i = 0; i < size; i += 4) {
		t = (unsigned int) *p;
		*p = (t & 0xff000000U) >> 24;
		*p |= (t & 0x00ff0000U) >> 8;
		*p |= (t & 0x0000ff00U) << 8;
		*p |= (t & 0x000000ffU) << 24;
		p++;
	}
}
#endif
#endif


void gfuiLoadFonts(void)
{
	void *param;
	char *fontName;
	int	size;
	int	i;

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	param = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	fontName = GfParmGetStr(param, "Menu Font", "name", "b5.glf");
	sprintf(buf, "data/fonts/%s", fontName);

	for(i = 0; i < 4; i++) {
		size = (int)GfParmGetNum(param, "Menu Font", keySize[i], (char*)NULL, 10.0);
		gfuiFont[i] = new GfuiFontClass(buf);
		gfuiFont[i]->create(size);
	}

	fontName = GfParmGetStr(param, "Console Font", "name", "b7.glf");
	sprintf(buf, "data/fonts/%s", fontName);

	for(i = 0; i < 4; i++) {
		size = (int)GfParmGetNum(param, "Console Font", keySize[i], (char*)NULL, 10.0);
		gfuiFont[i+4] = new GfuiFontClass(buf);
		gfuiFont[i+4]->create(size);
	}

	fontName = GfParmGetStr(param, "Digital Font", "name", "digital.glf");
	sprintf(buf, "data/fonts/%s", fontName);
	size = (int)GfParmGetNum(param, "Digital Font", keySize[0], (char*)NULL, 8.0);
	gfuiFont[8] = new GfuiFontClass(buf);
	gfuiFont[8]->create(size);

	//GfParmReleaseHandle(param);
}


GfuiFontClass::GfuiFontClass(char *FileName)
{
	FILE *Input;
	char *TexBytes;
	int	Num;
	uint Tex;

	font = NULL;
	size = 8.0;

	//Open font file
	if ((Input = fopen(FileName, "rb")) == NULL) {
		perror(FileName);
		return;
	}

	if ((font = (GLFONT *)malloc(sizeof(GLFONT))) == NULL) {
		return;
	}

	//Read glFont structure
	//fread(font, sizeof(GLFONT), 1, Input);
	fread(font, 24, 1, Input); // for IA64...

#ifndef WIN32
#if BYTE_ORDER == BIG_ENDIAN
	swap32((unsigned int *) font, 24);
#endif
#endif

	//Get number of characters
	Num = font->IntEnd - font->IntStart + 1;

	//Allocate memory for characters
	if ((font->Char = (GLFONTCHAR *)malloc(sizeof(GLFONTCHAR) * Num)) == NULL) {
		free(font);
		font = NULL;
		fclose(Input);
		return;
	}

    //Read glFont characters
	fread(font->Char, sizeof(GLFONTCHAR), Num, Input);

#ifndef WIN32
#if BYTE_ORDER == BIG_ENDIAN
	swap32((unsigned int *) font->Char, sizeof(GLFONTCHAR) * Num);
#endif
#endif

	//Get texture size
	Num = font->TexWidth * font->TexHeight * 2;

	//Allocate memory for texture data
	if ((TexBytes = (char *)malloc(Num)) == NULL) {
		fclose(Input);
		return;
	}

	//Read texture data
	fread(TexBytes, sizeof(char), Num, Input);

	fclose(Input);

	//Save texture number
	glGenTextures(1, &Tex);
	font->Tex = Tex;
	//Set texture attributes
	glBindTexture(GL_TEXTURE_2D, Tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//Create texture
	glTexImage2D(GL_TEXTURE_2D, 0, 2, font->TexWidth,
		 font->TexHeight, 0, GL_LUMINANCE_ALPHA,
		 GL_UNSIGNED_BYTE, (void *)TexBytes);

	//Clean up
	free(TexBytes);

	//Return pointer to new font
	return;
}


GfuiFontClass::~GfuiFontClass()
{
	if (font) {
		glDeleteTextures(1, &font->Tex);
		free(font->Char);
		free(font);
	}
}


void GfuiFontClass::create(int point_size)
{
	size = point_size;
}


int GfuiFontClass::getWidth(const char* text)
{
	int Length, i;
	GLFONTCHAR *Char;
	float width = 0;

	if (font == NULL) {
		return 0;
	}

	//Get length of string
	Length = strlen(text);

	//Loop through characters
	for (i = 0; i < Length; i++) {
		//Get pointer to glFont character
		Char = &font->Char[(int)text[i] - font->IntStart];
		float w2 = Char->dx * size;
		width = width + w2;
		//width += Char->dx * size;
	}

	return (int)width;
}


int GfuiFontClass::getHeight() const
{
	if (font == NULL) return 0;
	return (const int)(font->Char[0].dy * size);
}


int GfuiFontClass::getDescender() const
{
	if (font == NULL) return 0;
	return 0;
	return (const int)(font->Char[0].dy * size / 2.0);
}


void GfuiFontClass::output(int X, int Y, const char* text)
{
	int		Length, i;
	GLFONTCHAR	*Char;
	float	x = (float)X;
	float	y = (float)Y;

	//Return if we don't have a valid glFont
	if (font == NULL) return;

	//Get length of string
	Length = strlen(text);

	//Begin rendering quads
	glBindTexture(GL_TEXTURE_2D, font->Tex);
	glBegin(GL_QUADS);

	//Loop through characters
	for (i = 0; i < Length; i++)
	{
		//Get pointer to glFont character
		Char = &font->Char[(int)text[i] - font->IntStart];

		//Specify vertices and texture coordinates
		glTexCoord2f(Char->tx1, Char->ty1);
		glVertex2f(x, y + Char->dy * size);
		glTexCoord2f(Char->tx1, Char->ty2);
		glVertex2f(x, y);
		glTexCoord2f(Char->tx2, Char->ty2);
		glVertex2f(x + Char->dx * size, y);
		glTexCoord2f(Char->tx2, Char->ty1);
		glVertex2f(x + Char->dx * size, y + Char->dy * size);

		//Move to next character
		x += Char->dx*size;
	}

	//Stop rendering quads
	glEnd();
}
