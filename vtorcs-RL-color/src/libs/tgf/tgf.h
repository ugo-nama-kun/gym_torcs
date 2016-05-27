/***************************************************************************
                    tgf.h -- Interface file for The Gaming Framework                                    
                             -------------------                                         
    created              : Fri Aug 13 22:32:14 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: tgf.h,v 1.41.2.2 2008/11/09 17:50:22 berniw Exp $                                  
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
    	The Gaming Framework API.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: tgf.h,v 1.41.2.2 2008/11/09 17:50:22 berniw Exp $
*/



#ifndef __TGF__H__
#define __TGF__H__

#include <stdio.h>
#ifndef WIN32
#include <sys/param.h>
#include <assert.h>
#endif /* WIN32 */
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <stdarg.h>
#include <cstring>
#include <math.h>
#include <osspec.h>


/* typedef double tdble; */
/** Floating point type used in TORCS.
    @ingroup definitions
*/
typedef float tdble;

extern void setTextOnly(bool value);
extern bool getTextOnly ();

extern void GfInit(void);

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#define FREEZ(x) do {				\
    if (x) {					\
	free(x);				\
	x = 0;					\
    }						\
} while (0)

#define freez FREEZ

const double PI = 3.14159265358979323846;  /**< PI */
const tdble G = 9.80665f; /**< m/s/s */

/* conversion */
#define RADS2RPM(x) ((x)*9.549296585)		/**< Radian/s to RPM conversion */
#define RPM2RADS(x) ((x)*.104719755)		/**< RPM to Radian/s conversion */
#define RAD2DEG(x)  ((x)*(180.0/PI))		/**< Radian to degree conversion */
#define DEG2RAD(x)  ((x)*(PI/180.0))		/**< Degree to radian conversion */
#define FEET2M(x)   ((x)*0.304801)		/**< Feet to meter conversion */
#define SIGN(x)     ((x) < 0 ? -1.0 : 1.0)	/**< Sign of the expression */

/** Angle normalization between 0 and 2 * PI */
#define NORM0_2PI(x) 				\
do {						\
	while ((x) > 2*PI) { (x) -= 2*PI; }	\
	while ((x) < 0) { (x) += 2*PI; } 	\
} while (0)

/** Angle normalization between -PI and PI */
#define NORM_PI_PI(x) 				\
do {						\
	while ((x) > PI) { (x) -= 2*PI; }	\
	while ((x) < -PI) { (x) += 2*PI; } 	\
} while (0)


#ifndef DIST
/** Distance between two points */
#define DIST(x1, y1, x2, y2) sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))
#endif

#ifndef MIN
/** Minimum between two values */
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif


typedef struct {
    float	x;
    float	y;
    float	z;
} t3Df;

/** 3D point.
    @ingroup definitions
*/
typedef struct {
    tdble	x;		/**< x coordinate */
    tdble	y;		/**< y coordinate */
    tdble	z;		/**< z coordinate */
} t3Dd;

typedef struct {
    int		x;
    int		y;
    int		z;
} t3Di;

/** 6 DOF position.
    @ingroup definitions
*/
typedef struct {
    tdble	x;		/**< x coordinate */
    tdble	y;		/**< y coordinate */
    tdble	z;		/**< z coordinate */
    tdble	ax;		/**< angle along x axis */
    tdble	ay;		/**< angle along y axis */
    tdble	az;		/**< angle along z axis */
} tPosd;

/** Dynamic point structure.
    @ingroup definitions
*/
typedef struct 
{
    tPosd pos; /**< position */
    tPosd vel; /**< velocity */
    tPosd acc; /**< acceleration */
} tDynPt;

/** Forces and moments */
typedef struct
{
    t3Dd F; /**< Forces */
    t3Dd M; /**< Moments */
} tForces;


// <esppat>
#ifdef WIN32
#define malloc _tgf_win_malloc
#define calloc _tgf_win_calloc
#define realloc _tgf_win_realloc
#define free _tgf_win_free
#define strdup _tgf_win_strdup
#define _strdup _tgf_win_strdup
extern void * _tgf_win_malloc(size_t size);
extern void * _tgf_win_calloc(size_t num, size_t size);
extern void * _tgf_win_realloc(void * memblock, size_t size);
extern void _tgf_win_free(void * memblock);
extern char * _tgf_win_strdup(const char * str);
#endif // WIN32
// </esppat>

/*********************************
 * Interface For Dynamic Modules *
 *********************************/

