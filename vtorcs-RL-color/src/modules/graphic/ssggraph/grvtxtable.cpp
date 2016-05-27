/***************************************************************************

    file                 : grvtxtable.cpp
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grvtxtable.cpp,v 1.15 2005/09/19 19:00:57 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <plib/ssg.h>
#include "grvtxtable.h"
#include "grmain.h"
#include "grcam.h"
#include "grshadow.h"
#include "grskidmarks.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif
#ifdef WIN32
#include <GL/glext.h>
#endif
#include "grscene.h"
#include "grcar.h"
#include "grutil.h"
#include <GL/glu.h>

/* from grloadac.cpp (beuark!) */
extern double shad_xmax;
extern double shad_ymax;
extern double shad_xmin;
extern double shad_ymin;


void grVtxTable::copy_from (grVtxTable *src, int clone_flags)
{
	ssgVtxTable::copy_from (src, clone_flags);
	if (src->texcoords1 != NULL && (clone_flags & SSG_CLONE_GEOMETRY)) {
		texcoords1 = (ssgTexCoordArray *)(src->texcoords1->clone(clone_flags));
	} else {
		texcoords1 = src->texcoords1;
	}

	if (src->texcoords2 != NULL && (clone_flags & SSG_CLONE_GEOMETRY)) {
		texcoords2 = (ssgTexCoordArray *)(src->texcoords2->clone(clone_flags));
	} else {
		texcoords2 = src->texcoords2;
	}

	if (src->texcoords3 != NULL && (clone_flags & SSG_CLONE_GEOMETRY)) {
		texcoords3 = (ssgTexCoordArray *)(src->texcoords3->clone(clone_flags));
	} else {
		texcoords3 = src->texcoords3;
	}

	numMapLevel = src->numMapLevel;
	mapLevelBitmap = src->mapLevelBitmap;
	internalType = src->internalType;

	if (src->internalType == ARRAY) {
		numStripes = src->numStripes;
		ssgDeRefDelete(indices);
		if (src->indices != NULL && (clone_flags & SSG_CLONE_GEOMETRY)) {
			indices = (ssgIndexArray *)(src->indices->clone(clone_flags));
		} else {
			indices = src->indices;
		}

		if (indices != NULL) {
			indices->ref();
		}

		ssgDeRefDelete(stripes);
		if (src->stripes != NULL && (clone_flags & SSG_CLONE_GEOMETRY)) {
			stripes = (ssgIndexArray *)(src->stripes->clone(clone_flags));
		} else {
			stripes = src->stripes;
		}

		if (stripes != NULL) {
			stripes->ref();
		}
	}
}


ssgBase *grVtxTable::clone (int clone_flags)
{
	grVtxTable *b = new grVtxTable(1, LEVEL0);
	b->copy_from(this, clone_flags);
	return b;
}


grVtxTable::grVtxTable (int _numMapLevel,int _mapLevel)
{
	numMapLevel = _numMapLevel;
	mapLevelBitmap = _mapLevel;
	indexCar = -1;
	texcoords1 = NULL;
	texcoords2 = NULL;
	texcoords3 = NULL;
	state1 = state2 = state3 = NULL;
	internalType = TABLE;
	numStripes = 0;
	ssgVtxTable();
}


grVtxTable::grVtxTable (GLenum ty, ssgVertexArray   *vl,
			 ssgIndexArray    * stripeIndex,
			 int _numstripes,
			 ssgIndexArray    *il,
			 ssgNormalArray   *nl,
			 ssgTexCoordArray *tl,
			 ssgTexCoordArray *tl1,
			 ssgTexCoordArray *tl2,
			 ssgTexCoordArray *tl3,
			 int _numMapLevel,
			 int _mapLevel,
			 ssgColourArray   *cl,
			 int _indexCar) : ssgVtxTable(ty, vl, nl, tl, cl)
{
	type = ssgTypeVtxTable();
	numMapLevel = _numMapLevel;
	mapLevelBitmap =_mapLevel;
	indexCar = _indexCar;
	texcoords1 = (tl1!=NULL) ? tl1 : new ssgTexCoordArray();
	texcoords2 = (tl2!=NULL) ? tl2 : new ssgTexCoordArray();
	texcoords3 = (tl3!=NULL) ? tl3 : new ssgTexCoordArray();
	texcoords1->ref();
	texcoords2->ref();
	texcoords3->ref();
	state1 = state2 = state3 = NULL;
	internalType = ARRAY;
	indices = (il != NULL) ? il : new ssgIndexArray();
	indices->ref();
	stripes = (stripeIndex!=NULL) ? stripeIndex : new ssgIndexArray();
	stripes->ref();
	numStripes = _numstripes;
}


