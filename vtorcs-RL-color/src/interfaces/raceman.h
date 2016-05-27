/***************************************************************************

    file                 : raceman.h
    created              : Sun Jan 30 22:59:17 CET 2000
    copyright            : (C) 2000,2002 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: raceman.h,v 1.28 2006/02/20 20:17:43 berniw Exp $

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
    		This is the race information structures.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: raceman.h,v 1.28 2006/02/20 20:17:43 berniw Exp $
    @ingroup	raceinfo
*/

#ifndef _RACEMANV1_H_
#define _RACEMANV1_H_

#include <tgf.h>
#include <car.h>
#include <track.h>
#include <graphic.h>
#include <simu.h>

#define RCM_IDENT 0

struct RmInfo;

typedef int (*tfRmRunState) (struct RmInfo *);

#define RM_SYNC			0x00000001
#define RM_ASYNC		0x00000002

#define RM_END_RACE		0x00000010
#define RM_CONTINUE_RACE	0x00000020

#define RM_NEXT_STEP		0x00000100
#define RM_NEXT_RACE		0x00000200
#define RM_NEXT_EVENT		0x00000400

#define RM_ACTIVGAMESCR		0x01000000
#define RM_QUIT			0x40000000


#define RCM_MAX_DT_SIMU		0.002
#define RCM_MAX_DT_ROBOTS	0.2

/** General info on current race */
typedef struct {
    int			ncars;		/**< number of cars */
    int			totLaps;	/**< total laps */
    int			state;
#define RM_RACE_RUNNING		0X00000001
#define RM_RACE_FINISHING	0X00000002
#define RM_RACE_ENDED		0X00000004
#define RM_RACE_STARTING	0X00000008
#define RM_RACE_PRESTART	0X00000010
#define RM_RACE_PAUSED		0X40000000
    int			type;		/**< Race type */
#define RM_TYPE_PRACTICE	0 /* Please keep the order */
#define RM_TYPE_QUALIF		1
#define RM_TYPE_RACE		2
    int                 maxDammage;
    unsigned long	fps;
} tRaceAdmInfo;

#define _ncars		raceInfo.ncars
#define _totLaps	raceInfo.totLaps
#define _raceState	raceInfo.state
#define _raceType	raceInfo.type
#define _maxDammage	raceInfo.maxDammage

/** cars situation used to inform the GUI and the drivers */
typedef struct Situation {
    tRaceAdmInfo	raceInfo;
    double		deltaTime;
    double		currentTime;	/**< current time in sec since the beginning of the simulation */
    int			nbPlayers;	/**< number of human player in local (splitted screen) */
    tCarElt		**cars;		/**< list of cars */
} tSituation;

/** Race Engine */
typedef struct
{
    tTrackItf	trackItf;
    tGraphicItf	graphicItf;
    tSimItf	simItf;
} tRaceModIft;

#define RE_STATE_CONFIG			0
#define RE_STATE_EVENT_INIT		1
#define RE_STATE_PRE_RACE		3
#define RE_STATE_RACE_START		5
#define RE_STATE_RACE			6
#define RE_STATE_RACE_STOP		7
#define RE_STATE_RACE_END		8
#define RE_STATE_POST_RACE		9
#define RE_STATE_EVENT_SHUTDOWN		11
#define RE_STATE_SHUTDOWN		12
#define RE_STATE_ERROR			13
#define RE_STATE_EXIT			14

/** Race Engine Car Information about the race */
typedef struct
{
    tTrkLocPos	prevTrkPos;
    tdble	sTime;
    int		lapFlag;
    char	*raceMsg;
    double	totalPitTime;
    double	startPitTime;
    tdble	topSpd;
    tdble	botSpd;
    tdble	fuel;
} tReCarInfo;

/** Race Engine Information.
   @image	html raceenginestate.gif
 */
typedef struct
{
    int			state;
    void		*param;
    tRaceModIft		itf;
    void		*gameScreen;
    void		*menuScreen;
    char		*filename;
    char		*name;
    char		*raceName;
    tReCarInfo		*carInfo;
    double		curTime;
    double		lastTime;
    double		timeMult;
    int			running;
#define RM_DISP_MODE_NORMAL	0
#define RM_DISP_MODE_CAPTURE	1
#define RM_DISP_MODE_NONE	2
    int			displayMode;
    int			refreshDisplay;
} tRaceEngineInfo;

