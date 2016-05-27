/***************************************************************************
                           screen.cpp -- screen init
                             -------------------
    created              : Fri Aug 13 22:29:56 CEST 1999
    copyright            : (C) 1999, 2004 by Eric Espie, Bernhard Wymann
    email                : torcs@free.fr
    version              : $Id: screen.cpp,v 1.23.2.1 2008/11/09 17:50:22 berniw Exp $
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
    Screen management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: screen.cpp,v 1.23.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	screen
*/

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif /* WIN32 */

#include <tgfclient.h>
#include <portability.h>
#include "gui.h"
#include "fg_gm.h"
#include "glfeatures.h"

//#ifndef WIN32
//#define USE_RANDR_EXT
//#endif // WIN32

#ifdef USE_RANDR_EXT
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#endif // USE_RANDR_EXT

static int GfScrWidth;
static int GfScrHeight;
static int GfViewWidth;
static int GfViewHeight;
static int GfScrCenX;
static int GfScrCenY;

static void	*scrHandle = NULL;
static char 	buf[1024];

static int usedGM = 0;
#if !defined(FREEGLUT) && !defined(WIN32)
static int usedFG = 0;
#endif

#ifdef USE_RANDR_EXT
static char	**Res = NULL;
static int nbRes = 0;
#else // USE_RANDR_EXT
static char	*Res[] = {"64x64", "640x480", "800x600", "1024x768", "1152x768", "1152x864", "1200x854", "1200x960", "1280x1024", "1400x900", "1600x1200", "1680x1050", "1920x1200", "320x200","256x128"};
static const int nbRes = sizeof(Res) / sizeof(Res[0]);
#endif // USE_RANDR_EXT

static char	*Mode[] = {"Full-screen mode", "Window mode"};
static char *VInit[] = {GFSCR_VAL_VINIT_COMPATIBLE, GFSCR_VAL_VINIT_BEST};
static char	*Depthlist[] = {"24", "32", "16"};

//static const int nbRes = sizeof(Res) / sizeof(Res[0]);
static const int nbMode = sizeof(Mode) / sizeof(Mode[0]);
static const int nbVInit = sizeof(VInit) / sizeof(VInit[0]);
static const int nbDepth = sizeof(Depthlist) / sizeof(Depthlist[0]);

static int	curRes = 0;
static int	curMode = 0;
static int	curDepth = 0;
static int curVInit = 0;

static int	curMaxFreq = 75;
#ifdef WIN32
static int	MaxFreqId;
#endif

static int	ResLabelId;
static int	DepthLabelId;
static int	ModeLabelId;
static int VInitLabelId;

static void	*paramHdle;

static float LabelColor[] = {1.0, 0.0, 1.0, 1.0};