/** initialisation of the function table 
    @see	ModInfo
*/
typedef int (*tfModPrivInit)(int index, void *);

/** Maximum number of interface in one DLL
    @see	ModList
 */
#define MAX_MOD_ITF 10

/** Module information structure  */
typedef struct ModInfo {
    char		*name;		/**< name of the module (short) (NULL if no module) */
    char		*desc;		/**< description of the module (can be long) */
    tfModPrivInit	fctInit;	/**< init function */
    unsigned int	gfId;		/**< supported framework version */
    int			index;		/**< index if multiple interface in one dll */
    int			prio;		/**< priority if needed */
    int			magic;		/**< magic number for integrity check */
} tModInfo;

/* module init function interface */
typedef int (*tfModInfo)(tModInfo *);	/* first function called in the module */

/* module shutdown function interface */
typedef int (*tfModShut)(void);	/* last function called in the module */


/** list of module interfaces */
typedef struct ModList {
    tModInfo		modInfo[MAX_MOD_ITF];	/**< module info list for this dll */
#ifdef _WIN32
    HMODULE		handle;			/**< handle of loaded module */
#else
    void		*handle;		/**< handle of loaded module */
#endif
    char		*sopath;		/**< path name of file */
    struct ModList	*next;			/**< next module in list */
} tModList;


extern int GfModLoad(unsigned int gfid, char *dllname, tModList **modlist);
extern int GfModLoadDir(unsigned int gfid, char *dir, tModList **modlist);
extern int GfModUnloadList(tModList **modlist);
extern int GfModInfo(unsigned int gfid, char *filename, tModList **modlist);
extern int GfModInfoDir(unsigned int gfid, char *dir, int level, tModList **modlist);
extern int GfModFreeInfoList(tModList **modlist);

/************************
 * Directory management *
 ************************/

/** List of (DLL) files for a Directory 
    @see	GfDirGetList
*/
typedef struct FList 
{
    struct FList	*next;		/**< Next entry */
    struct FList	*prev;		/**< Previous entry */
    char		*name;		/**< File name */
    char		*dispName;	/**< Name to display on screen */
    void		*userData;	/**< User data */
} tFList;

extern tFList *GfDirGetList(char *dir);
extern tFList *GfDirGetListFiltered(char *dir, char *suffix);
typedef void (*tfDirfreeUserData)(void*);	/**< Function to call for releasing the user data associated with file entry */
extern void GfDirFreeList(tFList *list, tfDirfreeUserData freeUserDatabool, bool freename = false, bool freedispname = false);


/**********************************
 *  Interface For Parameter Files *
 **********************************/

/*
 *	This set of function is used to store and retrieve
 *	values in parameters files.
 */


/* parameters file type */
#define GFPARM_PARAMETER	0	/**< Parameter file */
#define GFPARM_TEMPLATE		1	/**< Template file */
#define GFPARM_PARAM_STR	"param"
#define GFPARM_TEMPL_STR	"template"

/* parameters access mode */
#define GFPARM_MODIFIABLE	1	/**< Parameter file allowed to be modified */
#define GFPARM_WRITABLE		2	/**< Parameter file allowed to be saved on disk */

/* parameter file read */
#define GFPARM_RMODE_STD	0x01	/**< if handle already openned return it */
#define GFPARM_RMODE_REREAD	0x02	/**< reread the parameters from file and release the previous ones */
#define GFPARM_RMODE_CREAT	0x04	/**< Create the file if doesn't exist */
#define GFPARM_RMODE_PRIVATE	0x08

extern void *GfParmReadFile(const char *file, int mode);
/* parameter file write */
extern int GfParmWriteFile(const char *file, void* handle, char *name);

extern char *GfParmGetName(void *handle);
extern char *GfParmGetFileName(void *handle);

/* set the dtd and header values */
extern void GfParmSetDTD (void *parmHandle, char *dtd, char*header);

/* get string parameter value */
extern char *GfParmGetStr(void *handle, const char *path, const char *key, char *deflt);
/* get string parameter value */
extern char *GfParmGetCurStr(void *handle, char *path, char *key, char *deflt);
/* set string parameter value */
extern int GfParmSetStr(void *handle, char *path, char *key, char *val);
/* set string parameter value */
extern int GfParmSetCurStr(void *handle, char *path, char *key, char *val);

