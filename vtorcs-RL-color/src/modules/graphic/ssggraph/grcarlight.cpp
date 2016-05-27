/***************************************************************************

    file                 : grcarlight.cpp
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grcarlight.cpp,v 1.8.2.3 2008/08/28 23:24:54 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <plib/ssg.h>

#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <graphic.h>
#include <robottools.h>

#include "grmain.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grsmoke.h"
#include "grcar.h"
#include "grcam.h"
#include "grscene.h"
#include "grboard.h"
#include "grssgext.h"
#include "grutil.h"
#include "grcarlight.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

void ssgVtxTableCarlight::copy_from(ssgVtxTableCarlight *src, int clone_flags)
{
	ssgVtxTable::copy_from(src, clone_flags);
	size = src->size;
	on = src->on;
	pos[0] = src->pos[0];
	pos[1] = src->pos[1];
	pos[2] = src->pos[2];
}


ssgBase *ssgVtxTableCarlight::clone(int clone_flags)
{
	ssgVtxTableCarlight *b = new ssgVtxTableCarlight;
	b->copy_from(this, clone_flags);
	return b;
}


ssgVtxTableCarlight::ssgVtxTableCarlight()
{
	sgVec3 p;
	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	ssgVtxTableCarlight(0, 0, p);
}


ssgVtxTableCarlight::ssgVtxTableCarlight(ssgVertexArray *vtx, double s, sgVec3 p)
{
	gltype = GL_TRIANGLE_STRIP;
	type = ssgTypeVtxTable();
	size = s;
	on = 1;
	pos[0] = p[0];
	pos[1] = p[1];
	pos[2] = p[2];
	vertices = (vtx!=NULL) ? vtx : new ssgVertexArray();
	normals = new ssgNormalArray();
	texcoords = new ssgTexCoordArray();
	colours = new ssgColourArray();
	vertices->ref();
	normals->ref();
	texcoords->ref();
	colours->ref();

	recalcBSphere();
}

ssgVtxTableCarlight::~ssgVtxTableCarlight ()
{
  /*  ssgDeRefDelete ( vertices  ) ;
      ssgDeRefDelete ( normals   ) ;
      ssgDeRefDelete ( texcoords ) ;
      ssgDeRefDelete ( colours   ) ; */
}



