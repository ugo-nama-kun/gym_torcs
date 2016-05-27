/***************************************************************************
                  trackselect.cpp -- interactive track selection
                             -------------------
    created              : Mon Aug 16 21:43:00 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: trackselect.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $
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
    @ingroup	racemantools
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: trackselect.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $
*/


#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include <track.h>
#include <osspec.h>
#include <raceman.h>
#include <racescreens.h>


/* Tracks Categories */
static tFList *CategoryList;
static void *scrHandle;
static int TrackLabelId;
static int CatLabelId;
static int MapId;
static int AuthorId;
static int LengthId;
static int WidthId;
static int DescId;
static int PitsId;
static tRmTrackSelect *ts;
static char buf[1024];
static char path[1024];


static void rmtsActivate(void * /* dummy */)
{
	/* call display function of graphic */
	//gfuiReleaseImage(MapId);
}


static void rmtsFreeLists(void *vl)
{
	GfDirFreeList((tFList*)(((tFList*)vl)->userData), NULL, true, true);
}


static char * rmGetMapName(void)
{
	sprintf(buf, "tracks/%s/%s/%s.png", CategoryList->name,
		((tFList*)CategoryList->userData)->name, ((tFList*)CategoryList->userData)->name);
	return buf;
}


static void rmtsDeactivate(void *screen)
{
	GfuiScreenRelease(scrHandle);

	GfDirFreeList(CategoryList, rmtsFreeLists, true, true);
	if (screen) {
		GfuiScreenActivate(screen);
	}
}


static void rmUpdateTrackInfo(void)
{
	void *trackHandle;
	float tmp;
	tTrack *trk;

	sprintf(buf, "tracks/%s/%s/%s.%s", CategoryList->name, ((tFList*)CategoryList->userData)->name,
		((tFList*)CategoryList->userData)->name, TRKEXT);
	trackHandle = GfParmReadFile(buf, GFPARM_RMODE_STD); /* COMMENT VALID? don't release, the name is used later */

	if (!trackHandle) {
		GfTrace("File %s has pb\n", buf);
		return;
	}
	trk = ts->trackItf.trkBuild(buf);

	GfuiLabelSetText(scrHandle, DescId, GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_DESCR, ""));
	GfuiLabelSetText(scrHandle, AuthorId, GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_AUTHOR, ""));

	tmp = GfParmGetNum(trackHandle, TRK_SECT_MAIN, TRK_ATT_WIDTH, NULL, 0);
	sprintf(buf, "%.2f m", tmp);
	GfuiLabelSetText(scrHandle, WidthId, buf);
	tmp = trk->length;
	sprintf(buf, "%.2f m", tmp);
	GfuiLabelSetText(scrHandle, LengthId, buf);

	if (trk->pits.nMaxPits != 0) {
		sprintf(buf, "%d", trk->pits.nMaxPits);
		GfuiLabelSetText(scrHandle, PitsId, buf);
	} else {
		GfuiLabelSetText(scrHandle, PitsId, "none");
	}

	ts->trackItf.trkShutdown();
	GfParmReleaseHandle(trackHandle);
}


static void rmtsPrevNext(void *vsel)
{
	if (vsel == 0) {
		CategoryList->userData = (void*)(((tFList*)CategoryList->userData)->prev);
	} else {
		CategoryList->userData = (void*)(((tFList*)CategoryList->userData)->next);
	}

	GfuiLabelSetText(scrHandle, TrackLabelId, ((tFList*)CategoryList->userData)->dispName);
	GfuiStaticImageSet(scrHandle, MapId, rmGetMapName());
	rmUpdateTrackInfo();
}


static void rmCatPrevNext(void *vsel)
{
	if (vsel == 0) {
		CategoryList = CategoryList->prev;
	} else {
		CategoryList = CategoryList->next;
	}

	GfuiLabelSetText(scrHandle, CatLabelId, CategoryList->dispName);
	GfuiLabelSetText(scrHandle, TrackLabelId, ((tFList*)CategoryList->userData)->dispName);
	GfuiStaticImageSet(scrHandle, MapId, rmGetMapName());
	rmUpdateTrackInfo();
}