void
gfScreenInit(void)
{
#ifdef USE_RANDR_EXT
	// Get display, screen and root window handles.
	char *displayname = getenv("DISPLAY");
	if (displayname == NULL) {
		displayname = ":0.0";
	}

	Display *display = XOpenDisplay(displayname);

	if( display != NULL) {
		// If we have a display fill in the resolutions advertised by Xrandr.
		int screen = DefaultScreen(display);
    	Window root = RootWindow(display, screen);

		XRRScreenConfiguration *screenconfig = XRRGetScreenInfo (display, root);
		if (screenconfig != NULL) {
			int i, j, nsize;
			XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);

			if (nsize > 0) {
				// Check if 64x64, 320x200, 640x480, 800x600 are available, construct a mode wish list.
				int check_resx[] = {64, 320, 640, 800};
				int check_resy[] = {64, 240, 480, 600};
				bool mode_in_list[] = {false, false, false};
				int add_modes = sizeof(check_resx)/sizeof(check_resx[0]);

				for (i = 0; i < nsize; i++) {
					for (j = 0; j < 3; j++) {
						if ((mode_in_list[j] == false) && (sizes[i].width == check_resx[j])) {
							if (sizes[i].height == check_resy[j]) {
								// Mode already in list.
								mode_in_list[j] = true;
								add_modes--;
							}
						}
					}
				}

				const int bufsize = 20;
				char buffer[bufsize];
				Res = (char**) malloc(sizeof(char *)*(nsize+add_modes));
				int resx[nsize+add_modes];
				int resy[nsize+add_modes];
				for (i = 0; i < nsize+add_modes; i++) {
					if (i < nsize) {
						// Add mode from screenconfig (system).
						snprintf(buffer, bufsize, "%dx%d", sizes[i].width, sizes[i].height);
						Res[i] = strndup(buffer, bufsize);
						resx[i] = sizes[i].width;
						resy[i] = sizes[i].height;
					} else {
						// Add mode from wish list.
						unsigned int j;
						for (j = 0; j < sizeof(check_resx)/sizeof(check_resx[0]); j++) {
							if (mode_in_list[j] == false) {
								mode_in_list[j] = true;
								snprintf(buffer, bufsize, "%dx%d", check_resx[j], check_resy[j]);
								Res[i] = strndup(buffer, bufsize);
								resx[i] = check_resx[j];
								resy[i] = check_resy[j];
								break;
							}
						}
					}

					// Stupid sorting (not much elements, don't worry).
					int j;
					for (j = i; j > 0; j--) {
						if ((resx[j] < resx[j-1]) ||
							(resx[j] == resx[j-1] && resy[j] < resy[j-1]))
						{
							int tx, ty;
							char *tc;
							tx = resx[j-1];
							ty = resy[j-1];
							resx[j-1] = resx[j];
							resy[j-1] = resy[j];
							resx[j] = tx;
							resy[j] = ty;
							tc = Res[j-1];
							Res[j-1] = Res[j];
							Res[j] = tc;
						} else {
							break;
						}
					}
				}

				nbRes = nsize + add_modes;
			}

			XRRFreeScreenConfigInfo(screenconfig);
		}
		XCloseDisplay(display);
	}

	if (Res == NULL || nbRes == 0) {
		// We failed to get a handle to the display, so fill in some defaults.
		GfOut("Failed to initialize resolutions for display '%s'", XDisplayName(displayname));
		nbRes = 9;
		Res = (char **) malloc(sizeof(char *)*nbRes);
		Res[0] = strdup("640x480");
		Res[1] = strdup("800x600");
		Res[2] = strdup("1024x768");
		Res[3] = strdup("1152x864");
		Res[4] = strdup("1200x960");
		Res[5] = strdup("1280x1024");
		Res[6] = strdup("1600x1200");
		Res[7] = strdup("320x200");
		Res[8] = strdup("256x128");
		Res[9] = strdup("64x64");// added
	}
#endif // USE_RANDR_EXT
}

static void Reshape(int width, int height)
{
    glViewport( (width-GfViewWidth)/2, (height-GfViewHeight)/2, GfViewWidth,  GfViewHeight);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, 640.0, 0.0, 480.0, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    GfScrWidth = width;
    GfScrHeight = height;
    GfScrCenX = width / 2;
    GfScrCenY = height / 2;
}

void GfScrInit(int argc, char *argv[])
{
    int		Window;
    int		xw, yw;
    int		winX, winY;
    void	*handle;
    char	*fscr;
	char	*vinit;
    int		fullscreen;
    int		maxfreq;
    int		i, depth;

    sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
    handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    xw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, 640);
    yw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, 480);
    winX = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, xw);
    winY = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, yw);
    depth = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, 32);
    maxfreq = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, 160);
    GfViewWidth = xw;
    GfViewHeight = yw;
    GfScrCenX = xw / 2;
    GfScrCenY = yw / 2;

	// The fullscreen hack must be run before glutInit, such that glut gets the right screen size, etc.
	fscr = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, GFSCR_VAL_NO);
	fullscreen = 0;
