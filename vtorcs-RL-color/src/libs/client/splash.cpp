/***************************************************************************

    file                 : splash.cpp
    created              : Sat Mar 18 23:49:03 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: splash.cpp,v 1.4.2.2 2008/08/16 14:58:51 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifdef WIN32
#include <windows.h>
#define HAVE_CONFIG_H
#endif
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tgfclient.h>

#include "splash.h"
#include "mainmenu.h"

static int s_imgWidth, s_imgHeight;
static GLuint s_texture = 0;
static int SplashDisplaying;
static char buf[1024];

/*
 * Function
 *	splashKey
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashKey( unsigned char /* key */, int /* x */, int /* y */)
{
	SplashDisplaying = 0;
	glDeleteTextures(1, &s_texture);
	s_texture = 0;
	TorcsMainMenuRun();
}

/*
 * Function
 *	
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashTimer(int /* value */)
{
	if (SplashDisplaying) {
		SplashDisplaying = 0;
		glDeleteTextures(1, &s_texture);
		s_texture = 0;
		TorcsMainMenuRun();
	}
}
	

/*
 * Function
 *	splashDisplay
 *
 * Description
 *
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashDisplay( void )
{
	int	ScrW, ScrH, ViewW, ViewH;
	
	SplashDisplaying = 1;
		
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	
	GfScrGetSize(&ScrW, &ScrH, &ViewW, &ViewH);
	
	glViewport((ScrW-ViewW) / 2, (ScrH-ViewH) / 2, ViewW, ViewH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ScrW, 0, ScrH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	if (s_texture != 0) {
		GLfloat tx1 = 0.0f, tx2 = 1.0f, ty1 = 0.0f, ty2 = 1.0f;
		
		// Compute texture coordinates to ensure proper unskewed/unstretched display of
		// image content.
		//tdble rfactor = (float)(s_imgWidth*ViewH)/(float)(s_imgHeight*ViewW);
		tdble rfactor = (16.0f*ViewH)/(10.0f*ViewW);

		if (rfactor >= 1.0f) {
			// Aspect ratio of view is smaller than 16:10, "cut off" sides
			tdble tdx = (1.0f-1.0f/rfactor)/2.0f;
			tx1 += tdx;
			tx2 -= tdx;
		} else {
			// Aspect ratio of view is larger than 16:10, "cut off" top and bottom
			tdble tdy = (1.0f-rfactor)/2.0f;
			ty1 += tdy;
			ty2 -= tdy;
		}

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, s_texture);
		glBegin(GL_QUADS);
		glTexCoord2f(tx1, ty1); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(tx1, ty2); glVertex3f(0.0, ScrH, 0.0);
		glTexCoord2f(tx2, ty2); glVertex3f(ScrW, ScrH, 0.0);
		glTexCoord2f(tx2, ty1); glVertex3f(ScrW, 0.0, 0.0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
		
#ifdef HAVE_CONFIG_H
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 640, 0, 480);
	
	static float grWhite[4] = {1.0, 1.0, 1.0, 1.0};
	GfuiPrintString(VERSION, grWhite, GFUI_FONT_SMALL_C, 640-8, 8, GFUI_ALIGN_HR_VB);
#endif

	glutSwapBuffers();
}

static void splashMouse(int /* b */, int s, int /* x */, int /* y */)
{
	if (s == GLUT_UP) {
		SplashDisplaying = 0;
		glDeleteTextures(1, &s_texture);
		s_texture = 0;
		TorcsMainMenuRun();
	}
}


/*
 * Function
 *	
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
int SplashScreen(void)
{
	void	*handle;
	float	screen_gamma;
	const char	*filename = "data/img/splash.png";
	
	if (s_texture != 0) {
		glDeleteTextures(1, &s_texture); 
	}
	
	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);	
	GLbyte *tex = (GLbyte*)GfImgReadPng(filename, &s_imgWidth, &s_imgHeight, screen_gamma);
	if (!tex) {
		GfParmReleaseHandle(handle);
		GfTrace("Couldn't read %s\n", filename);
		return -1;
	}

	glGenTextures(1, &s_texture);
	glBindTexture(GL_TEXTURE_2D, s_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s_imgWidth, s_imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)(tex));
	free(tex);
	
	glutDisplayFunc(splashDisplay);
	glutKeyboardFunc(splashKey);
	glutSpecialFunc((void (*)(int key, int x, int y))NULL);
	glutTimerFunc(7000, splashTimer, 0);
	glutMouseFunc(splashMouse);
    
	return 0;
}


