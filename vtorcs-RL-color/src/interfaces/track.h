/***************************************************************************

    file                 : track.h
    created              : Sun Jan 30 23:00:06 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: track.h,v 1.22 2006/02/20 20:19:04 berniw Exp $

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
    		This is the track structure.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: track.h,v 1.22 2006/02/20 20:19:04 berniw Exp $
    @ingroup	trackstruct
*/

 
#ifndef _TRACKV1_H_
#define _TRACKV1_H_

#include <tgf.h>
#include <tmath/linalg_t.h>

#define TRK_IDENT	0	/* from 0x01 to 0xFF */

/* Parameters strings for track files */

#define TRK_SECT_HDR	"Header"

#define TRK_ATT_WIDTH	"width"
#define TRK_ATT_SURF	"surface"
#define TRK_ATT_NAME	"name"
#define TRK_ATT_VERSION	"version"
#define TRK_ATT_AUTHOR	"author"
#define TRK_ATT_DESCR	"description"
#define TRK_ATT_CAT     "category"

#define TRK_SECT_GRAPH	"Graphic"

#define TRK_ATT_3DDESC	"3d description"
#define TRK_ATT_BKGRND	"background image"
#define TRK_ATT_BKGRND2	"background image2"
#define TRK_ATT_BGTYPE	"background type"
#define TRK_ATT_BGCLR_R	"background color R"
#define TRK_ATT_BGCLR_G	"background color G"
#define TRK_ATT_BGCLR_B	"background color B"
#define TRK_LST_ENV	"Environment Mapping"
#define TRK_ATT_ENVNAME	"env map image"
#define TRK_ATT_SPEC_R	"specular color R"
#define TRK_ATT_SPEC_G	"specular color G"
#define TRK_ATT_SPEC_B	"specular color B"
#define TRK_ATT_AMBIENT_R	"ambient color R"
#define TRK_ATT_AMBIENT_G	"ambient color G"
#define TRK_ATT_AMBIENT_B	"ambient color B"
#define TRK_ATT_DIFFUSE_R	"diffuse color R"
#define TRK_ATT_DIFFUSE_G	"diffuse color G"
#define TRK_ATT_DIFFUSE_B	"diffuse color B"
#define TRK_ATT_SHIN	"shininess"
#define TRK_ATT_LIPOS_X	"light position x"
#define TRK_ATT_LIPOS_Y	"light position y"
#define TRK_ATT_LIPOS_Z	"light position z"
#define TRK_ATT_FOVFACT	"fov factor"

#define TRK_SECT_TURNMARKS "Graphic/Turn Marks"
#define TRK_ATT_HSPACE	"horizontal space"
#define TRK_ATT_VSPACE	"vertical space"

#define TRK_SECT_TERRAIN  "Graphic/Terrain Generation"

#define TRK_ATT_TSTEP	"track step"
#define TRK_ATT_BMARGIN	"border margin"
#define TRK_ATT_BSTEP	"border step"
#define TRK_ATT_ORIENT	"orientation"
#define TRK_VAL_CLOCK	"clockwise"
#define TRK_VAL_CCLOCK	"counter-clockwise"
#define TRK_ATT_SURFRAND	"texture randomness"
#define TRK_ATT_RELIEF	"relief file"
#define TRK_ATT_ELEVATION	"elevation map"
#define TRK_ATT_ALT_MAX	"maximum altitude"
#define TRK_ATT_ALT_MIN	"minimum altitude"
#define TRK_ATT_GRPSZ   "group size"
#define TRK_ATT_OBJMAP	"object map"
#define TRK_ATT_COLOR	"color"
#define TRK_ATT_OBJECT	"object"
#define TRK_ATT_SEED	"random seed"
#define TRK_ATT_ORIENTATION_TYPE "orientation type"
#define TRK_ATT_ORIENTATION "orientation"
#define TRK_ATT_DH	"delta height"
#define TRK_ATT_DV	"delta vert"
#define TRK_SECT_OBJMAP	"Object Maps"
#define TRK_SECT_SURFACES "Surfaces"
#define TRK_SECT_OBJECTS  "Objects"