grVtxTable::grVtxTable (GLenum ty, ssgVertexArray   *vl,
			 ssgNormalArray   *nl,
			 ssgTexCoordArray *tl,
			 ssgTexCoordArray *tl1,
			 ssgTexCoordArray *tl2,
			 ssgTexCoordArray *tl3,
			 int _numMapLevel,
			 int _mapLevel,
			 ssgColourArray   *cl,
			 int _indexCar) : ssgVtxTable(ty, vl, nl, tl, cl)
{
	type = ssgTypeVtxTable ();
	numMapLevel = _numMapLevel;
	mapLevelBitmap = _mapLevel;
	indexCar = _indexCar;
	texcoords1 = (tl1!=NULL) ? tl1 : new ssgTexCoordArray();
	texcoords2 = (tl2!=NULL) ? tl2 : new ssgTexCoordArray();
	texcoords3 = (tl3!=NULL) ? tl3 : new ssgTexCoordArray();
	texcoords1->ref();
	texcoords2->ref();
	texcoords3->ref();
	state1 = state2 = state3 = NULL;
	internalType = TABLE;
	numStripes = 0;
}


grVtxTable::~grVtxTable ()
{
	ssgDeRefDelete (texcoords1);
	ssgDeRefDelete (texcoords2);
	ssgDeRefDelete (texcoords3);

	if(internalType==ARRAY) {
		ssgDeRefDelete(indices);
		ssgDeRefDelete(stripes);
	}

	ssgDeRefDelete(state1);
	ssgDeRefDelete(state2);
	ssgDeRefDelete(state3);
}


void grVtxTable::setState1 (ssgState *st)
{
	ssgDeRefDelete (state1);

	state1 = (grMultiTexState *)st;

	if (state1 != NULL) {
		state1->ref();
	}
}


void grVtxTable::setState2 (ssgState *st)
{
	ssgDeRefDelete (state2);

	state2 = (grMultiTexState *)st;

	if (state2 != NULL) {
		state2->ref();
	}
}


void grVtxTable::setState3 (ssgState *st)
{
	ssgDeRefDelete (state3);

	state3 = (grMultiTexState *)st;

	if (state3 != NULL) {
		state3->ref();
	}
}


void grVtxTable::draw ()
{
	if (!preDraw()) {
		return;
	}

	if (hasState()) {
		getState()->apply();
	}

#ifdef _SSG_USE_DLIST
	if (dlist)
		glCallList(dlist);
	else
#endif

	if(internalType==TABLE) {
		if (mapLevelBitmap==LEVEL0 || maxTextureUnits==1) {
			ssgVtxTable::draw_geometry ();
		} else if (mapLevelBitmap < 0) {
			draw_geometry_for_a_car();
		} else {
			draw_geometry_multi();
		}
	} else {
		if (mapLevelBitmap==LEVEL0 || maxTextureUnits==1) {
			draw_geometry_array();
		} else if (mapLevelBitmap < 0) {
			draw_geometry_for_a_car_array();
		} else {
			draw_geometry_array ();
		}
	}

	if (postDrawCB != NULL) {
		(*postDrawCB)(this);
	}
}

void grVtxTable::draw_geometry_multi ()
{
	if (numMapLevel > 1) {
		state1->apply(1);
	}

	if (numMapLevel > 2) {
		state2->apply(2);
	}

	int num_colours   = getNumColours   ();
	int num_normals   = getNumNormals   ();
	int num_vertices  = getNumVertices  ();
	int num_texcoords = getNumTexCoords ();

	sgVec3 *vx = (sgVec3 *) vertices->get(0);
	sgVec3 *nm = (sgVec3 *) normals->get(0);
	sgVec2 *tx = (sgVec2 *) texcoords->get(0);
	sgVec2 *tx1 = (sgVec2 *) texcoords1->get(0);
	sgVec2 *tx2 = (sgVec2 *) texcoords2->get(0);
	sgVec4 *cl = (sgVec4 *) colours->get(0);

	TRACE_GL("draw_geometry_multi: start");
	glBegin (gltype);

	if (num_colours == 0) {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (num_colours == 1){
		glColor4fv(cl[0]);
	}

	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}

	for (int i = 0; i < num_vertices; i++) {
		if (num_normals > 1) {
			glNormal3fv(nm[i]);
		}

		if (num_texcoords > 1){
			glTexCoord2fv (tx [ i ]);
			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, tx[i]);
			if (numMapLevel > 1) {
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, tx1[i]);
			}

			if (numMapLevel > 2) {
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, tx2[i]);
			}
		}
		glVertex3fv(vx[i]);
	}
	glEnd ();

	if (numMapLevel > 1) {
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
	}

	if (numMapLevel > 2) {
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
	}

	if (maxTextureUnits > 1) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
	TRACE_GL("draw_geometry_multi: end");
}



