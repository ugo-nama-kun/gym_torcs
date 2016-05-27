/***************************************************************************

    file                 : linuxspec.cpp
    created              : Sat Mar 18 23:54:05 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: linuxspec.cpp,v 1.13.2.1 2008/11/09 17:50:22 berniw Exp $

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
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <tgf.h>

#include "os.h"

/*
 * Function
 *	linuxModLoad
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
linuxModLoad(unsigned int /* gfid */, char *sopath, tModList **modlist)
{
    tfModInfo		fModInfo;	/* init function of the modules */
    void		*handle;	/* */
    tModList		*curMod;
    char		dname[256];	/* name of the funtions */
    char		*lastSlash;
    
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    lastSlash = strrchr(sopath, '/');
    if (lastSlash) {
	strcpy(dname, lastSlash+1);
    } else {
	strcpy(dname, sopath);
    }
    dname[strlen(dname) - 3] = 0; /* cut .so */
    
    handle = dlopen(sopath, RTLD_LAZY);
    if (handle != NULL) {
	if ((fModInfo = (tfModInfo)dlsym(handle, dname)) != NULL) {
	    /* DLL loaded, init function exists, call it... */
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
		dlclose(handle);
		printf("linuxModLoad: Module: %s not loaded\n", dname);
		return -1;
	    }
	} else {
	    printf("linuxModLoad: ...  %s\n", dlerror());
	    dlclose(handle);
	    return -1;
	}
    } else {
	printf("linuxModLoad: ...  %s\n", dlerror());
	return -1;
    }
    
    return 0;
}

/*
 * Function
 *	linuxModLoad
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
linuxModInfo(unsigned int /* gfid */, char *sopath, tModList **modlist)
{
    tfModInfo		fModInfo;	/* init function of the modules */
    void		*handle;	/* */
    tModList		*curMod;
    char		dname[256];	/* name of the funtions */
    char		*lastSlash;
    int			i;
    tModList		*cMod;
    int			prio;
    
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    lastSlash = strrchr(sopath, '/');
    if (lastSlash) {
	strcpy(dname, lastSlash+1);
    } else {
	strcpy(dname, sopath);
    }
    dname[strlen(dname) - 3] = 0; /* cut .so */
    
    handle = dlopen(sopath, RTLD_LAZY);
    if (handle != NULL) {
	if ((fModInfo = (tfModInfo)dlsym(handle, dname)) != NULL) {
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
		dlclose(handle);
	    } else {
		dlclose(handle);
		printf("linuxModInfo: Module: %s not loaded\n", dname);
		return -1;
	    }
	} else {
	    printf("linuxModInfo: ...  %s\n", dlerror());
	    dlclose(handle);
	    return -1;
	}
    } else {
	printf("linuxModInfo: ...  %s\n", dlerror());
	return -1;
    }
    
    return 0;
}

/*
 * Function
 *	linuxModLoadDir
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
linuxModLoadDir(unsigned int gfid, char *dir, tModList **modlist)
{
    tfModInfo		fModInfo;	/* init function of the modules */
    char		dname[256];	/* name of the funtions */
    char		sopath[256];	/* path of the lib[x].so */
    void		*handle;	/* */
    DIR			*dp;		/* */
    struct dirent	*ep;		/* */
    int			modnb;		/* number on loaded modules */
    tModList		*curMod;
    tModList		*cMod;
    int			prio;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    /* open the current directory */
    dp = opendir(dir);
    if (dp != NULL) {
	/* some files in it */
	while ((ep = readdir (dp)) != 0) {
	    if ((strlen(ep->d_name) > 4) &&
		(strcmp(".so", ep->d_name+strlen(ep->d_name)-3) == 0)) { /* xxxx.so */
		sprintf(sopath, "%s/%s", dir, ep->d_name);
		strcpy(dname, ep->d_name);
		dname[strlen(dname) - 3] = 0; /* cut .so */
		handle = dlopen(sopath, RTLD_LAZY);
		if (handle != NULL) {
		    if ((fModInfo = (tfModInfo)dlsym(handle, dname)) != NULL) {
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
			    dlclose(handle);
			    GfTrace("linuxModLoadDir: Module: %s not retained\n", dname);
			}
		    } else {
			printf("linuxModLoadDir: ...  %s [1]\n", dlerror());
			dlclose(handle);
			(void) closedir (dp);
			return -1;
		    }
		} else {
		    printf("linuxModLoadDir: ...  %s [2]\n", dlerror());
		    (void) closedir (dp);
		    return -1;
		}
	    }
	}
	(void) closedir (dp);
    } else {
	printf("linuxModLoadDir: ... Couldn't open the directory %s\n", dir);
	return -1;
    }

    free(curMod);
    return modnb;
}