#define TRK_ATT_FRICTION "friction"
#define TRK_ATT_ROLLRES  "rolling resistance"
#define TRK_ATT_ROUGHT   "roughness"
#define TRK_ATT_ROUGHTWL "roughness wavelength"
#define TRK_ATT_DAMMAGE  "dammage"
#define TRK_ATT_REBOUND  "rebound"
#define TRK_ATT_TEXTURE  "texture name"
#define TRK_ATT_BUMPNAME "bump name"
#define TRK_ATT_TEXTYPE  "texture type"
#define TRK_ATT_TEXLINK  "texture link with previous"
#define TRK_ATT_TEXSIZE  "texture size"
#define TRK_ATT_BUMPSIZE "bump size"
#define TRK_ATT_TEXSTARTBOUNDARY "texture start on boundary"
#define TRK_ATT_TEXMIPMAP "texture mipmap"
#define TRK_ATT_R1	 "color R1"
#define TRK_ATT_G1	 "color G1"
#define TRK_ATT_B1	 "color B1"
#define TRK_ATT_R2	 "color R2"
#define TRK_ATT_G2	 "color G2"
#define TRK_ATT_B2	 "color B2"

#define TRK_ATT_STEPLEN	"steps length"

#define TRK_VAL_YES	"yes"
#define TRK_VAL_NO	"no"

#define TRK_SECT_MAIN	"Main Track"
#define TRK_LST_SEGMENTS	"Track Segments"
#define TRK_ATT_TYPE		"type"

#define TRK_VAL_STR		"str"
#define TRK_VAL_LFT		"lft"
#define TRK_VAL_RGT		"rgt"

#define TRK_ATT_LG		"lg"
#define TRK_ATT_RADIUS		"radius"
#define TRK_ATT_RADIUSEND	"end radius"
#define TRK_ATT_ARC		"arc"
#define TRK_ATT_ZS		"z start"
#define TRK_ATT_ZE		"z end"
#define TRK_ATT_ZSL		"z start left"
#define TRK_ATT_ZSR		"z start right"
#define TRK_ATT_ZEL		"z end left"
#define TRK_ATT_ZER		"z end right"
#define TRK_ATT_GRADE		"grade"
#define TRK_ATT_BKS		"banking start"
#define TRK_ATT_BKE		"banking end"
#define TRK_ATT_ID		"id"
#define TRK_ATT_PROFIL		"profil"
#define TRK_ATT_PROFSTEPS	"profil steps"
#define TRK_ATT_PROFSTEPSLEN	"profil steps length"
#define TRK_ATT_PROFTGTS	"profil start tangent"
#define TRK_ATT_PROFTGTE	"profil end tangent"
#define TRK_ATT_PROFTGTSL	"profil start tangent left"
#define TRK_ATT_PROFTGTEL	"profil end tangent left"
#define TRK_ATT_PROFTGTSR	"profil start tangent right"
#define TRK_ATT_PROFTGTER	"profil end tangent right"
#define TRK_ATT_MARKS		"marks"
#define TRK_ATT_ENVMAP		"env map"
#define TRK_ATT_DOVFACTOR	"DoV factor"

#define TRK_SECT_PITS		"Pits"

#define TRK_ATT_ENTRY		"entry"
#define TRK_ATT_EXIT		"exit"
#define TRK_ATT_START		"start"
#define TRK_ATT_END		"end"
#define TRK_ATT_SIDE		"side"
#define TRK_ATT_LEN		"length"
#define TRK_ATT_SPD_LIM		"speed limit"

#define TRK_SECT_LSIDE		"Left Side"
#define TRK_SECT_RSIDE		"Right Side"
#define TRK_SECT_LBORDER	"Left Border"
#define TRK_SECT_RBORDER	"Right Border"
#define TRK_SECT_LBARRIER	"Left Barrier"
#define TRK_SECT_RBARRIER	"Right Barrier"

#define TRK_ATT_SWIDTH		"start width"
#define TRK_ATT_EWIDTH		"end width"
#define TRK_ATT_STYLE		"style"
#define TRK_ATT_HEIGHT		"height"
#define TRK_ATT_BANKTYPE	"banking type"