#if !defined(FREEGLUT) && !defined(WIN32)
	if (strcmp(fscr, GFSCR_VAL_YES) == 0) {	// Resize the screen
		GfOut ("Freeglut not detected...\n");
		for (i = maxfreq; i > 59; i--) {
			sprintf(buf, "%dx%d:%d@%d", winX, winY, depth, i);
			GfOut("Trying %s mode\n", buf);
			fglutGameModeString(buf);
			if (fglutEnterGameMode()) {
				GfOut("OK done for %s\n", buf);
				usedFG = 1;
				break;
			}
		}
	}
#endif

	vinit = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);

    glutInit(&argc, argv);

	// Depending on "video mode init" setting try to get the best mode or try to get a mode in a safe way...
	// This is a workaround for driver/glut/glx bug, which lie about the capabilites of the visual.

	if (strcmp(vinit, GFSCR_VAL_VINIT_BEST) == 0) {

		// Try to get "best" videomode, z-buffer >= 24bit, visual with alpha channel,
		// antialiasing support.

		int visualDepthBits = 24;
		bool visualSupportsMultisample = true;
		bool visualSupportsAlpha = true;

		glutInitDisplayString("rgba double depth>=24 samples alpha");

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without antialiasing support.
			visualDepthBits = 24;
			visualSupportsMultisample = false;
			visualSupportsAlpha = true;
			glutInitDisplayString("rgba double depth>=24 alpha");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without alpha channel.
			visualDepthBits = 24;
			visualSupportsMultisample = true;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=24 samples");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without antialiasing and alpha support.
			visualDepthBits = 24;
			visualSupportsMultisample = false;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=24");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without 24 bit z-Buffer and without antialiasing.
			visualDepthBits = 16;
			visualSupportsMultisample = false;
			visualSupportsAlpha = true;
			glutInitDisplayString("rgba double depth>=16 alpha");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without 24 bit z-Buffer, without antialiasing and without alpha.
			visualDepthBits = 16;
			visualSupportsMultisample = false;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=16");
		}

		printf("Visual Properties Report\n");
		printf("------------------------\n");

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// All failed.
			printf("The minimum display requirements are not fulfilled.\n");
			printf("We need a double buffered RGB visual with a 16 bit depth buffer at least.\n");
			// Try fallback as last resort.
			printf("Trying generic initialization, fallback.\n");
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		} else {
			// We have got a mode, report the properties.
			printf("z-buffer depth: %d (%s)\n", visualDepthBits, visualDepthBits < 24 ? "bad" : "good");
			printf("multisampling : %s\n", visualSupportsMultisample ? "available" : "no");
			printf("alpha bits    : %s\n", visualSupportsAlpha ? "available" : "no");
			if (visualDepthBits < 24) {
				// Show a hint if the z-buffer depth is not optimal.
				printf("The z-buffer resolution is below 24 bit, you will experience rendering\n");
				printf("artefacts. Try to improve the setup of your graphics board or look\n");
				printf("for an alternate driver.\n");
			}
		}
	} else {
		// Compatibility mode.
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		printf("Visual Properties Report\n");
		printf("------------------------\n");
		printf("Compatibility mode, properties unknown.\n");
	}


	if (strcmp(fscr, GFSCR_VAL_YES) == 0) {
		for (i = maxfreq; i > 59; i--) {
			sprintf(buf, "%dx%d:%d@%d", winX, winY, depth, i);
			glutGameModeString(buf);
			GfOut("2 - Trying %s mode\n", buf);
			if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
				GfOut("2- %s mode Possible\n", buf);
				glutEnterGameMode();
				if (glutGameModeGet(GLUT_GAME_MODE_DISPLAY_CHANGED)) {
					GfOut("Use GameMode %s\n", buf);
					usedGM = 1;
					fullscreen = 1;
					break;
				} else {
					glutLeaveGameMode();
				}
			}
		}
	}

	if (!fullscreen) {
	/* Give an initial size and position so user doesn't have to place window */
		glutInitWindowPosition(0, 0);
		glutInitWindowSize(winX, winY);
		Window = glutCreateWindow(argv[0]);
		if (!Window) {
			printf("Error, couldn't open window\n");
			GfScrShutdown();
			exit(1);
		}
	}

	if ((strcmp(fscr, GFSCR_VAL_YES) == 0) && (!fullscreen)) {
		/* glutVideoResize(0, 0, winX, winY); */
		glutFullScreen();
	}

    GfParmReleaseHandle(handle);

    glutReshapeFunc( Reshape );

	checkGLFeatures();
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
    if (usedGM) {
	glutLeaveGameMode();
    }