void ssgVtxTableCarlight::draw_geometry ()
{
	int num_normals = getNumNormals();
	float alpha;
	GLfloat modelView[16];
	sgVec3 A, B, C, D;
	sgVec3 right, up;
	sgVec3 axis;
	sgMat4 mat;
	sgMat4 mat3;
	sgVec3 *vx = (sgVec3 *) vertices->get(0);
	sgVec3 *nm = (sgVec3 *) normals->get(0);

	if (on == 0) {
		return;
	}

	alpha = 0.75f;
	glDepthMask(GL_FALSE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPolygonOffset(-15.0f, -20.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	// get the matrix.
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

	// get the up and right vector from the matrice view.
	up[0] = modelView[1];
	up[1] = modelView[5];
	up[2] = modelView[9];

	right[0] = modelView[0];
	right[1] = modelView[4];
	right[2] = modelView[8];

	// compute the coordinates of the four points of the quadri.

	// up and right points
	C[0] = right[0] + up[0];
	C[1] = right[1] + up[1];
	C[2] = right[2] + up[2];

	// left and up
	D[0] = -right[0] + up[0];
	D[1] = -right[1] + up[1];
	D[2] = -right[2] + up[2];

	// down and left
	A[0] = -right[0] - up[0];
	A[1] = -right[1] - up[1];
	A[2] = -right[2] - up[2];

	// right and down
	B[0] = right[0] - up[0];
	B[1] = right[1] - up[1];
	B[2] = right[2] - up[2];
	axis[0] = 0;
	axis[1] = 0;
	axis[2] = 1;

	if (maxTextureUnits > 1) {
		glActiveTextureARB (GL_TEXTURE0_ARB);
	}

	sgMakeRotMat4(mat, ((float)rand()/(float)RAND_MAX)*45, axis);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity ();
	sgMakeTransMat4(mat3, 0.5, 0.5, 0);
	glMultMatrixf((float *)mat3);
	glMultMatrixf((float *)mat);
	sgMakeTransMat4(mat3, -0.5, -0.5, 0);
	glMultMatrixf((float *)mat3);
	glMatrixMode(GL_MODELVIEW);

	glBegin(gltype) ;
	glColor4f(0.8, 0.8, 0.8, alpha);
	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}
	// the computed coordinates are translated from the smoke position with the x,y,z speed.
	glTexCoord2f(0, 0);
	glVertex3f(vx[0][0] + factor*size*A[0], vx[0][1] + factor*size*A[1], vx[0][2] + factor*size*A[2]);
	glTexCoord2f(0, 1);

	glVertex3f(vx[0][0] + factor*size*B[0], vx[0][1] + factor*size*B[1], vx[0][2] + factor*size*B[2]);
	glTexCoord2f(1, 0);

	glVertex3f(vx[0][0] + factor*size*D[0], vx[0][1] + factor*size*D[1], vx[0][2] + factor*size*D[2]);
	glTexCoord2f(1, 1);

	glVertex3f(vx[0][0]+factor*size*C[0],vx[0][1]+factor*size*C[1], vx[0][2]+factor*size*C[2]);

	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);

	if (maxTextureUnits > 1) {
		glActiveTextureARB (GL_TEXTURE0_ARB);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glDepthMask(GL_TRUE);
}


ssgSimpleState	*frontlight1 = NULL;
ssgSimpleState	*frontlight2 = NULL;
ssgSimpleState	*rearlight1 = NULL;
ssgSimpleState	*rearlight2 = NULL;
ssgSimpleState	*breaklight1 = NULL;
ssgSimpleState	*breaklight2 = NULL;

tgrCarlight * theCarslight = NULL;

ssgBranch *CarlightCleanupAnchor;


void grInitCarlight(int index)
{
	char buf[256];
	int i = 0;
	theCarslight = (tgrCarlight *)malloc(sizeof(tgrCarlight)*index);
	memset(theCarslight, 0, sizeof(tgrCarlight)*index);

	for (i = 0; i < index; i++) {
		theCarslight[i].lightAnchor= new ssgBranch();
	}

	if (!frontlight1) {
		sprintf(buf, "data/textures;data/img;.");
		frontlight1 = (ssgSimpleState*)grSsgLoadTexStateEx("frontlight1.rgb", buf, FALSE, FALSE);
		if (frontlight1 != NULL) {
			frontlight1->disable(GL_LIGHTING);
			frontlight1->enable(GL_BLEND);
			frontlight1->disable(GL_CULL_FACE);
			frontlight1->setTranslucent();
			frontlight1->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	if (!frontlight2) {
		sprintf(buf, "data/textures;data/img;.");
		frontlight2 = (ssgSimpleState*)grSsgLoadTexStateEx("frontlight2.rgb", buf, FALSE, FALSE);
		if (frontlight2 != NULL) {
			frontlight2->disable(GL_LIGHTING);
			frontlight2->enable(GL_BLEND);
			frontlight2->disable(GL_CULL_FACE);
			frontlight2->setTranslucent();
			frontlight2->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	if (!rearlight1) {
		sprintf(buf, "data/textures;data/img;.");
		rearlight1 = (ssgSimpleState*)grSsgLoadTexStateEx("rearlight1.rgb", buf, FALSE, FALSE);
		if (rearlight1 != NULL) {
			rearlight1->disable(GL_LIGHTING);
			rearlight1->enable(GL_BLEND);
			rearlight1->disable(GL_CULL_FACE);
			rearlight1->setTranslucent();
			rearlight1->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	if (!rearlight2) {
		sprintf(buf, "data/textures;data/img;.");
		rearlight2 = (ssgSimpleState*)grSsgLoadTexStateEx("rearlight2.rgb", buf, FALSE, FALSE);
		if (rearlight2 != NULL) {
			rearlight2->disable(GL_LIGHTING);
			rearlight2->enable(GL_BLEND);
			rearlight2->disable(GL_CULL_FACE);
			rearlight2->setTranslucent();
			rearlight2->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	if (!breaklight1) {
		sprintf(buf, "data/textures;data/img;.");
		breaklight1 = (ssgSimpleState*)grSsgLoadTexStateEx("breaklight1.rgb", buf, FALSE, FALSE);
		if (breaklight1 != NULL) {
			breaklight1->disable(GL_LIGHTING);
			breaklight1->enable(GL_BLEND);
			breaklight1->disable(GL_CULL_FACE);
			breaklight1->setTranslucent();
			breaklight1->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	if (!breaklight2) {
		sprintf(buf, "data/textures;data/img;.");
		breaklight2 = (ssgSimpleState*)grSsgLoadTexStateEx("breaklight2.rgb", buf, FALSE, FALSE);
		if (breaklight2 != NULL) {
			breaklight2->disable(GL_LIGHTING);
			breaklight2->enable(GL_BLEND);
			breaklight2->disable(GL_CULL_FACE);
			breaklight2->setTranslucent();
			breaklight2->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
	}

	CarlightCleanupAnchor = new ssgBranch();
}


void grShudownCarlight(void)
{
	CarlightAnchor->removeAllKids();
	CarlightCleanupAnchor->removeAllKids();
	delete CarlightCleanupAnchor;
	free(theCarslight);
	theCarslight=NULL;
}


void grAddCarlight(tCarElt *car, int type, sgVec3 pos, double size)
{
	ssgVertexArray *light_vtx = new ssgVertexArray(1);

	light_vtx->add(pos);
	theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]= new ssgVtxTableCarlight(light_vtx, size,pos);

	switch (type) {
		case LIGHT_TYPE_FRONT :
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(frontlight1);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
		case LIGHT_TYPE_FRONT2 :
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(frontlight2);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
		case LIGHT_TYPE_REAR:
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(rearlight1);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
		case LIGHT_TYPE_BRAKE:
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(breaklight1);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
		case LIGHT_TYPE_BRAKE2:
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(breaklight2);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
		case LIGHT_NO_TYPE:
		default :
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setState(rearlight1);
			theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->setCullFace(0);
			break;
	}

	theCarslight[car->index].lightType[theCarslight[car->index].numberCarlight] = type;
	theCarslight[car->index].lightCurr[theCarslight[car->index].numberCarlight] = (ssgVtxTableCarlight *)
		theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]->clone(SSG_CLONE_GEOMETRY);

	theCarslight[car->index].lightAnchor->addKid(theCarslight[car->index].lightCurr[theCarslight[car->index].numberCarlight]);
	CarlightCleanupAnchor->addKid(theCarslight[car->index].lightArray[theCarslight[car->index].numberCarlight]);
	theCarslight[car->index].numberCarlight++;
}


void grLinkCarlights(tCarElt *car)
{
	CarlightAnchor->addKid(theCarslight[car->index].lightAnchor);
}


void grUpdateCarlight(tCarElt *car,class cGrPerspCamera *curCam, int disp)
{
	int i = 0;
	sgVec3 *campos;
	sgVec3 *centerpos;
	sgVec3 * lightpos;
	ssgVtxTableCarlight	*clight;

	for (i = 0; i < theCarslight[car->index].numberCarlight; i++) {
		if (theCarslight[car->index].lightAnchor->getNumKids() != 0) {
			theCarslight[car->index].lightAnchor->removeKid(theCarslight[car->index].lightCurr[i]);
		}
	}

	campos = curCam->getPosv();
	centerpos = curCam->getCenterv();

	for (i = 0; i < theCarslight[car->index].numberCarlight; i++) {
		if (!disp) {
			continue;
		}

		clight = (ssgVtxTableCarlight *)theCarslight[car->index].lightArray[i]->clone(SSG_CLONE_GEOMETRY);
		clight->setCullFace(0);

		clight->transform(grCarInfo[car->index].carPos);
		theCarslight[car->index].lightCurr[i]=clight;
		theCarslight[car->index].lightAnchor->addKid(clight);
		lightpos=clight->getPos();

		switch (theCarslight[car->index].lightType[i]) {
			case LIGHT_TYPE_BRAKE:
			case LIGHT_TYPE_BRAKE2:
				if (car->_brakeCmd>0) {
					clight->setOnOff(1);
				} else {
					clight->setOnOff(0);
				}
				break;
			case LIGHT_TYPE_FRONT:
				if (car->_lightCmd & RM_LIGHT_HEAD1) {
					clight->setOnOff(1);
				} else {
					clight->setOnOff(0);
				}
				break;
			case LIGHT_TYPE_FRONT2:
				if (car->_lightCmd & RM_LIGHT_HEAD2) {
					clight->setOnOff(1);
				} else {
					clight->setOnOff(0);
				}
				break;
			case LIGHT_TYPE_REAR:
				if ((car->_lightCmd & RM_LIGHT_HEAD1) ||
					(car->_lightCmd & RM_LIGHT_HEAD2)) {
					clight->setOnOff(1);
				} else {
					clight->setOnOff(0);
				}
				break;
			default:
				break;
		}
		clight->setFactor(1);
	}
}
