/***************************************************************************

    file                 : windowsspec.cpp
    created              : Sat Sep  2 10:45:39 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
    email                : torcs@free.fr
    version              : $Id: windowsspec.cpp,v 1.12.2.1 2008/11/09 17:51:05 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <tgf.h>
#include <os.h>
#include <direct.h>
#include <io.h>

extern "C" int
ssggraph(tModInfo *modInfo);

/*
 * Function
 *	windowsModLoad
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
static int
windowsModLoad(unsigned int gfid, char *sopath, tModList **modlist)
{
    tfModInfo	fModInfo;	/* init function of the modules */
    HMODULE	handle;		/* */
    tModList	*curMod;
    char	dname[256];	/* name of the funtions */
    char	*lastSlash;
    
    curMod = (tModList*)calloc(1, sizeof(tModList));
    GfOut("loading windows module %s\n",sopath);
    lastSlash = strrchr(sopath, '/');
    if (lastSlash) {
	strcpy(dname, lastSlash+1);
    } else {
	strcpy(dname, sopath);
    }
    dname[strlen(dname) - 4] = 0; /* cut .dll */
    
    GfOut("LoadLibrary from %s\n",sopath);

    /* This one doesn't load dynamically... */
    if (strcmp(sopath,"modules/graphic/ssggraph.dll") == 0) {
        ssggraph(curMod->modInfo);
        if (*modlist == NULL) {
	    *modlist = curMod;
	    curMod->next = curMod;
	} else {
	    curMod->next = (*modlist)->next;
	    (*modlist)->next = curMod;
	    *modlist = curMod;
	}
	GfOut("%s loaded staticaly\n",sopath);
	return 0;
    } 

    handle = LoadLibrary( sopath ); 
    GfOut("LoadLibrary return from %s\n",sopath);
    if (handle != NULL) {
	if ((fModInfo = (tfModInfo)GetProcAddress(handle, dname)) != NULL) {
	    /* DLL loaded, init function exists, call it... */
	    GfOut("calling modInfo from %s\n",sopath);
	    if (fModInfo(curMod->modInfo) == 0) {
		GfOut(">>> %s >>>\n", sopath);
		curMod->handle = handle;
		curMod->sopath = strdup(sopath);
		if (*modlist == NULL) {
		    *modlist = curMod;
		    curMod->next = curMod;
		} else {
		    curMod->next = (*modlist)->next;
		    (*modlist)->next = curMod;
		    *modlist = curMod;
		}
	    } else {
		FreeLibrary(handle);
		GfTrace("windowsModLoad: Module: %s not loaded\n", dname);
		return -1;
	    }
	} else {
	    GfTrace("windowsModLoad: ... can't find proc %s\n", dname);
	    FreeLibrary(handle);
	    return -1;
	}
    } else {
	GfTrace("windowsModLoad: ...  can't open dll %s\n", sopath);
	return -1;
    }
      
    GfOut("windows module %s loaded\n",sopath);
    return 0;
}

/*
* Function
*	windowsModInfo
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
static int
windowsModInfo(unsigned int gfid, char *sopath, tModList **modlist)
{
    tfModInfo	fModInfo;	/* init function of the modules */
    HMODULE	handle;		/* */
    tModList	*curMod;
    char	dname[256];	/* name of the funtions */
    char	*lastSlash;
    int		i;
    tModList	*cMod;
    int		prio;
    
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    lastSlash = strrchr(sopath, '/');
    if (lastSlash) {
	strcpy(dname, lastSlash+1);
    } else {
	strcpy(dname, sopath);
    }
    dname[strlen(dname) - 4] = 0; /* cut .dll */
    
    handle = LoadLibrary( sopath );
    if (handle != NULL) {
	if ((fModInfo = (tfModInfo)GetProcAddress(handle, dname)) != NULL) {
	    /* DLL loaded, init function exists, call it... */
	    if (fModInfo(curMod->modInfo) == 0) {
		GfOut("Request Info for %s\n", sopath);
		for (i = 0; i < MAX_MOD_ITF; i++) {
		    if (curMod->modInfo[i].name) {
			curMod->modInfo[i].name = strdup(curMod->modInfo[i].name);
			curMod->modInfo[i].desc = strdup(curMod->modInfo[i].desc);
		    }
		}
		curMod->handle = NULL;
		curMod->sopath = strdup(sopath);
		if (*modlist == NULL) {
		    *modlist = curMod;
		    curMod->next = curMod;
		} else {
		    /* sort by prio */
		    prio = curMod->modInfo[0].prio;
		    if (prio >= (*modlist)->modInfo[0].prio) {
			curMod->next = (*modlist)->next;
			(*modlist)->next = curMod;
			*modlist = curMod;
		    } else {
			cMod = *modlist;
			do {
			    if (prio < cMod->next->modInfo[0].prio) {
				curMod->next = cMod->next;
				cMod->next = curMod;
				break;
			    }
			    cMod = cMod->next;
			} while (cMod != *modlist);
		    }
		}
		FreeLibrary(handle);
	    } else {
		FreeLibrary(handle);
		GfTrace("windowsModInfo: Module: %s not loaded\n", dname);
		return -1;
	    }
	} else {
	    GfTrace("windowsModInfo: ...  %d\n", GetLastError());
	    FreeLibrary(handle);
	    return -1;
	}
    } else {
	GfTrace("windowsModInfo: ...  %d\n", GetLastError());
	return -1;
    }
    
    return 0;
}