#define TRK_VAL_PLAN		"plan"
#define TRK_VAL_WALL		"wall"
#define TRK_VAL_CURB		"curb"
#define TRK_VAL_FENCE		"fence"


#define TRK_SECT_CAM		"Cameras"

#define TRK_ATT_SEGMENT		"segment"
#define TRK_ATT_TORIGHT 	"to right"
#define TRK_ATT_TOSTART 	"to start"
#define TRK_ATT_HEIGHT  	"height"
#define TRK_ATT_CAM_FOV 	"fov start"
#define TRK_ATT_CAM_FOVE 	"fov end"

#define TRK_VAL_ASPHALT		"asphalt"
#define TRK_VAL_CONCRETE	"concrete"
#define TRK_VAL_GRASS		"grass"
#define TRK_VAL_SAND		"sand"
#define TRK_VAL_DIRT		"dirt"
#define TRK_VAL_BARRIER		"barrier"

#define TRK_VAL_LINEAR		"linear"
#define TRK_VAL_SPLINE		"spline"

#define TRK_VAL_TANGENT		"tangent"
#define TRK_VAL_LEVEL		"level"

/** road camera */
typedef struct RoadCam
{
    char		*name;
    t3Dd		pos;
    struct RoadCam	*next;
} tRoadCam;

/** Extended track segment */
typedef struct SegExt
{
    int		nbMarks;	/**< turn marks */
    int		*marks;		/**< marks array */
} tSegExt;


/** Surface */
typedef struct trackSurface {
    struct trackSurface *next;	/**< Next surface in list */

    char *material;		/**< Type of material used */

    tdble kFriction;		/**< Coefficient of friction */
    tdble kRebound;		/**< Coefficient of energy restitution */
    tdble kRollRes;		/**< Rolling resistance */
    tdble kRoughness;		/**< Roughtness in m of the surface (wave height) */
    tdble kRoughWaveLen;	/**< Wave length in m of the surface */
    tdble kDammage;		/**< Dammages in case of collision */

} tTrackSurface;



/** Barrier */
typedef struct trackBarrier {
    int			style;	/**< Barrier style */
    tdble		width;	/**< Barrier width */
    tdble		height;	/**< Barrier height */
    tTrackSurface	*surface; /**< Barrier surface */
	vec2f normal;	// Normal on the vertical track inside pointing towards the track middle.
} tTrackBarrier;