/* get num parameter value */
extern tdble GfParmGetNum(void *handle, const char *path, const char *key, const char *unit, tdble deflt);
/* get num parameter value */
extern tdble GfParmGetCurNum(void *handle, char *path, char *key, char *unit, tdble deflt);
/* set num parameter value */
extern int GfParmSetNum(void *handle, const char *path, const char *key, const char *unit, tdble val);
/* set num parameter value */
extern int GfParmSetCurNum(void *handle, char *path, char *key, char *unit, tdble val);


/* clean all the parameters of a set */
extern void GfParmClean(void *handle);
/* clean the parms and release the handle without updating the file */
extern void GfParmReleaseHandle(void *handle);

/* Convert a value in "units" into SI */
extern tdble GfParmUnit2SI(const char *unit, tdble val);
/* convert a value in SI to "units" */
extern tdble GfParmSI2Unit(const char *unit, tdble val);

/* compare and merge different handles */
extern int GfParmCheckHandle(void *ref, void *tgt);
#define GFPARM_MMODE_SRC	1 /**< use ref and modify existing parameters with tgt */
#define GFPARM_MMODE_DST	2 /**< use tgt and verify ref parameters */
#define GFPARM_MMODE_RELSRC	4 /**< release ref after the merge */
#define GFPARM_MMODE_RELDST	8 /**< release tgt after the merge */
extern void *GfParmMergeHandles(void *ref, void *tgt, int mode);
extern int GfParmGetNumBoundaries(void *handle, char *path, char *key, tdble *min, tdble *max);


extern int GfParmGetEltNb(void *handle, char *path);
extern int GfParmListSeekFirst(void *handle, char *path);
extern int GfParmListSeekNext(void *handle, char *path);
extern char *GfParmListGetCurEltName(void *handle, char *path);
extern int GfParmListClean(void *handle, char *path);

/******************* 
 * Trace Interface *
 *******************/

#ifdef WIN32
#define GfTrace	printf
#define GfFatal printf
#else

#define GfTrace printf

static inline void
GfFatal(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    /* GfScrShutdown(); */
    assert (0);
    exit (1);
}
#endif

#define GfError printf

#if !(_DEBUG || DEBUG)
#ifdef WIN32
#define GfOut printf
#else


/** Console output
    @param	s	string to display
    @param	args	printf args
    @fn	 GfOut(s, args...)
 */
static inline void
GfOut(char *fmt, ...)
{
}

#endif /* WIN32 */

#else /* _DEBUG || DEBUG */

#define GfOut printf

#endif /* _DEBUG || DEBUG */

/******************* 
 * Time  Interface *
 *******************/
extern double GfTimeClock(void);
extern char *GfGetTimeStr(void);

/* Mean values */
#define GF_MEAN_MAX_VAL	5

typedef struct 
{
    int		curNum;
    tdble	val[GF_MEAN_MAX_VAL+1];
} tMeanVal;

extern tdble gfMean(tdble v, tMeanVal *pvt, int n, int w);
extern void gfMeanReset(tdble v, tMeanVal *pvt);

/* MISC */
extern char *GetLocalDir(void);
extern void SetLocalDir(char *buf);
extern char *GetLibDir(void);
extern void SetLibDir(char *buf);
extern char *GetDataDir(void);
extern void SetDataDir(char *buf);
extern int GetSingleTextureMode (void);
extern void SetSingleTextureMode (void);
extern int GfNearestPow2 (int x);
extern int GfCreateDir(char *path);

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

/*
 * Tail queue definitions.
 */
/** Head type definition
    @ingroup tailq */
#define GF_TAILQ_HEAD(name, type)					\
typedef struct name {							\
	type *tqh_first;	/* first element */			\
	type **tqh_last;	/* addr of last next element */		\
} t ## name

/** Entry in structure
    @ingroup tailq */
#define GF_TAILQ_ENTRY(type)						\
struct {								\
	type *tqe_next;	/* next element */				\
	type **tqe_prev;	/* address of previous next element */	\
}

/** First element of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_FIRST(head)		((head)->tqh_first)
/** Next element of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_NEXT(elm, field)	((elm)->field.tqe_next)
/** End of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_END(head)		NULL
/** Last element of a TAILQ
    @ingroup tailq */
#define GF_TAILQ_LAST(head, headname) 					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
/** Previous element of a TAILQ
    @ingroup tailq */
#define GF_TAILQ_PREV(elm, headname, field) 				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

/*
 * Tail queue functions.
 */
/** Head initialization (Mandatory)
    @ingroup tailq */
