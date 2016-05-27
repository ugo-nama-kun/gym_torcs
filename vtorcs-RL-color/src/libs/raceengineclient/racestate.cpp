/***************************************************************************

    file        : racestate.cpp
    created     : Sat Nov 16 12:00:42 CET 2002
    copyright   : (C) 2002 by Eric Espi�                       
    email       : eric.espie@torcs.org   
    version     : $Id: racestate.cpp,v 1.5 2005/08/17 20:48:39 berniw Exp $                                  

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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: racestate.cpp,v 1.5 2005/08/17 20:48:39 berniw Exp $
*/

/* The Race Engine State Automaton */

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>

#include "racemain.h"
#include "raceinit.h"
#include "raceengine.h"
#include "racegl.h"
#include "raceresults.h"
#include "racemanmenu.h"

#include "racestate.h"

static void *mainMenu;

/* State Automaton Init */
void
ReStateInit(void *prevMenu)
{
	mainMenu = prevMenu;
}


/* State Automaton Management         */
/* Called when a race menu is entered */
void
ReStateManage(void)
{
	int mode = RM_SYNC | RM_NEXT_STEP;

	do {
		switch (ReInfo->_reState) {
			case RE_STATE_CONFIG:
//				printf("RE_STATE_CONFIG\n");
				if (getTextOnly()==false)
				{
					GfOut("RaceEngine: state = RE_STATE_CONFIG\n");
					/* Display the race specific menu */
					mode = ReRacemanMenu();
					if (mode & RM_NEXT_STEP) {
						ReInfo->_reState = RE_STATE_EVENT_INIT;
					}
				}
				// GIUSE - VISION HERE!!
				else if (getVision())
				{
//					GfOut("RaceEngine: state = RE_STATE_CONFIG\n");
					/* Display the race specific menu */
//					mode = 
					ReRacemanMenu();
//					if (mode & RM_NEXT_STEP) {
						ReInfo->_reState = RE_STATE_EVENT_INIT;
//					}
				}
				else
				{
					ReInfo->_reState = RE_STATE_EVENT_INIT;
					ReRacemanMenu();
				}
				break;

			case RE_STATE_EVENT_INIT:
//				printf("RE_STATE_EVENT_INIT\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_EVENT_INIT\n");
				/* Load the event description (track and drivers list) */
				mode = ReRaceEventInit();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_PRE_RACE:
//				printf("RE_STATE_PRE_RACE\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_PRE_RACE\n");
				mode = RePreRace();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE_START;
				}
				break;

			case RE_STATE_RACE_START:
//				printf("RE_STATE_RACE_START\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_RACE_START\n");
				mode = ReRaceStart();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE;
				}
				break;

			case RE_STATE_RACE:
//				printf("RE_STATE_RACE\n");
				mode = ReUpdate();
				if (ReInfo->s->_raceState == RM_RACE_ENDED) {
					/* race finished */
					ReInfo->_reState = RE_STATE_RACE_END;
				} else if (mode & RM_END_RACE) {
					/* interrupt by player */
					ReInfo->_reState = RE_STATE_RACE_STOP;
				}
				break;

			case RE_STATE_RACE_STOP:
//				printf("RE_STATE_RACE_STOP\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_RACE_STOP\n");
				/* Interrupted by player */
				mode = ReRaceStop();
				if (mode & RM_NEXT_STEP) {
					if (RESTART==1)
					{
						RESTART=0;
						ReRaceCleanup();
						ReInfo->_reState = RE_STATE_PRE_RACE;
						if (getTextOnly()==false)
							GfuiScreenActivate(ReInfo->_reGameScreen);
            // GIUSE - VISION HERE!!
						else if (getVision())
							GfuiScreenActivate(ReInfo->_reGameScreen);
					}
					else
					{
						ReInfo->_reState = RE_STATE_RACE_END;
					}
				}
				break;

			case RE_STATE_RACE_END:
//				printf("RE_STATE_RACE_END\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_RACE_END\n");
				mode = ReRaceEnd();
				if(mode == RE_STATE_EXIT && getTextOnly())
				{
					ReInfo->_reState=RE_STATE_EXIT;
				}
				else
				{

					if (mode & RM_NEXT_STEP) {
						ReInfo->_reState = RE_STATE_POST_RACE;
					} else if (mode & RM_NEXT_RACE) {
						ReInfo->_reState = RE_STATE_RACE_START;
					}
				}
				break;

			case RE_STATE_POST_RACE:
//				printf("RE_STATE_POST_RACE\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_POST_RACE\n");
				mode = RePostRace();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_EVENT_SHUTDOWN:
//				printf("RE_STATE_EVENT_SHUTDOWN\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_EVENT_SHUTDOWN\n");
				mode = ReEventShutdown();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_SHUTDOWN:
//				printf("RE_STATE_SHUTDOWN\n");
			case RE_STATE_ERROR:
				printf("RE_STATE_ERROR\n");
				if (getTextOnly()==false)
					GfOut("RaceEngine: state = RE_STATE_SHUTDOWN\n");
				/* Back to race menu */
				ReInfo->_reState = RE_STATE_CONFIG;
				mode = RM_SYNC;
				break;

			case RE_STATE_EXIT:
//				printf("RE_STATE_EXIT\n");
				if (getTextOnly()==false)
					GfScrShutdown();
        // GIUSE - VISION HERE!!
				else if (getVision())
					GfScrShutdown();
				exit (0);		/* brutal isn't it ? */
				break;
		}


	} while ( ((mode & (RM_SYNC | RM_QUIT)) == RM_SYNC) || getTextOnly()) ;


	if (mode & RM_QUIT) {
		GfScrShutdown();
		exit (0);		/* brutal isn't it ? */
	}

	if (mode & RM_ACTIVGAMESCR) {
		GfuiScreenActivate(ReInfo->_reGameScreen);
	}
}

/* Change and Execute a New State  */
void
ReStateApply(void *vstate)
{
	long state = (long)vstate;

	ReInfo->_reState = (int)state;
	ReStateManage();
}
