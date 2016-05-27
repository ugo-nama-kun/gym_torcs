/***************************************************************************

    file                 : texmapper.cpp
    created              : Mon Nov 13 21:25:19 CET 2000
    copyright            : (C) 2000 by Eric Espi�
    email                : Eric.Espie@torcs.org
    version              : $Id: maintexmapper.cpp,v 1.1.2.1 2008/11/09 17:50:23 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include <unistd.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#include <math.h>
#include <plib/ssg.h>
#include <GL/glut.h>

#include <tgfclient.h>

char	*InputFileName = NULL;
char	*OutputFileName = NULL;
char	*ParamFileName = NULL;
char	*SkinFileName = NULL;
void	*ParamHandle = NULL;

FILE	*in, *out;

int	ImgSize;

ssgEntity	*Root;

typedef struct Face
{
    GF_TAILQ_ENTRY(struct Face) link;

    ssgTransform	*branch;
    char	*faceName;

    sgVec3	lbbmin;		/* Bounding (unscaled) box */
    sgVec3	lbbmax;

    sgVec3	sbbmin;		/* Bounding (scaled) box */
    sgVec3	sbbmax;

    float	lwidth;		/* logical (unscaled) values */
    float	lheight;

    sgCoord	xform;
    sgVec3	lscale;		/* object scale */
    sgVec3	offset;
    float	texScale;
    
    sgVec3	align;

    sgMat4	mat;
    
    bool	isPresent;
} tFace;

int	NbRows;
int	NbMaxCols;
float	*ColWidth;

GF_TAILQ_HEAD(RingListHead, struct Face);

typedef struct
{
    tRingListHead	faces;
    int			height;		/* Scaled height */

    float		lwidth;		/* logical (unscaled) values */
    float		lheight;
} tRow;

tRow	*Row;

extern int myssgSaveAC ( const char *filename, ssgEntity *ent, const char *skinfilename );


void load_database(void);
void save_database(void);
void set_texture_coord(void);


int BrNb = 0;

void print_mat4(char *title, sgMat4 m)
{
    int		i, j;

    printf(title);
    for (j = 0; j < 4; j ++){
	for (i = 0; i < 4; i ++) {
	    printf("%.6f  ", m[i][j]);
	}
	printf("\n");
    }
    printf("\n");
}


/*
 * Read the faces from AC3D file
 */
static ssgBranch *
hookNode(char *s)
{
    int		i;
    tFace	*curFace;
    
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	while (curFace) {
	    if (!strcmp(s, curFace->faceName)) {
		printf("Face %s found\n", s);
		curFace->branch = new ssgTransform();
		curFace->branch->setName(s);
		BrNb++;
		return (ssgBranch*)curFace->branch;
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
    }
    
    return (ssgBranch*)NULL;
}

extern char *optarg;
extern int optind, opterr, optopt;


void saveSkin(void)
{
    unsigned char	*img;
    
    img = (unsigned char*)malloc(ImgSize * ImgSize * 3);
    if (img == NULL) {
	return;
    }
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, ImgSize, ImgSize, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)img);

    GfImgWritePng(img, SkinFileName, ImgSize, ImgSize);

    free(img);
}


