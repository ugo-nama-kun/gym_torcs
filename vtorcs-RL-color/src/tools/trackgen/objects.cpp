/***************************************************************************

    file                 : objects.cpp
    created              : Fri May 24 20:09:20 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: objects.cpp,v 1.5.2.1 2008/11/09 17:50:23 berniw Exp $

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
    @version	$Id: objects.cpp,v 1.5.2.1 2008/11/09 17:50:23 berniw Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <plib/ssg.h>
#include <GL/glut.h>

#include <tgfclient.h>
#include <track.h>

#include "trackgen.h"
#include "util.h"
#include "elevation.h"
#include "ac3d.h"
#include "objects.h"

static char		path[1024];
static char		buf[1024];

static ssgRoot	*Root = NULL;
static ssgRoot	*GroupRoot = NULL;
static ssgRoot	*TrackRoot = NULL;

struct group
{
    ssgBranch	*br;
};

static struct group	*Groups;
static float		GroupSize;
static float		XGroupOffset;
static float		YGroupOffset;
static int		XGroupNb;
static int		GroupNb;
static int		ObjUniqId = 0;

class myLoaderOptions : public ssgLoaderOptions
{
public:
  virtual void makeModelPath ( char* path, const char *fname ) const
  {
    ulFindFile ( path, model_dir, fname, NULL ) ;
  }

  virtual void makeTexturePath ( char* path, const char *fname ) const
  {
    ulFindFile ( path, texture_dir, fname, NULL ) ;
  }
} ;


typedef struct objdef
{
    GF_TAILQ_ENTRY(objdef)	link;
    int			random;
    unsigned int	color;
    ssgEntity		*obj;
    tdble		deltaHeight;
    tdble		deltaVert;
} tobjdef;

GF_TAILQ_HEAD(objlist, objdef);
tobjlist objhead;

int
GetObjectsNb(void *TrackHandle)
{
    sprintf(path, "%s/%s", TRK_SECT_TERRAIN, TRK_SECT_OBJMAP);
    return GfParmGetEltNb(TrackHandle, path);
}

static void
ApplyTransform(sgMat4 m, ssgBase *node)
{
    int		i;
    ssgBranch	*br;

    if (node->isAKindOf(ssgTypeLeaf())) {
	((ssgLeaf *)node)->transform(m);
    } else {
	br = (ssgBranch *)node;
	for (i = 0; i < br->getNumKids(); i++) {
	    ApplyTransform(m, br->getKid(i));
	}
    }
}

    
static void
InitObjects(tTrack *track, void *TrackHandle)
{
    int			objnb, i;
    struct objdef	*curObj;
    char		*objName;
    static char		*search;
    myLoaderOptions	options ;
    sgMat4		m;

    ObjUniqId = 0;
    
    srand((unsigned int)GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_SEED, NULL, 1));
    
    ssgSetCurrentOptions ( &options ) ;    

    GF_TAILQ_INIT(&objhead);

    sprintf(buf, "tracks/%s/%s;data/objects", track->category, track->internalname);
    search = strdup(buf);
    
    sprintf(path, "tracks/%s/%s;data/objects;data/textures;.", track->category, track->internalname);
    ssgTexturePath(path);
    ssgModelPath(path);

    objnb = GfParmGetEltNb(TrackHandle, TRK_SECT_OBJECTS);
    GfParmListSeekFirst(TrackHandle, TRK_SECT_OBJECTS);

    for (i = 0; i < objnb; i++) {
	curObj = (struct objdef *)malloc(sizeof(struct objdef));
	curObj->color = (unsigned int)GfParmGetCurNum(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_COLOR, NULL, 0);
	objName = GfParmGetCurStr(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_OBJECT, NULL);
	if (!objName) {
	    GfOut("Missing %s in %s/%s", TRK_ATT_OBJECT, TRK_SECT_OBJECTS, GfParmListGetCurEltName(TrackHandle, TRK_SECT_OBJECTS));
	    exit(1);
	}
	
	GetFilename(objName, search, buf);
	curObj->obj = ssgLoadAC(buf);
	ssgFlatten(curObj->obj);
	if (strcmp(GfParmGetCurStr(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_ORIENTATION_TYPE, ""), "random") == 0) {
	    curObj->deltaHeight = GfParmGetCurNum(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_DH, NULL, 0);
	    curObj->deltaVert = GfParmGetCurNum(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_DV, NULL, 5.0);
	    curObj->random = 1;
	} else {
	    curObj->random = 0;
	    sgMakeRotMat4(m, GfParmGetCurNum(TrackHandle, TRK_SECT_OBJECTS, TRK_ATT_ORIENTATION, "deg", 0), 0.0, 0.0);
	    ApplyTransform(m, curObj->obj);
	}
	GF_TAILQ_INSERT_HEAD(&objhead, curObj, link);

	GfParmListSeekNext(TrackHandle, TRK_SECT_OBJECTS);
    }
    free(search);
}

/* Prune the group tree */
static void
AddToRoot(ssgEntity *node)
{
    int		i;
    ssgBranch	*br;

    if (node->isAKindOf(ssgTypeLeaf())) {
	Root->addKid(node);
    } else {
	br = (ssgBranch *)node;
	for (i = 0; i < br->getNumKids(); i++) {
	    AddToRoot(br->getKid(i));
	}
    }
}