#define _reState	raceEngineInfo.state
#define _reParam	raceEngineInfo.param
#define _reRacemanItf	raceEngineInfo.itf.racemanItf
#define _reTrackItf	raceEngineInfo.itf.trackItf
#define _reGraphicItf	raceEngineInfo.itf.graphicItf
#define _reSimItf	raceEngineInfo.itf.simItf
#define _reGameScreen	raceEngineInfo.gameScreen
#define _reMenuScreen	raceEngineInfo.menuScreen
#define _reFilename	raceEngineInfo.filename
#define _reName		raceEngineInfo.name
#define _reRaceName	raceEngineInfo.raceName
#define _reCarInfo	raceEngineInfo.carInfo
#define _reCurTime	raceEngineInfo.curTime
#define _reTimeMult	raceEngineInfo.timeMult
#define _reRunning	raceEngineInfo.running
#define _reLastTime	raceEngineInfo.lastTime
#define _displayMode	raceEngineInfo.displayMode
#define _refreshDisplay	raceEngineInfo.refreshDisplay

#define RM_PNST_DRIVETHROUGH	0x00000001
#define RM_PNST_STOPANDGO	0x00000002
#define RM_PNST_STOPANDGO_OK	0x00000004
#define RM_PNST_SPD		0x00010000
#define RM_PNST_STNGO		0x00020000

typedef struct RmCarRules
{
    int			ruleState;
} tRmCarRules;


// GIUSE - MOVED TO CAR.H FOR VISUAL PURPOSES
// IT'S STILL INCLUDED IN THE HEAD OF THIS FILE SO NO PROBLEM
//typedef struct RmMovieCapture
//{
//    int		enabled;
//    int		state;
//    double	deltaSimu;
//    double	deltaFrame;
//    double	lastFrame;
//    char	*outputBase;
//    int		currentCapture;
//    int		currentFrame;
//} tRmMovieCapture;

/**
 * Race Manager General Info
 */
typedef struct RmInfo
{
    tCarElt		*carList;	/**< List of all the cars racing */
    tSituation		*s;		/**< Situation during race */
    tTrack		*track;		/**< Current track */
    void		*params;	/**< Raceman parameters */
    void		*results;	/**< Race results */
    tModList		**modList;	/**< drivers loaded */
    tRmCarRules		*rules;		/**< by car rules */
    tRaceEngineInfo	raceEngineInfo;
    tRmMovieCapture	movieCapture;
    // GIUSE - VISION HERE!
    tRmVisionImg* vision;
} tRmInfo;

/*
 * Parameters name definitions for Race Managers
 */
#define RM_SECT_HEADER		"Header"
#define RM_SECT_DRIVERS		"Drivers"
#define RM_SECT_STARTINGGRID	"Starting Grid"
#define RM_SECT_RACES		"Races"
#define RM_SECT_TRACKS		"Tracks"
#define RM_SECT_CONF		"Configuration"
#define RM_SECT_OPTIONS		"Options"
#define RM_SECT_POINTS		"Points"

#define RM_SECT_DRIVERS_RACING	"Drivers Start List"

#define RM_ATTR_CUR_CONF	"current configuration"
#define RM_ATTR_START_ORDER	"starting order"
#define RM_ATTR_ALLOW_RESTART	"restart"
#define RM_ATTR_MUST_COMPLETE	"must complete"
#define RM_ATTR_SPLASH_MENU	"splash menu"
#define RM_ATTR_DISP_START_GRID	"display starting grid"

#define RM_ATTR_MAXNUM		"maximum number"
#define RM_ATTR_MAX_DRV		"maximum drivers"
#define RM_ATTR_CAR		"car"

#define RM_ATTR_PRIO		"priority"
#define RM_ATTR_NAME		"name"
#define RM_ATTR_DRVNAME		"driver name"
#define RM_ATTR_CATEGORY	"category"
#define RM_ATTR_DESCR		"description"
#define RM_ATTR_BGIMG		"menu image"
#define RM_ATTR_RUNIMG		"run image"
#define RM_ATTR_STARTIMG	"start image"