/*
 * Function
 *	linuxModInfoDir
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
linuxModInfoDir(unsigned int /* gfid */, char *dir, int level, tModList **modlist)
{
    tfModInfo		fModInfo;	/* init function of the modules */
    char		dname[256];	/* name of the funtions */
    char		sopath[256];	/* path of the lib[x].so */
    void		*handle;	/* */
    DIR			*dp;		/* */
    struct dirent	*ep;		/* */
    int			modnb;		/* number on loaded modules */
    tModList		*curMod;
    int			i;
    tModList		*cMod;
    int			prio;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    /* open the current directory */
    dp = opendir(dir);
    if (dp != NULL) {
	/* some files in it */
	while ((ep = readdir (dp)) != 0) {
	    if (((strlen(ep->d_name) > 4) && 
		 (strcmp(".so", ep->d_name+strlen(ep->d_name)-3) == 0)) || 
		((level == 1) && (ep->d_name[0] != '.'))) { /* xxxx.so */
		if (level == 1) {
		    sprintf(sopath, "%s/%s/%s.so", dir, ep->d_name, ep->d_name);
		    strcpy(dname, ep->d_name);
		} else {
		    sprintf(sopath, "%s/%s", dir, ep->d_name);
		    strcpy(dname, ep->d_name);
		    dname[strlen(dname) - 3] = 0; /* cut .so */
		}
		handle = dlopen(sopath, RTLD_LAZY);
		if (handle != NULL) {
		    if ((fModInfo = (tfModInfo)dlsym(handle, dname)) != NULL) {
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
			    dlclose(handle);
			    curMod = (tModList*)calloc(1, sizeof(tModList));
			} else {
			    dlclose(handle);
			    GfTrace("linuxModInfoDir: Module: %s not retained\n", dname);
			}
		    } else {
			printf("linuxModInfoDir: ...  %s [1]\n", dlerror());
			dlclose(handle);
		    }
		} else {
		    printf("linuxModInfoDir: ...  %s [2]\n", dlerror());
		}
	    }
	}
	(void) closedir (dp);
    } else {
	printf("linuxModInfoDir: ... Couldn't open the directory %s.\n", dir);
	return -1;
    }

    free(curMod);
    return modnb;
}

/*
 * Function
 *	linuxModUnloadList
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
linuxModUnloadList(tModList **modlist)
{
    tModList		*curMod;
    tModList		*nextMod;
    tfModShut		fModShut;
    char		dname[256];	/* name of the funtions */
    char		*lastSlash;

    curMod = *modlist;
    if (curMod == 0) {
	return 0;
    }
    nextMod = curMod->next;
    do {
	curMod = nextMod;
	nextMod = curMod->next;
	GfOut("<<< %s unloaded <<<\n", curMod->sopath);
	lastSlash = strrchr(curMod->sopath, '/');
	if (lastSlash) {
	    strcpy(dname, lastSlash+1);
	} else {
	    strcpy(dname, curMod->sopath);
	}
	strcpy(&dname[strlen(dname) - 3], "Shut"); /* cut .so */
	if ((fModShut = (tfModShut)dlsym(curMod->handle, dname)) != NULL) {
	    GfOut("Call %s\n", dname);
	    fModShut();
	}

	// Comment out for valgrind runs, be aware that the driving with the keyboard does
	// just work to first time this way.
	dlclose(curMod->handle);

	free(curMod->sopath);
	free(curMod);
    } while (curMod != *modlist);
    
    *modlist = (tModList *)NULL;
    return 0;
}

