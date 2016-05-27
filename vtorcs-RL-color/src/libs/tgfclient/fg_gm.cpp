/***************************************************************************

    file        : fg_gm.cpp
    created     : Sat Mar  8 14:40:50 CET 2003
    copyright   : (C) 2003 by Eric Espi�
    email       : eric.espie@torcs.org   
    version     : $Id: fg_gm.cpp,v 1.5.2.1 2008/11/09 17:50:22 berniw Exp $                                  

 ***************************************************************************/

/*
 * The freeglut library private include file.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** @file
    	Missing X11 gamemode in glut, taken from <a href="http://freeglut.sf.net">freeglut</a>
    @version	$Id: fg_gm.cpp,v 1.5.2.1 2008/11/09 17:50:22 berniw Exp $
*/

/*
	2004/10/03 Bernhard Wymann: Added Randr support, bugfixes.
*/

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tgfclient.h>
#include "fg_gm.h"

#ifndef WIN32

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef USE_RANDR_EXT
#include <X11/extensions/Xrandr.h>
#else // USE_RANDR_EXT
#include <X11/extensions/xf86vmode.h>
#endif // USE_RANDR_EXT


static int fgInitDone = 0;

typedef struct tagSFG_Display SFG_Display;
struct tagSFG_Display
{
	Display*			display;            // The display we are being run in.
	int					Screen;             // The screen we are about to use.
	Window				RootWindow;			//The screen's root window.

	int					ScreenWidth;		// The screen's width in pixels.
	int					ScreenHeight;		// The screen's height in pixels.
	int					error_base;			// Base error number of the extension.
	int					event_base;			// Base event number of the extension.

#ifdef USE_RANDR_EXT
	Rotation			rotation;			// The rotation mode.
	short				rate;				// Display refresh rate.
#else //USE_RANDR_EXT
	XF86VidModeModeLine DisplayMode;		// Current screen's display settings.
	int					DisplayModeClock;   // The display mode's refresh rate.
#endif //USE_RANDR_EXT
};


/*
 * A helper structure holding two ints and a boolean
 */
typedef struct tagSFG_XYUse SFG_XYUse;
struct tagSFG_XYUse
{
    int         X, Y;               /* The two integers...               */
};

/*
 * This structure holds different freeglut settings
 */
typedef struct tagSFG_State SFG_State;
struct tagSFG_State
{
	SFG_XYUse	GameModeSize;       // The game mode screen's dimensions.
	int			GameModeDepth;      // The pixel depth for game mode.
	int			GameModeRefresh;	// The refresh rate for game mode.
};

/*
 * A structure pointed by g_pDisplay holds all information
 * regarding the display, screen, root window etc.
 */
static SFG_Display fgDisplay;

/*
 * The settings for the current freeglut session
 */
static SFG_State fgState;


/*
 * A call to this function should initialize all the display stuff...
 */
static void fgInitialize(void)
{
	char* displayName;
	displayName = getenv("DISPLAY");
	if (!displayName) {
		displayName = ":0.0";
	}

	// Have the display created.
	fgDisplay.display = XOpenDisplay(displayName);
	if(fgDisplay.display == NULL) {
		// Failed to open a display. That's no good.
		GfOut( "failed to open display '%s'", XDisplayName( displayName ) );
	}

	// Grab the default screen for the display we have just opened.
	fgDisplay.Screen = DefaultScreen(fgDisplay.display);

	// The same applying to the root window
	fgDisplay.RootWindow = RootWindow(fgDisplay.display, fgDisplay.Screen);

	// Grab the logical screen's geometry.
	fgDisplay.ScreenWidth = DisplayWidth(fgDisplay.display, fgDisplay.Screen);
	fgDisplay.ScreenHeight = DisplayHeight(fgDisplay.display, fgDisplay.Screen);

#ifdef USE_RANDR_EXT
	int major, minor;
   	XRRQueryVersion(fgDisplay.display, &major, &minor);
	printf("Randr version: %d.%d\n", major, minor);
	XRRQueryExtension(fgDisplay.display, &fgDisplay.event_base, &fgDisplay.error_base);
#else //USE_RANDR_EXT
	int major, minor;
   	XF86VidModeQueryVersion(fgDisplay.display, &major, &minor);
	printf("Xxf86vm version: %d.%d\n", major, minor);
	XF86VidModeQueryExtension(fgDisplay.display, &fgDisplay.event_base, &fgDisplay.error_base);
#endif //USE_RANDR_EXT
}