static void
AddObject(unsigned int clr, tdble x, tdble y)
{
    struct objdef	*curObj;
    ssgEntity		*obj;
    sgMat4		m;
    tdble		dv;
    
    for (curObj = GF_TAILQ_FIRST(&objhead); curObj; curObj = GF_TAILQ_NEXT(curObj, link)) {
	if (clr == curObj->color) {
	    obj = (ssgEntity*)curObj->obj->clone(SSG_CLONE_RECURSIVE | SSG_CLONE_GEOMETRY | SSG_CLONE_STATE);
	    if (curObj->random) {
		/* random rotations */
/* 		sgMakeCoordMat4 (m, 0.0, 0.0, curObj->deltaHeight * rand() / (RAND_MAX + 1.0), 0.0, 0.0, 0.0); */
/* 		ApplyTransform (m, obj); */
		dv = curObj->deltaVert;
		sgMakeRotMat4 (m, 360.0 * rand() / (RAND_MAX + 1.0), dv / 2.0 - dv * rand() / (RAND_MAX + 1.0), dv / 2.0  - dv * rand() / (RAND_MAX + 1.0));
		ApplyTransform (m, obj);
	    }
	    sgMakeTransMat4(m, x, y, getHOT(TrackRoot, x, y));
	    ApplyTransform(m, obj);
	    AddToRoot(obj);
	    return;
	}
    }
}


/* Code extracted from PLIB-ssg */
static const int writeTextureWithoutPath = TRUE;

struct saveTriangle
{
  int    v[3];
  sgVec2 t[3];
};