#define	GF_TAILQ_INIT(head) do {					\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

/** Entry initialization (optionnal if inserted)
    @ingroup tailq */
#define GF_TAILQ_INIT_ENTRY(elm, field) do {	\
  (elm)->field.tqe_next = 0;			\
  (elm)->field.tqe_prev = 0;			\
} while (0)

/** Insert an element at the head
    @ingroup tailq */
#define GF_TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

/** Insert an element at the tail
    @ingroup tailq */
#define GF_TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

/** Insert an element after another element
    @ingroup tailq */
#define GF_TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

/** Insert an element before another element
    @ingroup tailq */
#define	GF_TAILQ_INSERT_BEFORE(listelm, elm, field) do {		\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

/** Remove an element
    @ingroup tailq */
#define GF_TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (0)


/* author      : Henrik Enqvist IB (henqvist@abo.fi) */
#ifdef PROFILER

#include <vector>
#include <map>

#define START_PROFILE(name) Profiler::getInstance()->startProfile(name)
#define STOP_PROFILE(name) Profiler::getInstance()->stopProfile()
#define STOP_ACTIVE_PROFILES() Profiler::getInstance()->stopActiveProfiles()
#define PRINT_PROFILE() Profiler::getInstance()->printProfile()

class ProfileInstance {
 public:
  ProfileInstance(const char * pname);
  ~ProfileInstance();
  char name[256];
  int calls;
  int openCalls;
  double totalTime;
  double addTime;
  double subTime;
  double tmpStart;
  std::map<ProfileInstance *, void *> mapChildren;
};

/** A simple high-level profiler for non-threaded non-recursive functions. */
class Profiler {
 protected:
  Profiler();
 public:
  ~Profiler();
  static Profiler * getInstance();
  void startProfile(const char * pname);
  void stopProfile();
  void stopActiveProfiles();
  void printProfile();
 private:
  static Profiler * profiler;
  ProfileInstance * curProfile;
  double fStartTime;
  std::vector<ProfileInstance *> vecProfiles;
  std::vector<ProfileInstance *> stkProfiles;
  std::map<ProfileInstance *, void *> mapWarning;
};

#else /* PROFILER */
#define START_PROFILE(a)
#define STOP_PROFILE(a)
#define STOP_ACTIVE_PROFILES()
#define PRINT_PROFILE()
#endif

/*******************/
/*   Hash Tables   */
/*******************/
#define GF_HASH_TYPE_STR	0	/**< String key based hash table */
#define GF_HASH_TYPE_BUF	1	/**< Memory buffer key based hash table */

typedef void (*tfHashFree)(void*);	/**< Function to call for releasing the user data associated with hash table */

void *GfHashCreate(int type);
int GfHashAddStr(void *hash, const char *key, void *data);
void *GfHashRemStr(void *hash, char *key);
void *GfHashGetStr(void *hash, const char *key);
void GfHashAddBuf(void *hash, char *key, size_t sz, void *data);
void *GfHashRemBuf(void *hash, char *key, size_t sz);
void *GfHashGetBuf(void *hash, char *key, size_t sz);
void GfHashRelease(void *hash, tfHashFree hashFree);
void *GfHashGetFirst(void *hash);
void *GfHashGetNext(void *hash);

#define GF_DIR_CREATION_FAILED 0
#define GF_DIR_CREATED 1

/* Helper to set and get the flags used to remove damage, time-limit and fuel consumption */
extern void setDamageLimit(bool damageLimit);
extern bool getDamageLimit();
extern void setFuelConsumption(bool fuelConsumption);
extern bool getFuelConsumption();
extern void setLaptimeLimit(bool laptimeLimit);
extern bool getLaptimeLimit();
/* Helper to set and get the timeout of UDP comm */
extern void setTimeout(long int timeout);
extern long int getTimeout();

extern void setNoisy(bool noisy);
extern bool getNoisy();

extern void setVersion(char *version);
extern char* getVersion();

// GIUSE - UDP LISTEN PORT
extern void setUDPListenPort(int port);
extern int getUDPListenPort();

// GIUSE - VISION HERE! ACTIVATE IMAGE GENERATION (and send them to clients if specified in the car/server)
extern void setVision(bool vis);
extern int getVision();

// GIUSE - FASTER THEN RUNTIME ACTIVATION FOR NON-TEXTUAL COMPUTATION
extern void setSpeedMult(double speedMult);
extern double getSpeedMult();


#endif /* __TGF__H__ */