/** Track segment (tTrackSeg)
    The segments can be straights (type TR_STR): (the track goes from the right to the left)
    @image html straight-desc.png
    Or can be turn segments (type TR_RGT or TR_LFT): (the track goes from the left to the right)
    @image html turn-1-desc.png
    The reference angle is the orientation of the first segment of the track.
    @ingroup trackstruct
*/
typedef struct trackSeg {
    char *name;			/**< Segment name */
    int	id;			/**< Segment number */

    int type;			/**< Geometrical type:
				 - TR_RGT
				 - TR_LFT
				 - TR_STR
				*/
#define TR_RGT	    1		/**< Right curve */
#define TR_LFT	    2		/**< Left curve */
#define TR_STR	    3		/**< Straight */

    int type2;			/**< Position type:
				   - TR_MAIN
				   - TR_LSIDE
				   - TR_RSIDE
				   - TR_LBORDER
				   - TR_RBORDER
				*/
#define TR_MAIN	    1		/**< Main track segment (ie road part) */
#define TR_LSIDE    2		/**< Left side segment (outer segment) */
#define TR_RSIDE    3		/**< Right side segment (outer segment) */
#define TR_LBORDER  4		/**< Left border segment (inner segment) */
#define TR_RBORDER  5		/**< Right border segment (inner segment) */

    int style;			/**< Border and barrier segments style:
				   - TR_PLAN
				   - TR_CURB
				   - TR_WALL
				   - TR_FENCE
				   - TR_PITBUILDING
				 */
#define TR_PLAN		0	/**< Flat (border only) */
#define TR_CURB		1	/**< Curb (border only) */
#define TR_WALL		2	/**< Wall (barrier only) */
#define TR_FENCE	3	/**< Fence (no width) (barrier only) */
#define TR_PITBUILDING	4	/**< Pit building wall (barrier only) */

    tdble length;		/**< Length in meters of the middle of the track */
    tdble width;		/**< Width of the segment (if constant width) */
    tdble startWidth;		/**< Width of the beginning of the segment */
    tdble endWidth;		/**< Width of the end of the segment */
    tdble lgfromstart;		/**< Length of begining of segment from starting line */
    tdble radius;		/**< Radius in meters of the middle of the track (>0) */
    tdble radiusr;		/**< Radius in meters of the right side of the track (>0) */
    tdble radiusl;		/**< Radius in meters of the left side of the track (>0) */
    tdble arc;			/**< Arc in rad of the curve (>0) */
    t3Dd center;		/**< Center of the curve */
    t3Dd vertex[4];		/**< Coord of the 4 corners of the segment.
				   <br>Index in:
				   - TR_SL
				   - TR_SL
				   - TR_EL
				   - TR_ER
				 */
#define TR_SL	0		/**< Start-Left corner */
#define TR_SR	1		/**< Start-Right corner */
#define TR_EL	2		/**< End-Left corner */
#define TR_ER	3		/**< End_Right corner */

    tdble angle[7];		/** Rotation angles of the track in rad anti-clockwise:
				    <br>Index in:
				   - TR_ZS
				   - TR_ZE
				   - TR_YL
				   - TR_YR
				   - TR_XS
				   - TR_XE
				   - TR_CS
				 */
#define TR_ZS	0		/**< Z Start angle */
#define TR_ZE	1		/**< Z End angle */
#define TR_YL	2		/**< Y Left angle */
#define TR_YR	3		/**< Y Right angle */
#define TR_XS	4		/**< X Start angle */
#define TR_XE	5		/**< X End angle */
#define TR_CS   6		/**< Center start angle */

    /* constants used to find the height of a point */
    tdble Kzl;		/* long constant */
    tdble Kzw;		/* width constant */
    /* constant used to find the width of a segment */
    tdble	Kyl;		/* find y along x */
    t3Dd	rgtSideNormal;	/* normal to the right side in case of straight segment */
    int		envIndex;	/* Environment mapping image index */

    tdble	height;		/**< Max height for curbs */

    unsigned int raceInfo;	/**< Type of segment regarding the race:
				   <br>Mask value in:
				   - TR_NORMAL
				   - TR_LAST
				   - TR_START
				   - TR_PITLANE
				   - TR_SPEEDLIMIT
				   - TR_PITENTRY
				   - TR_PITEXIT
				   - TR_PIT
				*/
#define TR_NORMAL	0x00000000 /**< Normal segment */
#define TR_LAST		0x00000001 /**< Segment before start line */
#define TR_START	0x00000002 /**< Segment after start line */
#define TR_PITLANE	0x00000004 /**< Pit lane segment */
#define TR_SPEEDLIMIT	0x00000008 /**< Segment where the speed is limited */
#define TR_PITENTRY	0x00000010 /**< Segment where the pit lane cross the main track */
#define TR_PITEXIT	0x00000020 /**< Segment where the pit lane cross the main track */
#define TR_PIT		0x00000040 /**< Car pit */
#define TR_PITSTART	0x00000080 /**< Car pit Star */
#define TR_PITEND	0x00000100 /**< Car pit End */

    tdble DoVfactor;            /* the factor to use in calculating DoV for this Seg */

    /* pointers */

    /* optionnal extensions */
    tSegExt		*ext;

    tTrackSurface	*surface; /**< Segment surface */
	tTrackBarrier	*barrier[2]; /**< Segment barriers */
    tRoadCam        *cam;	/* current camera */
    struct trackSeg *next;	/**< Next segment */
    struct trackSeg *prev;	/**< Previous segment */

	// Union to avoid code duplication for left/right side cases and to
	// keep compatibility of code. The side definition is so ugly to
	// match the one of the barrier[].
#define TR_SIDE_LFT 1
#define TR_SIDE_RGT 0
	union {
		struct { struct trackSeg *rside, *lside; };
		struct trackSeg* side[2];
	};

} tTrackSeg;

/* selection for local position structure */
#define TR_TORIGHT	0
#define TR_TOMIDDLE	1
#define TR_TOLEFT	2