/*
* Function
*	windowsModLoadDir
*
* Description
*	Load the modules contained in a directory
*
* Parameters
*	dir	directory to search (relative)
*	modlist	list of module description structure
*
* Return
*	>=0	number of modules loaded
*	-1	error
*
* Remarks
*	
*/
static int
windowsModLoadDir(unsigned int gfid, char *dir, tModList **modlist)
{
    tfModInfo	fModInfo;	/* init function of the modules */
    char	dname[256];	/* name of the funtions */
    char	sopath[256];	/* path of the lib[x].so */
    HMODULE	handle;		/* */
    int		modnb;		/* number on loaded modules */
    tModList	*curMod;
    tModList	*cMod;
    int		prio;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));

    // parcours du r�pertoire
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.dll", dir );
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 )
	do {
	    sprintf(sopath, "%s\\%s", dir, FData.name);
	    handle = LoadLibrary( sopath );
	    if (handle != NULL) {
		if ((fModInfo = (tfModInfo)GetProcAddress(handle, dname)) != NULL) {
		    /* DLL loaded, init function exists, call it... */
		    if ((fModInfo(curMod->modInfo) == 0) && (curMod->modInfo[0].gfId == gfid)) {
			GfOut(">>> %s loaded >>>\n", sopath);
			modnb++;
			curMod->handle = handle;
			curMod->sopath = strdup(sopath);
			/* add the module in the list */
			if (*modlist == NULL) {
			    *modlist = curMod;
			    curMod->next = curMod;
			} else {
			    /* sort by prio */
			    prio = curMod->modInfo[0].prio;
			    if (prio >= (*modlist)->modInfo[0].prio) {
				curMod->next = (*modlist)->next;
				(*modlist)->next = curMod;
				*modlist = curMod;
			    } else {
				cMod = *modlist;
				do {
				    if (prio < cMod->next->modInfo[0].prio) {
					curMod->next = cMod->next;
					cMod->next = curMod;
					break;
				    }
				    cMod = cMod->next;
				} while (cMod != *modlist);
			    }
			}
			curMod = (tModList*)calloc(1, sizeof(tModList));
		    } else {
			FreeLibrary(handle);
			GfTrace("windowsModLoadDir: Module: %s not retained\n", dname);
		    }
		} else {
		    GfTrace("windowsModLoadDir: ...  can't find proc %s\n", dname);
		    FreeLibrary(handle);
		    _findclose( Dirent );
		    return -1;
		}
	    }
	} while ( _findnext( Dirent, &FData ) != -1 );

    _findclose( Dirent );

    free(curMod);
    return modnb;
}
/*
* Function
*	windowsModInfoDir
*
* Description
*	Load the modules contained in a directory and retrieve info
*
* Parameters
*	dir	directory to search (relative)
*	modlist	list of module description structure
*
* Return
*	>=0	number of modules loaded
*	-1	error
*
* Remarks
*	
*/
static int
windowsModInfoDir(unsigned int gfid, char *dir, int level, tModList **modlist)
{
    tfModInfo	fModInfo;	/* init function of the modules */
    char	dname[256];	/* name of the funtions */
    char	sopath[256];	/* path of the lib[x].so */
    HMODULE	handle;		/* */
    int		modnb;		/* number on loaded modules */
    tModList	*curMod;
    int		i;
    tModList	*cMod;
    int		prio;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    /* open the current directory */
    _finddata_t FData;

    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir info %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 ) {
	do {
	    if (((strlen(FData.name) > 5) && 
		 (strcmp(".dll", FData.name+strlen(FData.name)-4) == 0)) || (level == 1)) { /* xxxx.dll */
		if (level == 1) {
		    sprintf(sopath, "%s/%s/%s.dll", dir, FData.name, FData.name);
		    strcpy(dname, FData.name);
		} else {
		    sprintf(sopath, "%s/%s", dir, FData.name);
		    strcpy(dname, FData.name);
		    dname[strlen(dname) - 4] = 0; /* cut .dll */
		}
		handle = LoadLibrary( sopath );
		if (handle != NULL) {
		    if ((fModInfo = (tfModInfo)GetProcAddress(handle, dname)) != NULL) {
			GfOut("Request Info for %s\n", sopath);
			/* DLL loaded, init function exists, call it... */
			if (fModInfo(curMod->modInfo) == 0) {
			    modnb++;
			    for (i = 0; i < MAX_MOD_ITF; i++) {
				if (curMod->modInfo[i].name) {
				    curMod->modInfo[i].name = strdup(curMod->modInfo[i].name);
				    curMod->modInfo[i].desc = strdup(curMod->modInfo[i].desc);
				}
			    }
			    curMod->handle = NULL;
			    curMod->sopath = strdup(sopath);
			    /* add the module in the list */
			    if (*modlist == NULL) {
				*modlist = curMod;
				curMod->next = curMod;
			    } else {
				/* sort by prio */
				prio = curMod->modInfo[0].prio;
				if (prio >= (*modlist)->modInfo[0].prio) {
				    curMod->next = (*modlist)->next;
				    (*modlist)->next = curMod;
				    *modlist = curMod;
				} else {
				    cMod = *modlist;
				    do {
					if (prio < cMod->next->modInfo[0].prio) {
					    curMod->next = cMod->next;
					    cMod->next = curMod;
					    break;
					}
					cMod = cMod->next;
				    } while (cMod != *modlist);
				}
			    }
			    FreeLibrary(handle);
			    curMod = (tModList*)calloc(1, sizeof(tModList));
			} else {
			    FreeLibrary(handle);
			    GfTrace("windowsModInfoDir: Module: %s not retained\n", dname);
			}
		    } else {
			GfTrace("windowsModInfoDir: ...  can't find proc %s\n", dname);
			FreeLibrary(handle);
		    }
		} else {
		    GfTrace("windowsModInfoDir: ...  can't open dll %s\n", sopath);
		}
	    }
	} while ( _findnext( Dirent, &FData ) != -1 );
    }
    
    _findclose( Dirent );
    free(curMod);
    return modnb;
}