#if !defined(FREEGLUT) && !defined(WIN32)
    if (usedFG) {
	fglutLeaveGameMode();
    }
#endif

#ifdef USE_RANDR_EXT
	int i;
	for (i = 0; i < nbRes; i++) {
		free(Res[i]);
	}
	free(Res);
#endif // USE_RANDR_EXT
}


/** Get the screen and viewport sizes.
    @ingroup	screen
    @param	scrw	address of screen with
    @param	scrh	address of screen height
    @param	vieww	address of viewport with
    @param	viewh	address of viewport height
    @return	none
 */
void GfScrGetSize(int *scrw, int *scrh, int *vieww, int *viewh)
{
    *scrw = GfScrWidth;
    *scrh = GfScrHeight;
    *vieww = GfViewWidth;
    *viewh = GfViewHeight;
}

static void
saveParams(void)
{
	int x, y, bpp;

	sscanf(Res[curRes], "%dx%d", &x, &y);
	sscanf(Depthlist[curDepth], "%d", &bpp);

	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, x);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, y);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, x);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, y);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, bpp);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, curMaxFreq);

	GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, VInit[curVInit]);

	if (curMode == 0) {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "yes");
	} else {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "no");
	}
	GfParmWriteFile(NULL, paramHdle, "Screen");
}


void
GfScrReinit(void * /* dummy */)
{
    int retcode = 0;
	static const int CMDSIZE = 1024;
	char cmd[CMDSIZE];

#ifndef WIN32
    char	*arg[8];
    int		curArg;
#endif

    saveParams();

#ifdef WIN32
	snprintf(cmd, CMDSIZE, "%swtorcs.exe", GetLocalDir());
    int i;
	for (i = 0; i < CMDSIZE && cmd[i] != NULL; i++) {
		if (cmd[i] == '/') {
			cmd[i] = '\\';
		}
	}

	char cmdarg[CMDSIZE];
	snprintf(cmdarg, CMDSIZE, "\"%swtorcs.exe\"", GetLocalDir());
	for (i = 0; i < CMDSIZE && cmdarg[i] != NULL; i++) {
		if (cmdarg[i] == '/') {
			cmdarg[i] = '\\';
		}
	}

	retcode = execlp(cmd, cmdarg, (const char *)NULL);
#else
    GfScrShutdown();

    sprintf (cmd, "%storcs-bin", GetLibDir ());
    memset (arg, 0, sizeof (arg));
    curArg = 0;
    if (GfuiMouseHW) {
	arg[curArg++] = "-m";
    }

    if (strlen(GetLocalDir ())) {
	arg[curArg++] = "-l";
	arg[curArg++] = GetLocalDir ();
    }

    if (strlen(GetLibDir ())) {
	arg[curArg++] = "-L";
	arg[curArg++] = GetLibDir ();
    }

    if (strlen(GetDataDir ())) {
	arg[curArg++] = "-D";
	arg[curArg++] = GetDataDir ();
    }

    switch (curArg) {
    case 0:
	retcode = execlp (cmd, cmd, (const char *)NULL);
	break;
    case 1:
	retcode = execlp (cmd, cmd, arg[0], (const char *)NULL);
	break;
    case 2:
	retcode = execlp (cmd, cmd, arg[0], arg[1], (const char *)NULL);
	break;
    case 3:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], (const char *)NULL);
	break;
    case 4:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], (const char *)NULL);
	break;
    case 5:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], (const char *)NULL);
	break;
    case 6:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], (const char *)NULL);
	break;
    case 7:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], (const char *)NULL);
	break;
    case 8:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], (const char *)NULL);
	break;
    }