/** Location on the track in local coordinates */
typedef struct
{
    tTrackSeg	*seg;		/**< Track segment */
    int		type;		/**< Type of description:
				   - TR_LPOS_MAIN
				   - TR_LPOS_SEGMENT
				   - TR_LPOS_TRACK
				 */

#define TR_LPOS_MAIN	0	/**< Relative to the main segment */
#define TR_LPOS_SEGMENT	1	/**< If the point is on a side, relative to this side */
#define TR_LPOS_TRACK	2	/**< Local pos includes all the track width */

    tdble	toStart;	/**< Distance to start of segment (or arc if turn) */
    tdble	toRight;	/**< Distance to right side of segment (+ to inside of track - to outside) */
    tdble	toMiddle;	/**< Distance to middle of segment (+ to left - to right) */
    tdble	toLeft;		/**< Distance to left side of segment (+ to inside of track - to outside) */
} tTrkLocPos;

struct CarElt;

/** Driver's pit */
typedef struct TrackOwnPit
{
	tTrkLocPos pos;	/**< Center of the pit position */
	int pitCarIndex;	/**< Index of the car in the car array below which occupies the pit. If the pit is free the value is TR_PIT_STATE_FREE */
#define TR_PIT_STATE_FREE	-1
#define TR_PIT_MAXCARPERPIT 4		// maximum cars per pit
	tdble lmin;	/**< Pitting area length min */
	tdble lmax;	/**< Pitting area length max */
	int freeCarIndex;	// Index of next free car entry (look at the next line).
	struct CarElt	*car[TR_PIT_MAXCARPERPIT];	/**< Car links for pit */
} tTrackOwnPit;

/** Pits Info Structure */
typedef struct 
{
	int type;		/**< Type of Pit:
				   - TR_PIT_NONE
				   - TR_PIT_ON_TRACK_SIDE
				 */
#define TR_PIT_NONE		0 /**< No pits for that tracks */
#define TR_PIT_ON_TRACK_SIDE	1 /**< The pits are on the track side */
#define TR_PIT_ON_SEPARATE_PATH 2
	int nMaxPits;	/**< number max of pits */
	int nPitSeg;	/**< actual number of pits */
	int side;		/**< Pits side:
				   - TR_RGT
				   - TR_LFT
				*/
	tdble len;					/**< Lenght of each pit stop */
	tdble width;				/**< Width of each pit stop */
	tdble speedLimit;			/**< Speed limit between pitStart and pitEnd */
	tTrackSeg *pitEntry;		/**< Pit lane segment */
	tTrackSeg *pitStart;		/**< Pit lane segment */
	tTrackSeg *pitEnd;			/**< Pit lane segment */
	tTrackSeg *pitExit;			/**< Pit lane segment */
	tTrackOwnPit *driversPits;	/**< List of pits by driver */
	int carsPerPit;
	int driversPitsNb;			/**< Number of drivers */
} tTrackPitInfo;

typedef struct
{
    tdble	height;
    tdble	width;
    tdble	hSpace;
    tdble	vSpace;
} tTurnMarksInfo;

typedef struct 
{
    char		*background;
    char		*background2;
    int			bgtype;
    float		bgColor[3];
    int			envnb;
    char		**env;
    tTurnMarksInfo	turnMarksInfo;
} tTrackGraphicInfo;

/** Track structure
    @ingroup trackstruct
*/
typedef struct
{
    char	  *name;	/**< Name of the track */
    char	  *author;	/**< Author's name */
    char	  *filename;	/**< Filename of the track description */
    void	  *params;	/**< Parameters handle */
    char	  *internalname; /**< Internal name of the track */
    char	  *category;	/**< Category of the track */
    int		  nseg;		/**< Number of segments */
    int		  version;	/**< Version of the track type */
    tdble	  length;	/**< main track length */
    tdble	  width;	/**< main track width */
    tTrackPitInfo pits;		/**< Pits information */
    tTrackSeg	  *seg;		/**< Main track */
    tTrackSurface *surfaces;	/**< Segment surface list */

    t3Dd		min;
    t3Dd		max;
    tTrackGraphicInfo	graphic;
} tTrack;