void grVtxTable::draw_geometry_for_a_car ()
{
	tdble ttx = 0;
	tdble tty = 0;
	tdble ttz = 0;
	sgMat4 mat;
	sgVec3 axis;

	TRACE_GL("draw_geometry_for_a_car: start");

	if (mapLevelBitmap <= LEVELC2) {
		/* UP Vector for OpenGl */
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;

		glActiveTextureARB(GL_TEXTURE2_ARB);
		sgMakeRotMat4(mat, grCarInfo[indexCar].envAngle, axis);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMultMatrixf((float *)mat);
		glMatrixMode(GL_MODELVIEW);
		grEnvShadowState->apply(2);
	}

	grEnvState->apply(1);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	ttx = grCarInfo[indexCar].distFromStart/50;
	sgMakeTransMat4(mat, ttx, tty, ttz);
	glMultMatrixf((float *)mat);
	glMatrixMode(GL_MODELVIEW);

	int num_colours   = getNumColours();
	int num_normals   = getNumNormals();
	int num_vertices  = getNumVertices();
	int num_texcoords = getNumTexCoords();

	sgVec3 *vx = (sgVec3 *) vertices->get(0);
	sgVec3 *nm = (sgVec3 *) normals->get(0);
	sgVec2 *tx = (sgVec2 *) texcoords->get(0);
	sgVec2 *tx1 = (sgVec2 *) texcoords1->get(0);
	sgVec2 *tx2 = (sgVec2 *) texcoords2->get(0);
	sgVec4 *cl = (sgVec4 *) colours->get(0);

	glBegin(gltype);

	if (num_colours == 0) {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (num_colours == 1) {
		glColor4fv(cl[0]);
	}

	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}

	for (int i = 0; i < num_vertices; i++)
	{
		if (num_normals > 1) {
			glNormal3fv(nm[i]);
		}

		if (num_texcoords > 1) {
			glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, tx[i]);
			if (mapLevelBitmap <= LEVELC2) {
				glMultiTexCoord2fvARB(GL_TEXTURE2_ARB, tx2[i]);
			}
			glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, tx1[i]);
		}
		glVertex3fv(vx[i]);
	}
	glEnd ();

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	if (mapLevelBitmap <= LEVELC2) {
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
	}
	glActiveTextureARB(GL_TEXTURE0_ARB);

	TRACE_GL("draw_geometry_for_a_car: end");
}


void grVtxTable::draw_geometry_array ()
{
	int num_colours   = getNumColours();
	int num_normals   = getNumNormals();
	int num_texcoords = getNumTexCoords();

	sgVec3 *nm = (sgVec3 *) normals->get(0);
	sgVec4 *cl = (sgVec4 *) colours->get(0);

	if (numMapLevel > 1) {
		state1->apply(1);
	}

	if (numMapLevel > 2) {
		state2->apply(2);
	}

	if (maxTextureUnits > 1) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	glEnable (GL_TEXTURE_2D);

	if (num_colours == 0) {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (num_colours == 1) {
		glColor4fv(cl[0]);
	}

	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	if (num_normals > 1) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals->get(0));
	}

	if (num_texcoords > 1) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoords->get(0));

		if (numMapLevel > 1) {
			glClientActiveTextureARB(GL_TEXTURE1_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoords1->get(0));
		}

		if (numMapLevel > 2) {
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoords2->get(0));
		}

	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices->get(0));

	if (maxTextureUnits > 1) {
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	int i = 0;
	short *ii = NULL;
	int j = 0;
	int p = 0;

	for (j = 0; j < numStripes; j++) {
		i = (short)*(stripes->get(j));
		ii = indices->get(p);
		glDrawElements(gltype, i, GL_UNSIGNED_SHORT, ii);
		p += i;
	}

	glPopClientAttrib ();
	if (numMapLevel > 1) {
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
	}

	if (numMapLevel > 2) {
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
	}

	if (maxTextureUnits > 1) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
}