/*
* Function
*	windowsModUnloadList
*
* Description
*	Unload the modules of a list
*
* Parameters
*	modlist	list of modules to unload
*
* Return
*	0	Ok
*	-1	Error
*
* Remarks
*	
*/
static int
windowsModUnloadList(tModList **modlist)
{
    tModList	*curMod;
    tModList	*nextMod;
    
    curMod = *modlist;
    if (curMod == 0) {
	return 0;
    }
    nextMod = curMod->next;
    do {
	curMod = nextMod;
	nextMod = curMod->next;
	GfOut("<<< %s unloaded <<<\n", curMod->sopath);
	FreeLibrary(curMod->handle);
	free(curMod->sopath);
	free(curMod);
    } while (curMod != *modlist);
    
    *modlist = (tModList *)NULL;
    return 0;
}

/*
* Function
*	windowsModFreeInfoList
*
* Description
*	
*
* Parameters
*	modlist	list of info to free
*
* Return
*	0	Ok
*	-1	Error
*
* Remarks
*	
*/
static int
windowsModFreeInfoList(tModList **modlist)
{
    tModList	*curMod;
    tModList	*nextMod;
    int		i;
    
    curMod = *modlist;
    nextMod = curMod->next;
    do {
	curMod = nextMod;
	for (i = 0; i < MAX_MOD_ITF; i++) {
	    if (curMod->modInfo[i].name) {
		free(curMod->modInfo[i].name);
		free(curMod->modInfo[i].desc);
	    }
	}
	free(curMod->sopath);
	free(curMod);
    } while (curMod != *modlist);
    
    *modlist = (tModList *)NULL;
    return 0;
}