/*
 * Remembers the current visual settings, so that
 * we can change them and restore later...
 */
static void fghRememberState( void )
{
	// This highly depends on the XFree86 extensions, not approved as X Consortium standards.
	if (!fgInitDone) {
		fgInitialize();
		fgInitDone = 1;

		// Query the current display settings.
#ifdef USE_RANDR_EXT
		XRRScreenConfiguration *screenconfig = XRRGetScreenInfo(fgDisplay.display, fgDisplay.RootWindow);
		if (screenconfig != NULL) {
			SizeID size = XRRConfigCurrentConfiguration (screenconfig, &fgDisplay.rotation);
			int nsize;
			XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);
			fgDisplay.ScreenWidth = sizes[size].width;
			fgDisplay.ScreenHeight = sizes[size].height;
			fgDisplay.rate = XRRConfigCurrentRate(screenconfig);
			//printf("x: %d, y: %d, rate: %d\n", fgDisplay.ScreenWidth, fgDisplay.ScreenHeight, fgDisplay.rate);
			XRRFreeScreenConfigInfo(screenconfig);
		} else {
			// Hmm, what to do here, exit?
		}
#else //USE_RANDR_EXT
		XF86VidModeGetModeLine(
			fgDisplay.display,
			fgDisplay.Screen,
			&fgDisplay.DisplayModeClock,
			&fgDisplay.DisplayMode
		);
#endif //USE_RANDR_EXT
	}
}

/*
 * Restores the previously remembered visual settings
 */
static void fghRestoreState( void )
{
#ifdef USE_RANDR_EXT
	XRRScreenConfiguration *screenconfig = XRRGetScreenInfo(fgDisplay.display, fgDisplay.RootWindow);
	if (screenconfig != NULL) {
		int nsize;
		XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);
		// Search for the display resolution ID for the old video mode.
		SizeID size;
		for (size = 0; size < nsize; size++) {
			if (sizes[size].width == fgDisplay.ScreenWidth && sizes[size].height == fgDisplay.ScreenHeight)
			break;
    	}

		XSelectInput(fgDisplay.display, fgDisplay.RootWindow, StructureNotifyMask);
		XRRSelectInput(fgDisplay.display, fgDisplay.RootWindow, RRScreenChangeNotifyMask);
		Status status = RRSetConfigFailed;
		status = XRRSetScreenConfigAndRate(
			fgDisplay.display,
			screenconfig,
			fgDisplay.RootWindow,
			size,
			fgDisplay.rotation,
			fgDisplay.rate,
			CurrentTime
		);

		if (status == RRSetConfigSuccess) {
			XEvent ev;
			do {
				XNextEvent(fgDisplay.display, &ev);
				XRRUpdateConfiguration(&ev);
			} while ((ev.type != ConfigureNotify) && ((ev.type - fgDisplay.event_base) != RRScreenChangeNotify));
			XSync(fgDisplay.display, True);
		}
		XRRFreeScreenConfigInfo(screenconfig);
	} else {
		// Hmm, what to do here, exit?
	}