static int
ssgSaveLeaf (ssgEntity *ent, FILE *save_fd)
{
    int i;
    static sgVec3       *vlist;
    static saveTriangle *tlist;

    ssgLeaf *vt = (ssgLeaf *)ent;

    int num_verts = vt->getNumVertices();
    int num_tris  = vt->getNumTriangles();

    vlist = new sgVec3[num_verts];
    tlist = new saveTriangle[num_tris];

    for (i = 0; i < num_verts; i++) {
	sgCopyVec3 (vlist[i], vt->getVertex (i));
    }

    for (i = 0; i < num_tris; i++) {
	short vv0, vv1, vv2;

	vt->getTriangle (i, &vv0, &vv1, &vv2);

	tlist[i].v[0] = vv0;
	tlist[i].v[1] = vv1;
	tlist[i].v[2] = vv2;

	sgCopyVec2 (tlist[i].t[0], vt->getTexCoord (vv0));
	sgCopyVec2 (tlist[i].t[1], vt->getTexCoord (vv1));
	sgCopyVec2 (tlist[i].t[2], vt->getTexCoord (vv2));
    }

    fprintf (save_fd, "OBJECT poly\n");
    sprintf(buf, "obj%d", ObjUniqId++);
    fprintf (save_fd, "name \"%s\"\n", buf);

    ssgState* st = vt->getState ();

    if (st && st->isAKindOf (ssgTypeSimpleState())) {
	ssgSimpleState* ss = (ssgSimpleState*) vt->getState ();

	if (ss->isEnabled (GL_TEXTURE_2D)) {
	    const char* tfname = ss->getTextureFilename ();

	    if ((tfname != NULL) && (tfname[0] != 0)) {
		if (writeTextureWithoutPath) {
		    char *s = strrchr ((char *)tfname, '\\');

		    if (s == NULL) {
			s = strrchr ((char *)tfname, '/');
		    }

		    if (s == NULL) {
			fprintf (save_fd, "texture \"%s\"\n", tfname);
		    } else {
			fprintf (save_fd, "texture \"%s\"\n", ++s);
		    }
		} else {
		    fprintf (save_fd, "texture \"%s\"\n", tfname);
		}
	    }
	}
    }

    fprintf (save_fd, "numvert %d\n", num_verts);
  
    for (i = 0; i < num_verts; i++) {
	fprintf (save_fd, "%g %g %g\n", vlist[i][0],vlist[i][2],-vlist[i][1]);
    }

    fprintf (save_fd, "numsurf %d\n", num_tris);

    for (i = 0; i < num_tris; i++) {
	fprintf (save_fd, "SURF 0x30\n");
	fprintf (save_fd, "mat 0\n");
	fprintf (save_fd, "refs 3\n");
	fprintf (save_fd, "%d %g %g\n",
		 tlist[i].v[0],tlist[i].t[0][0],tlist[i].t[0][1]);
	fprintf (save_fd, "%d %g %g\n",
		 tlist[i].v[1],tlist[i].t[1][0],tlist[i].t[1][1]);
	fprintf (save_fd, "%d %g %g\n",
		 tlist[i].v[2],tlist[i].t[2][0],tlist[i].t[2][1]);
    } 

    fprintf (save_fd, "kids 0\n");

    delete[] vlist;
    delete   tlist;

    return TRUE;
}


static int
ssgSaveACInner (ssgEntity *ent, FILE *save_fd)
{
  /* WARNING - RECURSIVE! */

  if (ent->isAKindOf (ssgTypeBranch())) {
      ssgBranch *br = (ssgBranch *) ent;
      sprintf(buf, "objg%d", ObjUniqId++);
      Ac3dGroup (save_fd, buf, ent->getNumKids());

    for (int i = 0; i < br->getNumKids (); i++) {
	if (! ssgSaveACInner(br->getKid (i), save_fd)) {
	    return FALSE;
	}
    }

    return TRUE;
  }
 
  return ssgSaveLeaf (ent, save_fd);
}

/* insert one leaf in group */
static void
InsertInGroup(ssgEntity *ent)
{
    int			grIdx;
    struct group	*curGrp;
    float		*center;

    ent->recalcBSphere();
    center = (float*)ent->getBSphere()->getCenter();

    grIdx = (int)((center[0] - XGroupOffset) / GroupSize) + 
	XGroupNb * (int)((center[1] - YGroupOffset) / GroupSize);
    
    curGrp = &(Groups[grIdx]);

    if (curGrp->br == 0) {
	curGrp->br = new ssgBranch();
	GroupRoot->addKid(curGrp->br);
    }
    curGrp->br->addKid(ent);
}

/* insert leaves in groups */
static void
InsertInner(ssgEntity *ent)
{
  /* WARNING - RECURSIVE! */

  if (ent->isAKindOf (ssgTypeBranch())) {
      ssgBranch *br = (ssgBranch *) ent;

      for (int i = 0; i < br->getNumKids (); i++) {
	  InsertInner(br->getKid (i));
      }
      return;
  }
 
  InsertInGroup (ent);
}