#endif
    if (retcode) {
	perror("torcs");
	exit(1);
    }
}

static void
updateLabelText(void)
{
    GfuiLabelSetText (scrHandle, ResLabelId, Res[curRes]);
    GfuiLabelSetText (scrHandle, DepthLabelId, Depthlist[curDepth]);
    GfuiLabelSetText (scrHandle, ModeLabelId, Mode[curMode]);
#ifdef WIN32
    sprintf(buf, "%d", curMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqId, buf);
#endif
	GfuiLabelSetText (scrHandle, VInitLabelId, VInit[curVInit]);
}

static void
ResPrevNext(void *vdelta)
{
    long delta = (long)vdelta;
    curRes += (int)delta;
    if (curRes < 0) {
	curRes = nbRes - 1;
    } else {
	if (curRes >= nbRes) {
	    curRes = 0;
	}
    }
    updateLabelText();
}

static void
DepthPrevNext(void *vdelta)
{
    long delta = (long)vdelta;

    curDepth += (int)delta;
    if (curDepth < 0) {
	curDepth = nbDepth - 1;
    } else {
	if (curDepth >= nbDepth) {
	    curDepth = 0;
	}
    }
    updateLabelText();
}

static void
ModePrevNext(void *vdelta)
{
    long delta = (long)vdelta;

    curMode += (int)delta;
    if (curMode < 0) {
	curMode = nbMode - 1;
    } else {
	if (curMode >= nbMode) {
	    curMode = 0;
	}
    }
    updateLabelText();
}


static void
VInitPrevNext(void *vdelta)
{
	long delta = (long)vdelta;

	curVInit += (int)delta;
	if (curVInit < 0) {
		curVInit = nbVInit - 1;
	} else {
		if (curVInit >= nbVInit) {
			curVInit = 0;
		}
	}
	updateLabelText();
}


static void
initFromConf(void)
{
	int x, y, bpp;
	int i;

	x = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, NULL, 640);
	y = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, NULL, 480);

	sprintf(buf, "%dx%d", x, y);
	for (i = 0; i < nbRes; i++) {
		if (!strcmp(buf, Res[i])) {
			curRes = i;
			break;
		}
	}

	if (!strcmp("yes", GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "yes"))) {
		curMode = 0;
	} else {
		curMode = 1;
	}

	curVInit = 0;
	char *tmp = GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);
	for (i = 0; i < nbVInit; i++) {
		if (strcmp(VInit[i], tmp) == 0) {
			curVInit = i;
			break;
		}
	}

	bpp = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, NULL, 24);
	sprintf(buf, "%d", bpp);
	for (i = 0; i < nbDepth; i++) {
		if (!strcmp(buf, Depthlist[i])) {
			curDepth = i;
			break;
		}
	}

	curMaxFreq = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, NULL, curMaxFreq);
}

#ifdef WIN32
static void
ChangeMaxFreq(void * /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, MaxFreqId);
    curMaxFreq = (int)strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", curMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqId, buf);
}
#endif

static void
onActivate(void * /* dummy */)
{
    initFromConf();
    updateLabelText();
}