#else //USE_RANDR_EXT
	/*
     * This highly depends on the XFree86 extensions, not approved as X Consortium standards
     */
	XF86VidModeModeInfo** displayModes;
    int i, displayModesCount;

    /*
     * Query for all the display available...
     */
	XF86VidModeGetAllModeLines(
        fgDisplay.display,
        fgDisplay.Screen,
        &displayModesCount,
        &displayModes
    );

    /*
     * Check every of the modes looking for one that matches our demands
     */
	for( i=0; i<displayModesCount; i++ ) {
		if( displayModes[ i ]->hdisplay == fgDisplay.DisplayMode.hdisplay &&
			displayModes[ i ]->vdisplay == fgDisplay.DisplayMode.vdisplay &&
			displayModes[ i ]->dotclock == (unsigned int)fgDisplay.DisplayModeClock )
		{
            /*
             * OKi, this is the display mode we have been looking for...
             */
			XSync(fgDisplay.display, True);
			XF86VidModeSwitchToMode(fgDisplay.display, fgDisplay.Screen, displayModes[ i ]);
			// FIXME: usleep: Hack to avoid screen corruption (does not work always). Do you know a clean solution?
			usleep(100000);
			XSync(fgDisplay.display, False);
			XF86VidModeSetViewPort(fgDisplay.display, fgDisplay.Screen, 0, 0);
			// FIXME: usleep: Hack to avoid screen corruption (does not work always). Do you know a clean solution?
			usleep(100000);
			XSync(fgDisplay.display, False);
			XWarpPointer(
				fgDisplay.display,
				None,
				fgDisplay.RootWindow,
				0, 0, 0, 0,
				fgDisplay.ScreenWidth/2,
				fgDisplay.ScreenHeight/2
			);

			XFree(displayModes);
			if (fgDisplay.DisplayMode.privsize > 0) {
				// Cannot compile with c++! --> private keyword.
				//XFree(fgDisplay.DisplayMode.private);
			}
			XFlush(fgDisplay.display);
			return;
		}
	}
	XFree(displayModes);
	XFlush(fgDisplay.display);
#endif //USE_RANDR_EXT
}

#ifndef USE_RANDR_EXT
/*
 * Checks the display mode settings against user's preferences
 */
static int fghCheckDisplayMode( int width, int height, int depth, int refresh )
{
    /*
     * The desired values should be stored in fgState structure...
     */
    return( (width == fgState.GameModeSize.X) && (height == fgState.GameModeSize.Y) &&
            (depth == fgState.GameModeDepth)  && (refresh == fgState.GameModeRefresh) );
}
#endif //USE_RANDR_EXT

/*
 * Changes the current display mode to match user's settings
 */
static int fghChangeDisplayMode( int /* haveToTest */ )
{
#ifdef USE_RANDR_EXT
	XRRScreenConfiguration *screenconfig = XRRGetScreenInfo(fgDisplay.display, fgDisplay.RootWindow);
	if (screenconfig != NULL) {
		int nsize;
		XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);
		// Search for the requested display resolution.
		SizeID size;
		for (size = 0; size < nsize; size++) {
			if (sizes[size].width == fgState.GameModeSize.X && sizes[size].height == fgState.GameModeSize.Y) {
				break;
			}
    	}

		if (size >= nsize) {
			// We did not find a matching resolution.
			XRRFreeScreenConfigInfo(screenconfig);
			return FALSE;
		} else {
			// Ok, resolution is ok, now check the refresh rate.
			int nrate;
			short *rates = XRRConfigRates(screenconfig, size, &nrate);
			int rate;
			for (rate = 0; rate < nrate; rate++) {
				if (rates[rate] == fgState.GameModeRefresh) {
					break;
				}
			}

			if (rate >= nrate) {
				// We did not find a matching refresh rate.
				XRRFreeScreenConfigInfo(screenconfig);
				return FALSE;
			} else {
				// We have everything, try to switch.
				XSelectInput(fgDisplay.display, fgDisplay.RootWindow, StructureNotifyMask);
				XRRSelectInput(fgDisplay.display, fgDisplay.RootWindow, RRScreenChangeNotifyMask);
				Status status = RRSetConfigFailed;
				status = XRRSetScreenConfigAndRate(
					fgDisplay.display,
					screenconfig,
					fgDisplay.RootWindow,
					size,
					fgDisplay.rotation,
					rates[rate],
					CurrentTime
				);

				XRRFreeScreenConfigInfo(screenconfig);

				if (status == RRSetConfigSuccess) {
					XEvent ev;
					do {
						XNextEvent(fgDisplay.display, &ev);
						XRRUpdateConfiguration(&ev);
					} while ((ev.type != ConfigureNotify) && ((ev.type - fgDisplay.event_base) != RRScreenChangeNotify));
					XSync(fgDisplay.display, True);
					return TRUE;
				} else {
					return FALSE;
				}
			}
		}
	}

	return FALSE;
