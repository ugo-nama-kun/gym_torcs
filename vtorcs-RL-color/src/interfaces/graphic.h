/***************************************************************************

    file                 : graphic.h
    created              : Sun Jan 30 22:58:45 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: graphic.h,v 1.14 2005/06/05 10:47:31 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef _GRAPHV1_H_
#define _GRAPHV1_H_

#include <track.h>
#include <car.h>

#define GRX_IDENT	0

#define GR_PARAM_FILE		"config/graph.xml"

#define GR_SOUND_PARM_CFG			"config/sound.xml"
#define GR_SCT_SOUND				"Sound Settings"
#define GR_ATT_SOUND_STATE			"state"
#define GR_ATT_SOUND_STATE_PLIB     "plib"
#define GR_ATT_SOUND_STATE_OPENAL	"openal"
#define GR_ATT_SOUND_STATE_DISABLED	"disabled"
#define GR_ATT_SOUND_VOLUME	        "volume"

#define GR_SCT_GLFEATURES					"OpenGL Features"
#define GR_ATT_TEXTURECOMPRESSION			"texture compression ARB"
#define GR_ATT_TEXTURECOMPRESSION_ENABLED	"enabled"
#define GR_ATT_TEXTURECOMPRESSION_DISABLED	"disabled"

#define GR_ATT_TEXTURESIZE					"user texture sizelimit"

#define GR_SCT_DISPMODE		"Display Mode"
#define GR_ATT_CAM		"camera"
#define GR_ATT_CAM_HEAD		"camera head list"
#define GR_ATT_MIRROR		"enable mirror"
#define GR_ATT_MAP		"map mode"
#define GR_ATT_FOVY		"fovy"
#define GR_ATT_BOARD		"driver board"
#define GR_ATT_COUNTER		"driver counter"
#define GR_ATT_LEADER		"leader board"
#define GR_ATT_DEBUG		"debug info"
#define GR_ATT_GGRAPH		"G graph"
#define GR_ATT_ARCADE		"arcade"
#define GR_ATT_NBLEADER		"Max leaders entries"

#define GR_SCT_TVDIR		"TV Director View"
#define GR_ATT_CHGCAMINT	"change camera interval"
#define GR_ATT_EVTINT		"event interval"
#define GR_ATT_PROXTHLD		"proximity threshold"

#define GR_SCT_GRAPHIC		"Graphic"
#define GR_ATT_SMOKENB		"smoke value"
#define GR_ATT_SMOKEDELTAT	"smoke interval"
#define GR_ATT_SMOKEDLIFE	"smoke duration"

#define GR_ATT_MAXSTRIPBYWHEEL	"skid value"
#define GR_ATT_MAXPOINTBYSTRIP	"skid length"
#define GR_ATT_SKIDDELTAT	"skid interval"
#define GR_ATT_FOVFACT		"fov factor"
#define GR_ATT_LODFACTOR	"LOD Factor"

#define GR_ATT_NB_SCREENS	"number of screens"
#define GR_ATT_CUR_DRV		"current driver"

#define GR_SCT_PLAYABLE_DOV     "Playable Cameras Distance of Views"
#define GR_ATT_FRONT_GLOBAL     "Front Level Group Global"
#define GR_ATT_FRONT_LEVEL3     "Front Level Group 3"
#define GR_ATT_FRONT_LEVEL2     "Front Level Group 2"
#define GR_ATT_FRONT_LEVEL1     "Front Level Group 1"
#define GR_ATT_REAR_GLOBAL      "Rear Level Group Global"
#define GR_ATT_REAR_LEVEL3      "Rear Level Group 3"
#define GR_ATT_REAR_LEVEL2      "Rear Level Group 2"
#define GR_ATT_REAR_LEVEL1      "Rear Level Group 1"

#define GR_ATT_FRONT_MAP1       "Front Level Map 1" 
#define GR_ATT_FRONT_MAP2       "Front Level Map 2"
#define GR_ATT_FRONT_MAP3       "Front Level Map 3"
#define GR_ATT_REAR_MAP1        "Rear Level Map 1"
#define GR_ATT_REAR_MAP2        "Rear Level Map 2"
#define GR_ATT_REAR_MAP3        "Rear Level Map 3"


/* graphic functions prototypes */

struct Situation;

typedef int (*tfGraphicInitTrack)(tTrack *);
typedef int (*tfGraphicInitCars)(struct Situation *); 
typedef int (*tfGraphicInitView)(int /*x*/, int /*y*/, int /*width*/, int /*height*/, int /*flag*/, void * /*screen*/);
#define GR_VIEW_STD  0 /* full screen view */
#define GR_VIEW_PART 1 /* partial screen view (scissor test) */

typedef int (*tfGraphicRefresh)(struct Situation *);
typedef void (*tfGraphicShutdwnCars)(void);
typedef void (*tfGraphicShutdwnTrack)(void);

class ssgEntity;
typedef void (*tfGraphicBendCar) (int /*index*/, sgVec3 /*poc*/, sgVec3 /*force*/, int /*cnt*/);

/* Interface with the graphic lib */
typedef struct {
    tfGraphicInitTrack	    inittrack;	    /* Graphic init function */
    tfGraphicInitView       initview;       /* Graphic init function */
    tfGraphicInitCars	    initcars;	    /* Graphic init function */
    tfGraphicRefresh	    refresh;	    /* Graphic refresh function */
    tfGraphicShutdwnCars    shutdowncars;   /* Graphic shutdown function */
    tfGraphicShutdwnTrack   shutdowntrack;  /* Graphic shutdown function */
	//tfGraphicBendCar        bendcar;
} tGraphicItf;


    

#endif /* _GRAPHV1_H_ */ 