void draw(void)
{
    int i;
    tFace	*curFace;
    sgVec3	cam;
    

    cam[0] = cam[2] = 0;
    cam[1] = -10000;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ssgSetOrtho(ImgSize, ImgSize);
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	while (curFace) {
	    if (curFace->isPresent) {
		printf("Drawing face %s\n", curFace->faceName);
		ssgSetNearFar(MIN(curFace->sbbmin[1], curFace->sbbmax[1]) - 1, 
			      MAX(curFace->sbbmin[1], curFace->sbbmax[1]) + 1);
		ssgCullAndDraw((ssgRoot*)curFace->branch);
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
    }
    //saveSkin();
}

int DisplayCount = 0;

void
Display(void)
{
    DisplayCount++;
    switch (DisplayCount) {
    case 10:
	draw();
	break;
    case 11:
	saveSkin();
	break;
    case 12:
	set_texture_coord();
	save_database();
	break;
    case 20:
	exit(0);
	break;
    }
    glutSwapBuffers();
    glutPostRedisplay();
}


void init_graphics ()
{
    int   fake_argc = 1 ;
    char *fake_argv[3] ;
    fake_argv[0] = "TexMapper" ;
    fake_argv[1] = "Texture Auto Mapper" ;
    fake_argv[2] = NULL ;

    /*
      Initialise GLUT
    */

    glutInitWindowPosition(0, 0);
    glutInitWindowSize(ImgSize, ImgSize);
    glutInit(&fake_argc, fake_argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow(fake_argv[1]);
 
    /* Callbacks */
    glutDisplayFunc(Display);
    
    /*
      Initialise SSG
    */
    ssgInit();

    /*
      Some basic OpenGL setup
    */
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutSwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutSwapBuffers();

    load_database();

    glutMainLoop();
}

void updt_bbox(ssgEntity *start, sgVec3 min, sgVec3 max)
{
    int		i, j, k;
    float	*vert;
    
    for (i = start->getNumKids() - 1; i >= 0; i--) {
	ssgEntity *e = ((ssgBranch*)start)->getKid(i);
	if (e->getNumKids() != 0) {
	    updt_bbox(e, min, max);
	} else {
	    if (e->isAKindOf(ssgTypeLeaf())) {
		int nbvert = ((ssgLeaf*)e)->getNumVertices();
		for (j = 0; j < nbvert; j++) {
		    vert = ((ssgLeaf*)e)->getVertex(j);
		    for (k = 0; k < 3; k++) {
			if (vert[k] < min[k]) min[k] = vert[k];
			if (vert[k] > max[k]) max[k] = vert[k];
		    }
		}
	    }
	}
    }
}

/*
 * Recalculate the bounding box of the faces
 */
void calc_bbox(void)
{
    int		i, j;
    tFace	*curFace;
    ssgEntity	*e;
    sgVec3	min, max;
    int		nbcol;
    
    fprintf(stderr, "Bounding boxes:\n");
    NbMaxCols = 0;
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	nbcol = 0;
	while (curFace) {
	    nbcol++;
	    if (curFace->isPresent) {
		e = curFace->branch;
		min[0] = min[1] = min[2] = INT_MAX;
		max[0] = max[1] = max[2] = -INT_MAX;
		updt_bbox(e, min, max);
		for (j = 0; j < 3; j++) {
		    curFace->lbbmin[j] = min[j];
		    curFace->lbbmax[j] = max[j];
		}
		fprintf(stderr, "      Face %s : %f %f %f  ---  %f %f %f\n", curFace->faceName, min[0], min[1], min[2], max[0], max[1], max[2]);
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
	NbMaxCols = MAX(NbMaxCols, nbcol);
    }
}

void calc_coord(void)
{
    sgMat4	m, m2;
    int		i, j;
    tFace	*curFace;
    float	width, height;
    float	scale, offX, offY;
    int		largerRow;
    float	maxWidth;
    int		col;
    ssgBranch	*branch;

    ColWidth = (float*)calloc(NbMaxCols, sizeof(float));

    fprintf(stderr, "After Rotation:\n");
    largerRow = 0;
    maxWidth = 0;
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	while (curFace) {
	    if (curFace->isPresent) {
		branch = curFace->branch->getParent(0);
		if (branch->isAKindOf(_SSG_TYPE_BASETRANSFORM)) {
		    ((ssgBaseTransform*)branch)->getTransform(m2);
		    curFace->align[0] *= m2[3][0];
		    curFace->align[1] *= m2[3][1];
		    curFace->align[2] *= m2[3][2];
		    fprintf(stderr, "Align face %s : %f %f %f\n", curFace->faceName, curFace->align[0], curFace->align[1], curFace->align[2]);
		}
		sgMakeTransMat4(m, curFace->align);


		sgMakeIdentMat4(m2);
		for (j = 0; j < 3; j++) {
		    m2[j][j] = curFace->lscale[j];
		}
		sgPostMultMat4(m, m2);

		sgMakeRotMat4(m2, curFace->xform.hpr);
		sgPostMultMat4(m, m2);

		sgCopyMat4(curFace->mat, m);

		sgXformPnt3(curFace->sbbmin, curFace->lbbmin, m);
		sgXformPnt3(curFace->sbbmax, curFace->lbbmax, m);

		fprintf(stderr, "      Face %s : %f %f %f  ---  %f %f %f\n",
			curFace->faceName, curFace->sbbmin[0], curFace->sbbmin[1], curFace->sbbmin[2],
			curFace->sbbmax[0], curFace->sbbmax[1], curFace->sbbmax[2]);
		curFace->lwidth = 2.0 * MAX(fabs(curFace->sbbmin[0]), fabs(curFace->sbbmax[0]));
		curFace->lheight = 2.0 * MAX(fabs(curFace->sbbmin[2]), fabs(curFace->sbbmax[2]));
		//curFace->lwidth = fabs(curFace->sbbmin[0] - curFace->sbbmax[0]);
		//curFace->lheight = fabs(curFace->sbbmin[2] - curFace->sbbmax[2]);
		Row[i].lwidth += curFace->lwidth;
		Row[i].lheight = MAX(Row[i].lheight, curFace->lheight);
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
	if (Row[i].lwidth > maxWidth) {
	    maxWidth = Row[i].lwidth;
	    largerRow = i;
	}
    }

    height = 0;
    fprintf(stderr, "After Scaling:\n");
    for (i = 0; i < NbRows; i++) {
	height += Row[i].lheight;
    }
    width = maxWidth;
    scale = (float)ImgSize / MAX(width, height);

    curFace = GF_TAILQ_FIRST(&(Row[largerRow].faces));
    i = 0;
    fprintf(stderr, "Columns : ");
    while (curFace) {
	ColWidth[i] = curFace->lwidth * scale;
	fprintf(stderr, "%.2f  ", ColWidth[i]);
	curFace = GF_TAILQ_NEXT(curFace, link);
	i++;
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "Total Width = %.2f   Height = %.2f\n", width, height);
    fprintf(stderr, "Image Width = %.2f   Height = %.2f\n", width*scale, height*scale);

    offY = - (float)ImgSize / 2.0;
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	offY += Row[i].lheight*scale / 2.0;
	col = 0;
	offX = - (float)ImgSize / 2.0;
	while (curFace) {
	    if (curFace->isPresent) {
		sgCopyMat4(m, curFace->mat);
		curFace->texScale = scale;
		sgMakeIdentMat4(m2);
		for (j = 0; j < 3; j++) {
		    m2[j][j] = scale;
		}
		sgPostMultMat4(m, m2);
		sgXformPnt3(curFace->sbbmin, curFace->lbbmin, m);
		sgXformPnt3(curFace->sbbmax, curFace->lbbmax, m);
		offX += ColWidth[col] / 2.0;
		curFace->offset[0] = offX;
		curFace->offset[2] = offY;
		offX += ColWidth[col] / 2.0;
		sgMakeTransMat4(m2, curFace->offset);
		sgPostMultMat4(m, m2);
		sgCopyMat4(curFace->mat, m);

		curFace->branch->setTransform(m);
	    } else {
		offX += ColWidth[col];
	    }
	    col++;

	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
	offY += Row[i].lheight*scale / 2.0;
    }
}

void set_texcoord(ssgEntity *start, sgMat4 m)
{
    int		i;

    for (i = 0; i < start->getNumKids(); i++) {
	ssgEntity *k = ((ssgBranch*)start)->getKid(i);
	if (k->getNumKids() != 0) {
	    set_texcoord(k, m);
	} else {
	    if (k->isAKindOf(_SSG_TYPE_VTXTABLE)) {
		ssgVtxTable	*vtt = (ssgVtxTable*)k;
		int		j;
		float		*texCoord;
		float		*vtx;
		sgVec3		vtx2;
		
		for (j = 0; j < vtt->getNumVertices(); j++) {

		    vtx = vtt->getVertex(j);
		    sgXformPnt3(vtx2, vtx, m);
		    texCoord = vtt->getTexCoord(j);
		    texCoord[0] = (vtx2[0] + (float)ImgSize / 2.0) / (float)ImgSize;
		    texCoord[1] = (vtx2[2] + (float)ImgSize / 2.0) / (float)ImgSize;
		}
	    }
	}
    }
}


void set_texture_coord(void)
{
    int		i;
    tFace	*curFace;

    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	while (curFace) {
	    if (curFace->isPresent) {
		/* now apply the matrix to the vertices */
		set_texcoord(curFace->branch, curFace->mat);
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
    }
    
}


/*
  Load a simple database
*/
void load_database(void)
{

    ssgLoaderOptions *loaderopt = new ssgLoaderOptions();
    
    loaderopt->setCreateBranchCallback(hookNode);
    
    Root = ssgLoadAC(InputFileName, loaderopt);

    fprintf(stderr, "%d branches found\n", BrNb);

    calc_bbox();
    calc_coord();
}

void save_database(void)
{
    int i;
    tFace	*curFace;
    sgMat4	m;
    ssgBranch	*b = new ssgBranch();
    

    sgMakeIdentMat4(m);
    for (i = 0; i < NbRows; i++) {
	curFace = GF_TAILQ_FIRST(&(Row[i].faces));
	while (curFace) {
	    if (curFace->isPresent) {
		curFace->branch->setTransform(m);
		ssgFlatten(curFace->branch);
		ssgStripify(curFace->branch);
	    }
	    curFace = GF_TAILQ_NEXT(curFace, link);
	}
    }
    

    b->addKid(Root);
    ssgFlatten(Root);
    ssgStripify(Root);

    myssgSaveAC(OutputFileName, b, SkinFileName);
}


void load_params(void)
{
    int		i, j;
    int		nbcol;
    char	*col;
    char	buf[256];
    tFace	*curFace;
    char	*s;
    
    ImgSize  = (int)GfParmGetNum(ParamHandle, "image", "size", NULL, 256);

    NbRows = GfParmGetEltNb(ParamHandle, "faces");
    
    Row = (tRow*)calloc(NbRows, sizeof(tRow));
    GfParmListSeekFirst(ParamHandle, "faces");
    for (i = 0; i < NbRows; i++) {
	col = GfParmListGetCurEltName(ParamHandle, "faces");
	GF_TAILQ_INIT(&(Row[i].faces));
	sprintf(buf, "faces/%s/col", col);
	nbcol = GfParmGetEltNb(ParamHandle, buf);
	GfParmListSeekFirst(ParamHandle, buf);
	for (j = 0; j < nbcol; j++) {
	    curFace = (tFace*)calloc(1, sizeof(tFace));
	    GF_TAILQ_INSERT_TAIL(&(Row[i].faces), curFace, link);
	    curFace->faceName = GfParmGetCurStr(ParamHandle, buf, "face name", NULL);
	    if ((curFace->faceName != 0) && (strlen(curFace->faceName) != 0)) {
		curFace->isPresent = true;
		curFace->xform.hpr[1] =  GfParmGetCurNum(ParamHandle, buf, "rotX", NULL, 0.0);
		curFace->xform.hpr[2] = -GfParmGetCurNum(ParamHandle, buf, "rotZ", NULL, 0.0);
		curFace->xform.hpr[0] =  GfParmGetCurNum(ParamHandle, buf, "rotY", NULL, 0.0);
		curFace->lscale[0] =  GfParmGetCurNum(ParamHandle, buf, "scaleX", NULL, 1.0);
		curFace->lscale[1] =  GfParmGetCurNum(ParamHandle, buf, "scaleZ", NULL, 1.0);
		curFace->lscale[2] =  GfParmGetCurNum(ParamHandle, buf, "scaleY", NULL, 1.0);
		s = GfParmGetCurStr(ParamHandle, buf, "align", "");
		switch (s[0]) {
		case 'X':
		case 'x':
		    curFace->align[0] = 1.0;
		    break;
		case 'Y':
		case 'y':
		    curFace->align[2] = 1.0;
		    break;
		case 'Z':
		case 'z':
		    curFace->align[1] = -1.0;
		    break;
		}
	    }
	    GfParmListSeekNext(ParamHandle, buf);
	}
	GfParmListSeekNext(ParamHandle, "faces");
    }
}


void usage(void)
{
    fprintf(stderr, "Texture Auto Mapper $Revision: 1.1.2.1 $ \n");
    fprintf(stderr, "Usage: texmapper [options]\n");
    fprintf(stderr, "The options are:\n");
    fprintf(stderr, "-f <input acfile>     : input model file in AC3D format [car.ac]\n");
    fprintf(stderr, "-o <output acfile>    : ouput model file in AC3D format [car-out.ac]\n");
    fprintf(stderr, "-p <parameters>       : parameters in XML format [texmapper.xml]\n");
    fprintf(stderr, "-s <file>             : output skin file [skin.rgb]\n");
    fprintf(stderr, "--help                : print this message\n");
}

void init_args(int argc, char **argv)
{
    int		c;
    /* int		digit_optind = 0; */

    while (1) {
	/* int this_option_optind = optind ? optind : 1; */
	int option_index = 0;
	static struct option long_options[] = {
	    {"help", 0, 0, 0}
	};
	     
	c = getopt_long (argc, argv, "f:o:p:s:",
			 long_options, &option_index);
	if (c == -1)
	    break;
	     
	switch (c) {
	case 0:
	    switch (option_index) {
	    case 0:
		usage();
		exit(0);
		break;
	    default:
		usage();
		exit(1);
	    }
	    break;

	case 'f':
	    InputFileName = strdup(optarg);
	    break;
	case 'o':
	    OutputFileName = strdup(optarg);
	    break;
	case 'p':
	    ParamFileName = strdup(optarg);
	    break;
	case 's':
	    SkinFileName = strdup(optarg);
	    break;
	default:
	    usage();
	    exit(1);
	}
    }

    if (!ulFileExists(InputFileName)) {
	InputFileName = "car.ac";
	if (!ulFileExists(InputFileName)) {
	    fprintf(stderr, "The Input AC3D file must be provided\n");
	    usage();
	    exit(1);
	}
    }

    if (OutputFileName == NULL) {
	OutputFileName = "car-out.ac";
    }

    if (!ulFileExists(ParamFileName)) {
	ParamFileName = "texmapper.xml";
	if (!ulFileExists(ParamFileName)) {
	    fprintf(stderr, "The parameters file is mandatory\n");
	    usage();
	    exit(1);
	}
    }
	
    ParamHandle = GfParmReadFile(ParamFileName, GFPARM_RMODE_STD);
    if (ParamHandle == NULL) {
	fprintf(stderr, "The parameters file should not be empty\n");
	usage();
	exit(1);
    }

    if (SkinFileName == NULL) {
	SkinFileName = "skin.rgb";
    }
}


int main(int argc, char **argv)
{
    init_args(argc, argv);
    load_params();
    init_graphics();

    /* not used */
    load_database();
    draw();
    set_texture_coord();
    save_database();
    return 0;
}