static void
Group(tTrack *track, void *TrackHandle, ssgEntity *ent)
{
    tdble	Margin;
    
    if (GroupRoot) {
	delete (GroupRoot);
    }
    GroupRoot = new ssgRoot();

    Margin    = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BMARGIN, NULL, 100.0);
    GroupSize = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_GRPSZ, NULL, 100.0);
    XGroupOffset = track->min.x - Margin;
    YGroupOffset = track->min.y - Margin;

    XGroupNb = (int)((track->max.x + Margin - (track->min.x - Margin)) / GroupSize) + 1;
    
    GroupNb = XGroupNb * ((int)((track->max.y + Margin - (track->min.y - Margin)) / GroupSize) + 1);

    if (Groups) {
	free(Groups);
    }
    Groups = (struct group *)calloc(GroupNb, sizeof (struct group));

    InsertInner(Root);
}


void
GenerateObjects(tTrack *track, void *TrackHandle, void *CfgHandle, FILE *save_fd, char *meshFile)
{
    ssgLoaderOptionsEx	options;
    int			i, j;
    char		*map;
    unsigned char	*MapImage;
    int			width, height;
    tdble		xmin, xmax, ymin, ymax;
    tdble 		Margin;
    tdble		kX, kY, dX, dY;
    unsigned int	clr;
    int			index;
    char		*extName;
    FILE		*curFd;

    ssgSetCurrentOptions(&options);
    sprintf(buf, "tracks/%s/%s;data/textures;data/img;.", track->category, track->internalname);
    ssgTexturePath(buf);
    sprintf(buf, ".;tracks/%s/%s", track->category, track->internalname);
    ssgModelPath(buf);
    TrackRoot = (ssgRoot*)ssgLoadAC(meshFile);

    InitObjects(track, TrackHandle);

    Margin = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BMARGIN, NULL, 0);
    
    xmin = track->min.x - Margin;
    xmax = track->max.x + Margin;
    ymin = track->min.y - Margin;
    ymax = track->max.y + Margin;

    sprintf(path, "%s/%s", TRK_SECT_TERRAIN, TRK_SECT_OBJMAP);
    if (GfParmGetEltNb(TrackHandle, path) == 0) {
	return;
    }
    GfParmListSeekFirst(TrackHandle, path);

    index = 0;
    do {
	Root = new ssgRoot();

	index++;
	map = GfParmGetCurStr(TrackHandle, path, TRK_ATT_OBJMAP, "");
	sprintf(buf, "tracks/%s/%s/%s", track->category, track->internalname, map);

	printf("Processing object map %s\n", buf);
	MapImage = GfImgReadPng(buf, &width, &height, 2.0);
	if (!MapImage) {
	    return;
	}
    
	kX = (xmax - xmin) / width;
	dX = xmin;
	kY = (ymax - ymin) / height;
	dY = ymin;

	for (j = 0; j < height; j++) {
	    for (i = 0; i < width; i++) {
		clr = (MapImage[4 * (i + width * j)] << 16) + (MapImage[4 * (i + width * j) + 1] << 8) + MapImage[4 * (i + width * j) + 2];
		if (clr) {
		    AddObject(clr, i * kX + dX, j * kY + dY);
		}
	    }
	}
	
	Group(track, TrackHandle, Root);

	extName = GfParmGetStr(CfgHandle, "Files", "object", "obj");
	sprintf(buf, "%s-%s-%d.ac", OutputFileName, extName, index);
	curFd = Ac3dOpen(buf, 1);
	ssgSaveACInner(GroupRoot, curFd);
	Ac3dClose(curFd);
	if (save_fd) {
	    ssgSaveACInner(GroupRoot, save_fd);
	}
	delete (Root);
    } while (!GfParmListSeekNext(TrackHandle, path));
}