/*
* Function
*	windowsDirGetList
*
* Description
*	Get a list of entries in a directory
*
* Parameters
*	directory name
*
* Return
*	list of directory entries
*/
static tFList *
windowsDirGetList(char *dir)
{
    tFList	*flist = NULL;
    tFList	*curf;
	
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 ) {
	do {
	    if ( strcmp(FData.name, ".") != 0 && strcmp(FData.name, "..") != 0 ) {
		curf = (tFList*)calloc(1, sizeof(tFList));
		curf->name = strdup(FData.name);
		if (flist == (tFList*)NULL) {
		    curf->next = curf;
		    curf->prev = curf;
		    flist = curf;
		} else {
		    /* sort entries... */
		    if (_stricmp(curf->name, flist->name) > 0) {
			do {
			    flist = flist->next;
			} while ((stricmp(curf->name, flist->name) > 0) && (stricmp(flist->name, flist->prev->name) > 0));
			flist = flist->prev;
		    } else {
			do {
			    flist = flist->prev;
			} while ((stricmp(curf->name, flist->name) < 0) && (stricmp(flist->name, flist->next->name) < 0));
		    }
		    curf->next = flist->next;
		    flist->next = curf;
		    curf->prev = curf->next->prev;
		    curf->next->prev = curf;
		    flist = curf;
		}
	    }
	} while ( _findnext( Dirent, &FData ) != -1 );
    }
    
    return flist;
}

/*
* Function
*	windowsDirGetListFiltered
*
* Description
*	Get a list of entries in a directory
*
* Parameters
*	directory name
*
* Return
*	list of directory entries
*/
static tFList *
windowsDirGetListFiltered(char *dir, char *suffix)
{
    tFList	*flist = NULL;
    tFList	*curf;
    int		suffixLg;
    int		fnameLg;

    if ((suffix == NULL) || (strlen(suffix) == 0))
	return windowsDirGetList(dir);

    suffixLg = strlen(suffix);
	
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 ) {
	do {
	    fnameLg = strlen(FData.name);
	    if ((fnameLg > suffixLg) && (strcmp(FData.name + fnameLg - suffixLg, suffix) == 0)) {
		curf = (tFList*)calloc(1, sizeof(tFList));
		curf->name = strdup(FData.name);
		if (flist == (tFList*)NULL) {
		    curf->next = curf;
		    curf->prev = curf;
		    flist = curf;
		} else {
		    /* sort entries... */
		    if (_stricmp(curf->name, flist->name) > 0) {
			do {
			    flist = flist->next;
			} while ((stricmp(curf->name, flist->name) > 0) && (stricmp(flist->name, flist->prev->name) > 0));
			flist = flist->prev;
		    } else {
			do {
			    flist = flist->prev;
			} while ((stricmp(curf->name, flist->name) < 0) && (stricmp(flist->name, flist->next->name) < 0));
		    }
		    curf->next = flist->next;
		    flist->next = curf;
		    curf->prev = curf->next->prev;
		    curf->next->prev = curf;
		    flist = curf;
		}
	    }
	} while ( _findnext( Dirent, &FData ) != -1 );
    }
    
    return flist;
}

static double
windowsTimeClock(void)
{
    LARGE_INTEGER Frequency;
    if ( !QueryPerformanceFrequency( &Frequency ) )
	return( 0 );

    LARGE_INTEGER Counter;
    if ( !QueryPerformanceCounter( &Counter ) )
	return( 0 );

    double D = (double)Counter.QuadPart / (double)Frequency.QuadPart;
    return( D );
}


/*
* Function
*	WindowsSpecInit
*
* Description
*	Init the specific windows functions
*
* Parameters
*	none
*
* Return
*	none
*
* Remarks
*	
*/
void
WindowsSpecInit(void)
{
    memset(&GfOs, 0, sizeof(GfOs));
	
    GfOs.modLoad = windowsModLoad;
    GfOs.modLoadDir = windowsModLoadDir;
    GfOs.modUnloadList = windowsModUnloadList;
    GfOs.modInfo = windowsModInfo;
    GfOs.modInfoDir = windowsModInfoDir;
    GfOs.modFreeInfoList = windowsModFreeInfoList;
    GfOs.dirGetList = windowsDirGetList;
    GfOs.dirGetListFiltered = windowsDirGetListFiltered;
    GfOs.timeClock = windowsTimeClock;


}