/*
 * Function
 *	linuxModFreeInfoList
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
linuxModFreeInfoList(tModList **modlist)
{
    tModList		*curMod;
    tModList		*nextMod;
    int			i;
    
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
 *	linuxDirGetList
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
linuxDirGetList(char *dir)
{
	DIR *dp;
	struct dirent *ep;
	tFList *flist = (tFList*)NULL;
	tFList *curf;

	/* open the current directory */
	dp = opendir(dir);
	if (dp != NULL) {
		/* some files in it */
		while ((ep = readdir(dp)) != 0) {
			if ((strcmp(ep->d_name, ".") != 0) && (strcmp(ep->d_name, "..") != 0)) {
				curf = (tFList*)calloc(1, sizeof(tFList));
				curf->name = strdup(ep->d_name);
				if (flist == (tFList*)NULL) {
					curf->next = curf;
					curf->prev = curf;
					flist = curf;
				} else {
					/* sort entries... */
					if (strcasecmp(curf->name, flist->name) > 0) {
						do {
							flist = flist->next;
						} while ((strcasecmp(curf->name, flist->name) > 0) && (strcasecmp(flist->name, flist->prev->name) > 0));
						flist = flist->prev;
					} else {
						do {
							flist = flist->prev;
						} while ((strcasecmp(curf->name, flist->name) < 0) && (strcasecmp(flist->name, flist->next->name) < 0));
					}
					curf->next = flist->next;
					flist->next = curf;
					curf->prev = flist;
					curf->next->prev = curf;
					flist = curf;
				}
			}
		}
		closedir(dp);
	}
    return flist;
}

/*
 * Function
 *	linuxDirGetListFiltered
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
linuxDirGetListFiltered(char *dir, char *suffix)
{
	DIR	*dp;
	struct dirent *ep;
	tFList *flist = (tFList*)NULL;
	tFList *curf;
	int	suffixLg;
	int	fnameLg;

	if ((suffix == NULL) || (strlen(suffix) == 0)) {
		return linuxDirGetList(dir);
	}

	suffixLg = strlen(suffix);

	/* open the current directory */
	dp = opendir(dir);
	if (dp != NULL) {
		/* some files in it */
		while ((ep = readdir(dp)) != 0) {
			fnameLg = strlen(ep->d_name);
			if ((fnameLg > suffixLg) && (strcmp(ep->d_name + fnameLg - suffixLg, suffix) == 0)) {
				curf = (tFList*)calloc(1, sizeof(tFList));
				curf->name = strdup(ep->d_name);
				if (flist == (tFList*)NULL) {
					curf->next = curf;
					curf->prev = curf;
					flist = curf;
				} else {
					/* sort entries... */
					if (strcasecmp(curf->name, flist->name) > 0) {
						do {
							flist = flist->next;
						} while ((strcasecmp(curf->name, flist->name) > 0) && (strcasecmp(flist->name, flist->prev->name) > 0));
						flist = flist->prev;
					} else {
						do {
							flist = flist->prev;
						} while ((strcasecmp(curf->name, flist->name) < 0) && (strcasecmp(flist->name, flist->next->name) < 0));
					}
					curf->next = flist->next;
					flist->next = curf;
					curf->prev = flist;
					curf->next->prev = curf;
					flist = curf;
				}
	    	}
		}
		closedir(dp);
	}
	return flist;
}

static double
linuxTimeClock(void)
{
    struct timeval tv;

    gettimeofday(&tv, 0);
    return (double)(tv.tv_sec + tv.tv_usec * 1e-6);

}


/*
 * Function
 *	LinuxSpecInit
 *
 * Description
 *	Init the specific linux functions
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
LinuxSpecInit(void)
{
    memset(&GfOs, 0, sizeof(GfOs));

    GfOs.modLoad = linuxModLoad;
    GfOs.modLoadDir = linuxModLoadDir;
    GfOs.modUnloadList = linuxModUnloadList;
    GfOs.modInfo = linuxModInfo;
    GfOs.modInfoDir = linuxModInfoDir;
    GfOs.modFreeInfoList = linuxModFreeInfoList;
    GfOs.dirGetList = linuxDirGetList;
    GfOs.dirGetListFiltered = linuxDirGetListFiltered;
    GfOs.timeClock = linuxTimeClock;
}