typedef tTrack*(*tfTrackBuild)(char*);
typedef tdble(*tfTrackHeightG)(tTrackSeg*, tdble, tdble);
typedef tdble(*tfTrackHeightL)(tTrkLocPos*);
typedef void(*tfTrackGlobal2Local)(tTrackSeg* /*seg*/, tdble /*X*/, tdble /*Y*/, tTrkLocPos* /*pos*/, int /*sides*/);
typedef void(*tfTrackLocal2Global)(tTrkLocPos*, tdble *, tdble *);
typedef void(*tfTrackSideNormal)(tTrackSeg*, tdble, tdble, int, t3Dd*);
typedef void(*tfTrackSurfaceNormal)(tTrkLocPos *, t3Dd*);
typedef void(*tfTrackShutdown)(void);

typedef struct {
    tfTrackBuild		trkBuild;		/* build track structure for simu */
    tfTrackBuild		trkBuildEx;		/* build with graphic extensions  */
    tfTrackHeightG		trkHeightG;
    tfTrackHeightL		trkHeightL;
    tfTrackGlobal2Local		trkGlobal2Local;
    tfTrackLocal2Global		trkLocal2Global;
    tfTrackSideNormal   	trkSideNormal;
    tfTrackSurfaceNormal	trkSurfaceNormal;
    tfTrackShutdown		trkShutdown;
} tTrackItf;


/* For Type 3 tracks (now obsolete) */

#define TRK_LST_SURF			"List"
#define TRK_LST_CAM			"list"
#define TRK_ATT_PIT_ENTRY		"pit entry"
#define TRK_ATT_PIT_EXIT		"pit exit"
#define TRK_ATT_PIT_START		"pit start"
#define TRK_ATT_PIT_END			"pit end"
#define TRK_ATT_PIT_TYPE		"pit type"
#define TRK_ATT_PIT_SIDE		"pit side"
#define TRK_ATT_PIT_LEN			"pit length"
#define TRK_ATT_PIT_WIDTH		"pit width"

#define TRK_LST_SEG	"segments"
#define TRK_ATT_ENVIND	"env map index"


#define TRK_ATT_LST	"lside type"
#define TRK_ATT_LSW	"lside width"
#define TRK_ATT_LSWS	"lside start width"
#define TRK_ATT_LSWE	"lside end width"
#define TRK_ATT_LSSURF	"lside surface"
#define TRK_ATT_RST	"rside type"
#define TRK_ATT_RSW	"rside width"
#define TRK_ATT_RSWS	"rside start width"
#define TRK_ATT_RSWE	"rside end width"
#define TRK_ATT_RSSURF	"rside surface"
#define TRK_ATT_LBS	"lborder style"
#define TRK_ATT_LBW	"lborder width"
#define TRK_ATT_LBH	"lborder height"
#define TRK_ATT_LBSURF	"lborder surface"
#define TRK_ATT_RBS	"rborder style"
#define TRK_ATT_RBW	"rborder width"
#define TRK_ATT_RBH	"rborder height"
#define TRK_ATT_RBSURF	"rborder surface"

#define TRK_ATT_BHEIGHT	"border height"

#define TRK_ATT_LBAS	"lbarrier style"
#define TRK_ATT_LBAW	"lbarrier width"
#define TRK_ATT_LBAH	"lbarrier height"
#define TRK_ATT_LBAFRCT	"lbarrier friction"
#define TRK_ATT_LBASURF	"lbarrier surface"
#define TRK_ATT_RBAS	"rbarrier style"
#define TRK_ATT_RBAW	"rbarrier width"
#define TRK_ATT_RBAH	"rbarrier height"
#define TRK_ATT_LBAFRCT	"lbarrier friction"
#define TRK_ATT_RBASURF	"rbarrier surface"

#define TRK_VAL_PIT_TYPE_NONE		"no pit"
#define TRK_VAL_PIT_TYPE_SIDE		"track side"
#define TRK_VAL_PIT_TYPE_SEP_PATH	"seperate path"

#define TRK_ATT_FINISH	"finish segment"

#endif /* _TRACKV1_H_ */ 