#else //USE_RANDR_EXT
    /*
     * This highly depends on the XFree86 extensions, not approved as X Consortium standards
     */
	XF86VidModeModeInfo** displayModes;
    int i, displayModesCount;

    /*
     * Query for all the display available...
     */
    XF86VidModeGetAllModeLines(
        fgDisplay.display,
        fgDisplay.Screen,
        &displayModesCount,
        &displayModes
    );

    /*
     * Check every of the modes looking for one that matches our demands
     */
    for( i=0; i<displayModesCount; i++ )
    {
		// Compute the displays refresh rate, dotclock comes in kHz.
		int refresh = displayModes[ i ]->dotclock*1000/(displayModes[ i ]->htotal*displayModes[ i ]->vtotal);

		if( fghCheckDisplayMode( displayModes[ i ]->hdisplay, displayModes[ i ]->vdisplay,
                                     fgState.GameModeDepth, refresh ) )
        {
			// Added failure code path, who can guarantee if that works always?
			// If i == 0, then the videomode we prefer is already set, so the short circuit
			// "||" does not evaluate the (in this case unnecessary) XF86VidModeSwitchToMode.
			//XFlush(fgDisplay.display);
			//XSync(fgDisplay.display, True);
			if ((i == 0) || (XF86VidModeSwitchToMode(
				fgDisplay.display,
				fgDisplay.Screen,
				displayModes[ i ]) > 0))
			{
				XF86VidModeSetViewPort(
					fgDisplay.display,
					fgDisplay.Screen,
					(fgDisplay.ScreenWidth - fgState.GameModeSize.X) / 2,
					(fgDisplay.ScreenHeight - fgState.GameModeSize.Y) / 2
				);

				// Move the mouse pointer in the middle of the viewport.
				XWarpPointer(
					fgDisplay.display,
					None,
					fgDisplay.RootWindow,
					0, 0, 0, 0,
					fgDisplay.ScreenWidth/2,
					fgDisplay.ScreenHeight/2
				);
				XFree(displayModes);
				XFlush(fgDisplay.display);
				return TRUE;
			} else {
				XFree(displayModes);
				XFlush(fgDisplay.display);
				return( FALSE );
			}
        }
    }

    /*
     * Something must have went wrong
     */
	XFree(displayModes);
	XFlush(fgDisplay.display);
    return( FALSE );
#endif //USE_RANDR_EXT
}

#endif /* WIN32 */

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Sets the game mode display string
 */
void fglutGameModeString( const char* string )
{
#ifndef WIN32
    int width = 640, height = 480, depth = 16, refresh = 72;

    /*
     * This one seems a bit easier than glutInitDisplayString. The bad thing
     * about it that I was unable to find the game mode string definition, so
     * that I assumed it is: "[width]x[height]:[depth]@[refresh rate]", which
     * appears in all GLUT game mode programs I have seen to date.
     */
    if( sscanf( string, "%ix%i:%i@%i", &width, &height, &depth, &refresh ) != 4 )
        if( sscanf( string, "%ix%i:%i", &width, &height, &depth ) != 3 )
            if( sscanf( string, "%ix%i@%i", &width, &height, &refresh ) != 3 )
                if( sscanf( string, "%ix%i", &width, &height ) != 2 )
                    if( sscanf( string, ":%i@%i", &depth, &refresh ) != 2 )
                        if( sscanf( string, ":%i", &depth ) != 1 )
                            if( sscanf( string, "@%i", &refresh ) != 1 )
                                GfOut( "unable to parse game mode string `%s'", string );

    /*
     * Hopefully it worked, and if not, we still have the default values
     */
    fgState.GameModeSize.X  = width;
    fgState.GameModeSize.Y  = height;
    fgState.GameModeDepth   = depth;
    fgState.GameModeRefresh = refresh;
#endif /* WIN32 */
}

/*
 * Enters the game mode
 */
int fglutEnterGameMode( void )
{
#ifndef WIN32
    /*
     * Remember the current resolution, etc.
     */
    fghRememberState();
    /*
     * We are ready to change the current screen's resolution now
     */
    if( fghChangeDisplayMode( FALSE ) == FALSE )
    {
        GfOut( "failed to change screen settings" );
        return( FALSE );
    }

    /*
     * Return successfull
     */
    return( TRUE );
#else /* WIN32 */
    return( FALSE );
#endif /* WIN32 */
}

/*
 * Leaves the game mode
 */
void fglutLeaveGameMode( void )
{
#ifndef WIN32
    /*
     * Then, have the desktop visual settings restored
     */
    fghRestoreState();
#endif /* WIN32 */
}