void rmtsSelect(void * /* dummy */)
{
	int curTrkIdx;

	curTrkIdx = (int)GfParmGetNum(ts->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
	sprintf(path, "%s/%d", RM_SECT_TRACKS, curTrkIdx);
	GfParmSetStr(ts->param, path, RM_ATTR_CATEGORY, CategoryList->name);
	GfParmSetStr(ts->param, path, RM_ATTR_NAME, ((tFList*)CategoryList->userData)->name);

	rmtsDeactivate(ts->nextScreen);
}


static void rmtsAddKeys(void)
{
	GfuiAddKey(scrHandle, 13, "Select Track", NULL, rmtsSelect, NULL);
	GfuiAddKey(scrHandle, 27, "Cancel Selection", ts->prevScreen, rmtsDeactivate, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_LEFT, "Previous Track", (void*)0, rmtsPrevNext, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_RIGHT, "Next Track", (void*)1, rmtsPrevNext, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_UP, "Previous Track Category", (void*)0, rmCatPrevNext, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_DOWN, "Next Track Category", (void*)1, rmCatPrevNext, NULL);
}


/** Get the track name
    @param	category	track category directory
    @param	trackName	track name for file
    @return	Long track name
    @ingroup	racemantools
 */
char *
RmGetTrackName(char *category, char *trackName)
{
	void *trackHandle;
	char *name;

	sprintf(buf, "tracks/%s/%s/%s.%s", category, trackName, trackName, TRKEXT);
	trackHandle = GfParmReadFile(buf, GFPARM_RMODE_STD); /* don't release, the name is used later */

	if (trackHandle) {
		name = strdup(GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_NAME, trackName));
	} else {
		GfTrace("File %s has pb\n", buf);
		return "";
	}

	GfParmReleaseHandle(trackHandle);
	return name;
}


/** Get the track category name from the directory name
    @param	category	track category directory
    @return	category display name
    @ingroup	racemantools
*/
char *
RmGetCategoryName(char *category)
{
	void *categoryHandle;
	char *name;

	sprintf(buf, "data/tracks/%s.%s", category, TRKEXT);
	categoryHandle = GfParmReadFile(buf, GFPARM_RMODE_STD); /* don't release, the name is used later */

	if (categoryHandle) {
		name = strdup(GfParmGetStr(categoryHandle, TRK_SECT_HDR, TRK_ATT_NAME, category));
	} else {
		GfTrace("File %s has pb\n", buf);
		return "";
	}

	GfParmReleaseHandle(categoryHandle);
	return name;
}



/** Interactive track selection
    @param	vs	Pointer on a tRmTrackSelect structure (cast to void *)
    @warning	The race manager's parameters are updated but not saved.
    @ingroup	racemantools
 */
