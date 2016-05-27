/***************************************************************************

    file                 : robot.h
    created              : Sun Jan 30 22:59:40 CET 2000, 2002
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: robot.h,v 1.10 2006/02/20 20:18:07 berniw Exp $

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
    @version	$Id: robot.h,v 1.10 2006/02/20 20:18:07 berniw Exp $
    @ingroup	robotmodint
*/

/**
   @defgroup	robotmodint	Robots functions interface.
   This is the call flow of the robots callbacks during a race event.
   <br>The square boxes are for the race manager and
   <br>the ellipses are for the robot.
   @image	html robot_call.gif
   @ingroup	modint
*/
 
#ifndef _ROBOTV1_H_
#define _ROBOTV1_H_

#include <raceman.h>

#define ROB_IDENT	0

/** Callback prototype */
typedef void (*tfRbNewTrack)(int index, tTrack *track, void *carHandle, void **myCarSettings, tSituation *s);
/** Callback prototype */
typedef void (*tfRbNewRace) (int index, tCarElt *car, tSituation *s);
/** Callback prototype */
typedef void (*tfRbEndRace) (int index, tCarElt *car, tSituation *s);
/** Callback prototype */
typedef void (*tfRbDrive)   (int index, tCarElt *car, tSituation *s);
/** Callback prototype */
typedef void (*tfRbShutdown)(int index);
/** Callback prototype */
typedef int  (*tfRbPitCmd)  (int index, tCarElt* car, tSituation *s);

#define ROB_PIT_IM	0	/**< Immediate return from pit command */
#define ROB_PIT_MENU	1	/**< Call the interactive menu for pit command */


/** Interface for robots.
    @ingroup	robotmodint
*/
typedef struct RobotItf {
    tfRbNewTrack rbNewTrack;	/**< Give the robot the track view. Called for every track change or new race */
    tfRbNewRace  rbNewRace;	/**< Start a new race */
    tfRbEndRace  rbEndRace;	/**< End of the current race */
    tfRbDrive	 rbDrive;	/**< Drive during race */
    tfRbPitCmd	 rbPitCmd;	/**< Get the driver's pit commands.
				 <br>Returns:
				 - ROB_PIT_IM
				 - ROB_PIT_MENU
				*/
    tfRbShutdown rbShutdown;	/**< Called before the dll is unloaded */
    int		 index;		/**< Index used if multiple interfaces */
} tRobotItf;



/*
 * Parameters definitions for driver
 */
#define ROB_SECT_ROBOTS		"Robots"

#define ROB_LIST_INDEX		"index"

#define ROB_ATTR_NAME		"name"
#define ROB_ATTR_TEAM		"team"
#define ROB_ATTR_DESC		"desc"
#define ROB_ATTR_AUTHOR		"author"
#define ROB_ATTR_CAR		"car name"
#define ROB_ATTR_CATEGORY	"category"
#define ROB_ATTR_RACENUM	"race number"
#define ROB_ATTR_RED		"red"
#define ROB_ATTR_GREEN		"green"
#define ROB_ATTR_BLUE		"blue"

#define ROB_ATTR_TYPE		"type"

#define ROB_VAL_HUMAN		"human"
#define ROB_VAL_ROBOT		"robot"

#define ROB_ATTR_LEVEL		"skill level"

#define ROB_VAL_ROOKIE		"rookie"
#define ROB_VAL_AMATEUR		"amateur"
#define ROB_VAL_SEMI_PRO	"semi-pro"
#define ROB_VAL_PRO		"pro"

#endif /* _ROBOTV1_H_ */ 