void grVtxTable::draw_geometry_for_a_car_array ()
{
	int num_colours   = getNumColours();
	int num_normals   = getNumNormals();
	int num_texcoords = getNumTexCoords();
	tdble ttx = 0;
	tdble tty = 0;
	tdble ttz = 0;
	sgMat4 mat;
	sgMat4 mat2;
	sgMat4 mat4;
	sgVec3 axis;

	sgVec3 *nm = (sgVec3 *) normals->get(0);
	sgVec4 *cl = (sgVec4 *) colours->get(0);

	if (mapLevelBitmap <= LEVELC2) {
		/* UP Vector for OpenGl */
		axis[0] = 0;
		axis[1] = 0;
		axis[2] = 1;

		glActiveTextureARB(GL_TEXTURE2_ARB);
		sgMakeRotMat4(mat, grCarInfo[indexCar].envAngle, axis);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMultMatrixf((float *)mat);
		glMatrixMode(GL_MODELVIEW);
		grEnvShadowState->apply(2);
	}

	if (mapLevelBitmap <= LEVELC3 && grEnvShadowStateOnCars) {
		tdble xxx = (grCarInfo[indexCar].px-shad_xmin)/(shad_xmax-shad_xmin);
		tdble yyy = (grCarInfo[indexCar].py-shad_ymin)/(shad_ymax-shad_ymin);

		/* UP Vector for OpenGl */
		axis[0]=0;
		axis[1]=0;
		axis[2]=1;

		mat2[0][0] = grCarInfo[indexCar].sx;
		mat2[0][1] = 0;
		mat2[0][2] = 0;
		mat2[0][3] = 0 ;

		mat2[1][0] = 0;
		mat2[1][1] = grCarInfo[indexCar].sy;
		mat2[1][2] = 0;
		mat2[1][3] = 0 ;

		mat2[2][0] = 0;
		mat2[2][1] = 0;
		mat2[2][2] = 1;
		mat2[2][3] = 0 ;


		mat2[3][0] = 0;
		mat2[3][1] = 0;
		mat2[3][2] = 0;
		mat2[3][3] = 1;


		glActiveTextureARB(GL_TEXTURE3_ARB);
		sgMakeRotMat4(mat, grCarInfo[indexCar].envAngle, axis);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();

		sgMakeTransMat4(mat4, xxx, yyy, 0);
		glMultMatrixf((float *)mat4);

		glMultMatrixf((float *)mat);
		glMultMatrixf((float *)mat2);

		glMatrixMode(GL_MODELVIEW);
		grEnvShadowStateOnCars->apply(3);
	}


	grEnvState->apply(1);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	ttx = grCarInfo[indexCar].distFromStart/50;
	sgMakeTransMat4(mat, ttx, tty, ttz);
	glMultMatrixf((float *)mat);
	glMatrixMode(GL_MODELVIEW);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	if (num_colours == 0) {
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (num_colours == 1) {
		glColor4fv(cl[0]);
	}

	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	if (num_normals > 1) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, normals->get(0));
	}

	if (num_texcoords > 1) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoords->get(0));

		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoords1->get(0));

		if (mapLevelBitmap <= LEVELC2) {
			glClientActiveTextureARB(GL_TEXTURE2_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoords2->get(0));
		}

		if (mapLevelBitmap <= LEVELC3) {
			glClientActiveTextureARB (GL_TEXTURE3_ARB);
			glEnableClientState (GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer (2, GL_FLOAT, 0, texcoords3->get(0));
		}
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices->get(0));

	glClientActiveTextureARB (GL_TEXTURE0_ARB);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);

	int i = 0;
	short *ii = NULL;
	int j = 0;
	int p = 0;

	for (j = 0; j < numStripes; j++) {
		i = (short)*(stripes->get(j));
		ii = indices->get(p);
		glDrawElements(gltype, i, GL_UNSIGNED_SHORT, ii);
		p += i;
	}

	glPopClientAttrib();
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable (GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	if (mapLevelBitmap <= LEVELC2) {
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_TEXTURE_2D);
	}

	if (mapLevelBitmap <= LEVELC3 && grEnvShadowStateOnCars) {
		glActiveTextureARB(GL_TEXTURE3_ARB);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_TEXTURE_2D);
	}

	glActiveTextureARB (GL_TEXTURE0_ARB);
}