void
RmTrackSelect(void *vs)
{
	char *defaultTrack;
	char *defaultCategory;
	tFList *CatCur;
	tFList *TrList, *TrCur;
	int Xpos, Ypos, DX, DY;
	int curTrkIdx;

	ts = (tRmTrackSelect*)vs;

	/* Get the list of categories directories */
	CategoryList = GfDirGetList("tracks");
	if (CategoryList == NULL) {
		GfTrace("RmTrackSelect: No track category available\n");
		return;
	}

	CatCur = CategoryList;
	do {
		CatCur->dispName = RmGetCategoryName(CatCur->name);
		if (strlen(CatCur->dispName) == 0) {
			GfTrace("RmTrackSelect: No definition for track category %s\n", CatCur->name);
			return;
		}

		/* get the tracks in the category directory */
		sprintf(buf, "tracks/%s", CatCur->name);
		TrList = GfDirGetList(buf);
		if (TrList == NULL) {
			GfTrace("RmTrackSelect: No track for category %s available\n", CatCur->name);
			return;
		}
		TrList = TrList->next; /* get the first one */
		CatCur->userData = (void*)TrList;
		TrCur = TrList;
		do {
			TrCur->dispName = RmGetTrackName(CatCur->name, TrCur->name);
			if (strlen(TrCur->dispName) == 0) {
				GfTrace("RmTrackSelect: No definition for track %s\n", TrCur->name);
				return;
			}
			TrCur = TrCur->next;
		} while (TrCur != TrList);

		CatCur = CatCur->next;
	} while (CatCur != CategoryList);

	curTrkIdx = (int)GfParmGetNum(ts->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
	sprintf(path, "%s/%d", RM_SECT_TRACKS, curTrkIdx);
	defaultCategory = GfParmGetStr(ts->param, path, RM_ATTR_CATEGORY, CategoryList->name);
	/* XXX coherency check */
	defaultTrack = GfParmGetStr(ts->param, path, RM_ATTR_NAME, ((tFList*)CategoryList->userData)->name);

	CatCur = CategoryList;
	do {
	if (strcmp(CatCur->name, defaultCategory) == 0) {
		CategoryList = CatCur;
		TrCur = (tFList*)(CatCur->userData);
		do {
		if (strcmp(TrCur->name, defaultTrack) == 0) {
			CatCur->userData = (void*)TrCur;
			break;
		}
		TrCur = TrCur->next;
		} while (TrCur != TrList);
		break;
	}
	CatCur = CatCur->next;
	} while (CatCur != CategoryList);

	scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, rmtsActivate, NULL, (tfuiCallback)NULL, 1);
	GfuiScreenAddBgImg(scrHandle, "data/img/splash-qrtrk.png");

	rmtsAddKeys();

	GfuiTitleCreate(scrHandle, "Select Track", 0);

	GfuiGrButtonCreate(scrHandle,
			"data/img/arrow-left.png",
			"data/img/arrow-left.png",
			"data/img/arrow-left.png",
			"data/img/arrow-left-pushed.png",
			80, 400, GFUI_ALIGN_HC_VB, 0,
			(void*)0, rmCatPrevNext,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);


	CatLabelId = GfuiLabelCreate(scrHandle,
				CategoryList->dispName,
				GFUI_FONT_LARGE_C,
				320, 400, GFUI_ALIGN_HC_VB,
				30);

	GfuiGrButtonCreate(scrHandle,
			"data/img/arrow-right.png",
			"data/img/arrow-right.png",
			"data/img/arrow-right.png",
			"data/img/arrow-right-pushed.png",
			540, 400, GFUI_ALIGN_HC_VB, 0,
			(void*)1, rmCatPrevNext,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	GfuiGrButtonCreate(scrHandle,
			"data/img/arrow-left.png",
			"data/img/arrow-left.png",
			"data/img/arrow-left.png",
			"data/img/arrow-left-pushed.png",
			80, 370, GFUI_ALIGN_HC_VB, 0,
			(void*)0, rmtsPrevNext,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);


	TrackLabelId = GfuiLabelCreate(scrHandle,
				((tFList*)CategoryList->userData)->dispName,
				GFUI_FONT_LARGE_C,
				320, 370, GFUI_ALIGN_HC_VB,
				30);

	GfuiGrButtonCreate(scrHandle,
			"data/img/arrow-right.png",
			"data/img/arrow-right.png",
			"data/img/arrow-right.png",
			"data/img/arrow-right-pushed.png",
			540, 370, GFUI_ALIGN_HC_VB, 0,
			(void*)1, rmtsPrevNext,
			NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	MapId = GfuiStaticImageCreate(scrHandle,
				320, 100, 260, 195,
				rmGetMapName());

	GfuiButtonCreate(scrHandle, "Accept", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
			NULL, rmtsSelect, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	GfuiButtonCreate(scrHandle, "Back", GFUI_FONT_LARGE, 430, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
			ts->prevScreen, rmtsDeactivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	Xpos = 20;
	Ypos = 320;
	DX = 110;
	DY = 30;

	GfuiLabelCreate(scrHandle,
			"Description:",
			GFUI_FONT_MEDIUM,
			Xpos, Ypos,
			GFUI_ALIGN_HL_VB, 0);

	DescId =  GfuiLabelCreate(scrHandle,
				"",
				GFUI_FONT_MEDIUM_C,
				Xpos + DX, Ypos,
				GFUI_ALIGN_HL_VB, 50);

	Ypos -= DY;

	GfuiLabelCreate(scrHandle,
			"Author:",
			GFUI_FONT_MEDIUM,
			Xpos, Ypos,
			GFUI_ALIGN_HL_VB, 0);

	AuthorId = GfuiLabelCreate(scrHandle,
				"",
				GFUI_FONT_MEDIUM_C,
				Xpos + DX, Ypos,
				GFUI_ALIGN_HL_VB, 20);

	Ypos -= DY;

	GfuiLabelCreate(scrHandle,
			"Length:",
			GFUI_FONT_MEDIUM,
			Xpos, Ypos,
			GFUI_ALIGN_HL_VB, 0);

	LengthId = GfuiLabelCreate(scrHandle,
				"",
				GFUI_FONT_MEDIUM_C,
				Xpos + DX, Ypos,
				GFUI_ALIGN_HL_VB, 20);

	Ypos -= DY;

	GfuiLabelCreate(scrHandle,
			"Width:",
			GFUI_FONT_MEDIUM,
			Xpos, Ypos,
			GFUI_ALIGN_HL_VB, 0);

	WidthId = GfuiLabelCreate(scrHandle,
				"",
				GFUI_FONT_MEDIUM_C,
				Xpos + DX, Ypos,
				GFUI_ALIGN_HL_VB, 20);

	Ypos -= DY;

	GfuiLabelCreate(scrHandle,
			"Pits:",
			GFUI_FONT_MEDIUM,
			Xpos, Ypos,
			GFUI_ALIGN_HL_VB, 0);

	PitsId = GfuiLabelCreate(scrHandle,
				"",
				GFUI_FONT_MEDIUM_C,
				Xpos + DX, Ypos,
				GFUI_ALIGN_HL_VB, 20);

	rmUpdateTrackInfo();

	GfuiScreenActivate(scrHandle);
}