#define RM_ATTR_MODULE		"module"
#define RM_ATTR_IDX		"idx"
#define RM_ATTR_FOCUSED		"focused module"
#define RM_ATTR_FOCUSEDIDX	"focused idx"
#define RM_ATTR_DISPMODE	"display mode"
#define RM_ATTR_DISPRES		"display results"

#define RM_ATTR_TIMESTEP	"time step"

#define RM_ATTR_TYPE		"type"
#define RM_ATTR_RACE		"race"
#define RM_ATTR_ROWS		"rows"
#define RM_ATTR_TOSTART		"distance to start"
#define RM_ATTR_COLDIST		"distance between columns"
#define RM_ATTR_COLOFFSET	"offset within a column"
#define RM_ATTR_INITSPEED	"initial speed"
#define RM_ATTR_INITHEIGHT	"initial height"
#define RM_ATTR_SHOW_RACE	"show race"
#define RM_ATTR_MAX_DMG		"maximum dammage"
#define RM_ATTR_DISTANCE	"distance"
#define RM_ATTR_LAPS		"laps"
#define RM_ATTR_QUAL_LAPS	"Qualification laps"
#define RM_ATTR_POLE		"pole position side"
#define RM_ATTR_CARSPERPIT	"cars per pit"

#define RM_ATTR_POINTS		"points"

#define RM_VAL_TRACKSEL		"track select"
#define RM_VAL_DRVSEL		"drivers select"
#define RM_VAL_RACECONF		"race config"
#define RM_VAL_CONFRACELEN	"race length"
#define RM_VAL_CONFDISPMODE	"display mode"

#define RM_VAL_DRV_LIST_ORDER	"drivers list"
#define RM_VAL_LAST_RACE_ORDER	"last race"
#define RM_VAL_LAST_RACE_RORDER	"last race reversed"

#define RM_VAL_RACE		"race"
#define RM_VAL_QUALIF		"qualifications"
#define RM_VAL_PRACTICE		"practice"

#define RM_VAL_YES		"yes"
#define RM_VAL_NO		"no"

#define RM_VAL_VISIBLE		"normal"
#define RM_VAL_INVISIBLE	"results only"


#define RM_SECT_MOVIE_CAPTURE	"Movie Capture"

#define RM_ATT_CAPTURE_ENABLE	"enable capture"
#define RM_ATT_CAPTURE_FPS	"fps"
#define RM_ATT_CAPTURE_OUT_DIR	"output directory"

/* RESULTS */

#define RE_SECT_HEADER		"Header"
#define RE_ATTR_DATE		"date"
#define RE_ATTR_TYPE		"race"

#define RE_SECT_CURRENT		"Current"
#define RE_ATTR_CUR_RACE	"current race"
#define RE_ATTR_CUR_TRACK	"current track"
#define RE_ATTR_CUR_DRIVER	"current driver"

#define RE_SECT_DRIVERS		"Drivers"
#define RE_SECT_DRIVER		"Driver"
#define RE_ATTR_DLL_NAME	"dll name"
#define RE_ATTR_INDEX		"index"

#define RE_SECT_STANDINGS	"Standings"

#define RE_SECT_RESULTS		"Results"
#define RE_SECT_STARTINGGRID	"Starting Grid"

#define RE_SECT_QUALIF		"Qualifications"

#define RE_SECT_FINAL		"Final"

#define RE_SECT_RANK		"Rank"

#define RE_ATTR_NAME		"name"
#define RE_ATTR_CAR		"car"
#define RE_ATTR_MODULE		"module"
#define RE_ATTR_IDX		"idx"
#define RE_ATTR_LAPS		"laps"
#define RE_ATTR_BEST_LAP_TIME	"best lap time"
#define RE_ATTR_TIME		"time"
#define RE_ATTR_TOP_SPEED	"top speed"
#define RE_ATTR_BOT_SPEED	"bottom speed"
#define RE_ATTR_DAMMAGES	"dammages"
#define RE_ATTR_NB_PIT_STOPS	"pits stops"
#define RE_ATTR_POINTS		"points"

#endif /* _RACEMANV1_H_ */