/** Create and activate the video options menu screen.
    @ingroup	screen
    @param	precMenu	previous menu to return to
*/
void *
GfScrMenuInit(void *precMenu)
{
    int		y, x1, x2;
#ifndef WIN32
	const int yoffset1 = 30, yoffset2 = 60;
#else // WIN32
	const int yoffset1 = 30, yoffset2 = 40;
#endif // WIN32
    sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
    paramHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    if (scrHandle) return scrHandle;

    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
    GfuiTitleCreate(scrHandle, "Screen configuration", 0);
    GfuiScreenAddBgImg(scrHandle, "data/img/splash-graphic.png");

    x1 = 200;
    x2 = 440;
    y = 400;
    GfuiLabelCreate(scrHandle,
		    "Screen Resolution",
		    GFUI_FONT_LARGE,
		    320, y, GFUI_ALIGN_HC_VB,
		    0);

    y -= yoffset1; //30;
    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left-pushed.png",
		       x1, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)-1, ResPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_LEFT, "Previous Resolution", (void*)-1, ResPrevNext, NULL);

    ResLabelId = GfuiLabelCreate(scrHandle,
				 "",
				 GFUI_FONT_LARGE_C,
				 320, y, GFUI_ALIGN_HC_VB,
				 30);
    GfuiLabelSetColor(scrHandle, ResLabelId, LabelColor);

    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right-pushed.png",
		       x2, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)1, ResPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_RIGHT, "Next Resolution", (void*)1, ResPrevNext, NULL);

    y -= yoffset2; //60;
    GfuiLabelCreate(scrHandle,
		    "Color Depth",
		    GFUI_FONT_LARGE,
		    320, y, GFUI_ALIGN_HC_VB,
		    0);
    y -= yoffset1; //30;
    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left-pushed.png",
		       x1, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)-1, DepthPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    DepthLabelId = GfuiLabelCreate(scrHandle,
				   "",
				   GFUI_FONT_LARGE_C,
				   320, y, GFUI_ALIGN_HC_VB,
				   30);
    GfuiLabelSetColor(scrHandle, DepthLabelId, LabelColor);

    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right-pushed.png",
		       x2, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)1, DepthPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    y -= yoffset2; //60;
    GfuiLabelCreate(scrHandle,
		    "Display Mode",
		    GFUI_FONT_LARGE,
		    320, y, GFUI_ALIGN_HC_VB,
		    0);

    y -= yoffset1; //30;
    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left-pushed.png",
		       x1, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)-1, ModePrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    ModeLabelId = GfuiLabelCreate(scrHandle,
				  "",
				  GFUI_FONT_LARGE_C,
				  320, y, GFUI_ALIGN_HC_VB,
				  30);
    GfuiLabelSetColor(scrHandle, ModeLabelId, LabelColor);

    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right-pushed.png",
		       x2, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)1, ModePrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    GfuiAddKey(scrHandle, 'f', "Display Mode", (void*)1, ModePrevNext, NULL);

#ifdef WIN32
    y -= yoffset2; //60;
    GfuiLabelCreate(scrHandle,
		    "Max Frequency",
		    GFUI_FONT_LARGE,
		    320, y, GFUI_ALIGN_HC_VB,
		    0);
    y -= yoffset1; //30;
    MaxFreqId = GfuiEditboxCreate(scrHandle, "", GFUI_FONT_MEDIUM_C,
				   275, y, 0, 8, NULL, (tfuiCallback)NULL, ChangeMaxFreq);
#endif

	y -= yoffset2; //60;
    GfuiLabelCreate(scrHandle,
		    "Video Mode Initialization",
		    GFUI_FONT_LARGE,
		    320, y, GFUI_ALIGN_HC_VB,
		    0);
    y -= yoffset1; //30;
    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left.png",
		       "data/img/arrow-left-pushed.png",
		       x1, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)-1, VInitPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    VInitLabelId = GfuiLabelCreate(scrHandle,
				  "",
				  GFUI_FONT_LARGE_C,
				  320, y, GFUI_ALIGN_HC_VB,
				  30);
    GfuiLabelSetColor(scrHandle, VInitLabelId, LabelColor);

    GfuiGrButtonCreate(scrHandle,
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right.png",
		       "data/img/arrow-right-pushed.png",
		       x2, y, GFUI_ALIGN_HC_VB, 0,
		       (void*)1, VInitPrevNext,
		       NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);


    GfuiAddKey(scrHandle, 13, "Apply Mode", NULL, GfScrReinit, NULL);
    GfuiButtonCreate(scrHandle, "Apply", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     NULL, GfScrReinit, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    GfuiAddKey(scrHandle, 27, "Cancel", precMenu, GfuiScreenActivate, NULL);
    GfuiButtonCreate(scrHandle, "Back", GFUI_FONT_LARGE, 430, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     precMenu, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    return scrHandle;
}



int GfuiGlutExtensionSupported(char *str)
{
    return glutExtensionSupported(str);
}
