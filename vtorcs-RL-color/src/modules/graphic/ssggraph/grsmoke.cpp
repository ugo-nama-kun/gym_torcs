/***************************************************************************

    file                 : grsmoke.cpp
    created              : Fri Mar 22 23:17:54 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grsmoke.cpp,v 1.27 2005/07/21 21:27:15 berniw Exp $

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

#ifdef DMALLOC
#include "dmalloc.h"
#endif

int grSmokeMaxNumber;
double grSmokeDeltaT;
double grFireDeltaT;
double grSmokeLife;


static tgrSmokeManager *smokeManager = 0;
/** initialize the smoke structure */
ssgSimpleState *mst = NULL;
ssgSimpleState *mstf0 = NULL;
ssgSimpleState *mstf1 = NULL;
double * timeSmoke = 0;
double * timeFire = 0;



void grInitSmoke(int index)
{
    char buf[256];

    grSmokeMaxNumber = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB,
										 (char*)NULL, MAX_SMOKE_NUMBER);
    grSmokeDeltaT = (double)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKEDELTAT,
										 (char*)NULL, DELTAT);
    grSmokeLife = (double)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKEDLIFE,
									   (char*)NULL, MAX_SMOKE_LIFE);

    if (!grSmokeMaxNumber) {
		return;
    }

    grFireDeltaT=grSmokeDeltaT*8;

    if (!timeSmoke) {
		timeSmoke = (double *) malloc(sizeof(double)*index*4);
		memset(timeSmoke,0,sizeof(double)*index*4);
    }

    if (!timeFire) {
		timeFire = (double *) malloc(sizeof(double)*index);
		memset(timeFire,0,sizeof(double)*index);
    }

    if (!smokeManager) {
		smokeManager = (tgrSmokeManager*) malloc(sizeof(tgrSmokeManager));
		smokeManager->smokeList = NULL;
		smokeManager->number = 0;
    }

    // add temp object to get a reference on the states
    if (!mst) {
		sprintf(buf, "data/textures;data/img;.");
		mst = (ssgSimpleState*)grSsgLoadTexStateEx("smoke.rgb", buf, FALSE, FALSE);
		if (mst!=NULL) {
			mst->disable(GL_LIGHTING);
			mst->enable(GL_BLEND);
			mst->disable(GL_CULL_FACE);
			mst->setTranslucent();
			mst->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
    }

    if (!mstf0) {
		sprintf(buf, "data/textures;data/img;.");
		mstf0 = (ssgSimpleState*)grSsgLoadTexStateEx("fire0.rgb", buf, FALSE, FALSE);
		if (mst!=NULL) {
			mstf0->disable(GL_LIGHTING);
			mstf0->enable(GL_BLEND);
			mstf0->disable(GL_CULL_FACE);
			mstf0->setTranslucent();
			mstf0->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
    }

    if (!mstf1) {
		sprintf(buf, "data/textures;data/img;.");
		mstf1 = (ssgSimpleState*)grSsgLoadTexStateEx("fire1.rgb", buf, FALSE, FALSE);
		if (mst!=NULL) {
			mstf1->disable(GL_LIGHTING);
			mstf1->enable(GL_BLEND);
			mstf1->disable(GL_CULL_FACE);
			mstf1->setTranslucent();
			mstf1->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
		}
    }
}


/** update if necessary the smoke for a car */
void grUpdateSmoke(double t)
{
	tgrSmoke * tmp, *tmp2;
	tgrSmoke * prev;

	if (!grSmokeMaxNumber) {
		return;
	}

	prev = NULL;
	tmp = smokeManager->smokeList;
	while( tmp != NULL) {
		if (tmp->smoke->cur_life >= tmp->smoke->max_life) {
			if (prev) {
				prev->next = tmp->next;
			} else {
				smokeManager->smokeList = tmp->next;
			}

			smokeManager->number--;

			SmokeAnchor->removeKid(tmp->smoke);
			tmp2 = tmp;
			tmp = tmp->next;
			free(tmp2);
			continue;
		}

		// update the smoke
		tmp->smoke->dt = t-tmp->smoke->lastTime;
		// expand the Y value
		tmp->smoke->sizey += tmp->smoke->dt*tmp->smoke->vexp*2.0f;
		tmp->smoke->sizez += tmp->smoke->dt*tmp->smoke->vexp*0.25f;
		tmp->smoke->sizex += tmp->smoke->dt*tmp->smoke->vexp*2.0f;

		if (tmp->smoke->smokeType == SMOKE_TYPE_ENGINE) {
			if (tmp->smoke->smokeTypeStep == 0) {
				if (tmp->smoke->cur_life >= tmp->smoke->step0_max_life) {
					// changing from fire to smoke
					tmp->smoke->smokeTypeStep = 1;
					tmp->smoke->setState(mstf1);
				} else if (tmp->smoke->smokeTypeStep == 1) {
					if (tmp->smoke->cur_life >= tmp->smoke->step1_max_life) {
						// changing from fire to smoke
						tmp->smoke->smokeTypeStep = 2;
						tmp->smoke->setState(mst);
					}
				}
			}
		}

		sgVec3 *vx = (sgVec3 *) tmp->smoke->getVertices()->get(0) ;

		tdble dt = tmp->smoke->dt;

		tdble damp = 0.2f;
		tmp->smoke->vvx -= damp*tmp->smoke->vvx*fabs(tmp->smoke->vvx) * dt;
		tmp->smoke->vvy -= damp*tmp->smoke->vvy*fabs(tmp->smoke->vvy) * dt;
		tmp->smoke->vvz -= damp*tmp->smoke->vvz*fabs(tmp->smoke->vvz) * dt;
		tmp->smoke->vvz += 0.0001f;

		vx[0][0] += tmp->smoke->vvx * dt;
		vx[0][1] += tmp->smoke->vvy * dt;
		vx[0][2] += tmp->smoke->vvz * dt;

		tmp->smoke->lastTime = t;
		tmp->smoke->cur_life += tmp->smoke->dt;
		prev = tmp;
		tmp = tmp->next;
	}
}


void grAddSmoke(tCarElt *car, double t)
{
	int i = 0;
	tgrSmoke * tmp;
	sgVec3 vtx;
	ssgVertexArray *shd_vtx ;
	tgrCarInstrument *curInst;
	tdble val;
	tdble spd2;
	int index;

	if (!grSmokeMaxNumber) {
		return;
	}

	spd2 = car->_speed_x * car->_speed_x + car->_speed_y * car->_speed_y;

	for (i = 0; i < 4; i++) {
		if (spd2 > 0.001f) {
			if (smokeManager->number < grSmokeMaxNumber) {
				if ((t - timeSmoke[car->index*4+i]) < grSmokeDeltaT) {
					continue;
				} else {
					timeSmoke[car->index*4+i] = t;
				}

				sgVec3 cur_clr;
				tdble init_speed;
				tdble threshold = 0.1f;
				tdble smoke_life_coefficient = 30.0f;
				tdble smoke_speed_coefficient = 0.0f;

				cur_clr[0] = 0.8f;
				cur_clr[1] = 0.8f;
				cur_clr[2] = 0.8f;
				init_speed = 0.01f;

				if (car->priv.wheel[i].seg) { // sanity check
					char* surface = car->priv.wheel[i].seg->surface->material;
					if (strstr(surface, "sand")) {
						cur_clr[0] = 0.8f;
						cur_clr[1] = 0.7f + urandom()*0.1f;
						cur_clr[2] = 0.4f + urandom()*0.2f;
						init_speed = 0.5f;
						threshold = 0.05f;
						smoke_life_coefficient = 12.5f;
						smoke_speed_coefficient = 0.25f;
					} else if (strstr(surface, "dirt")) {
						cur_clr[0] = 0.7f + urandom()*0.1f;
						cur_clr[1] = 0.6f + urandom()*0.1f;
						cur_clr[2] = 0.5f + urandom()*0.1f;
						init_speed = 0.45f;
						threshold=0.0f;
						smoke_life_coefficient = 10.0f;
						smoke_speed_coefficient = 0.5f;
					} else if (strstr(surface,"mud")) {
						cur_clr[0] = 0.65f;
						cur_clr[1] = 0.4f + urandom()*0.2f;
						cur_clr[2] = 0.3f + urandom()*0.2f;
						init_speed = 0.4f;
						threshold = 0.2f;
						smoke_speed_coefficient = 0.05f;
					} else if (strstr(surface,"gravel")) {
						cur_clr[0] = 0.6f;
						cur_clr[1] = 0.6f;
						cur_clr[2] = 0.6f;
						init_speed = 0.35f;
						smoke_life_coefficient = 20.0f;
						smoke_speed_coefficient = 0.1f;
					} else if (strstr(surface,"grass")) {
						cur_clr[0] = 0.4f + urandom()*0.2f;
						cur_clr[1] = 0.5f + urandom()*0.1f;
						cur_clr[2] = 0.3f + urandom()*0.1f;
						init_speed = 0.3f;
						smoke_life_coefficient = 25.0f;
					} else {
						cur_clr[0] = 0.8f;
						cur_clr[1] = 0.8f;
						cur_clr[2] = 0.8f;
						init_speed = 0.01f;
					}
				}

				smoke_life_coefficient = smoke_life_coefficient * (1.0f - urandom()*urandom());
				tdble spd_fx=tanh(0.001f*car->_reaction[i])*smoke_speed_coefficient*sqrt(spd2);
				if (car->_skid[i] + 0.025f*urandom()*spd_fx>urandom() + threshold) {// instead of 0.3, to randomize

					float init_speed_z = 0.1f;
					float stretch_factor = 0.5f;
					tdble sinCarYaw = sin(car->_yaw);
					tdble cosCarYaw = cos(car->_yaw);

					shd_vtx = new ssgVertexArray(1);
					//shd_clr = new ssgColourArray(1);

					tmp = (tgrSmoke *) malloc(sizeof(tgrSmoke));
					vtx[0] = car->priv.wheel[i].relPos.x;
					vtx[1] = car->priv.wheel[i].relPos.y;
					vtx[2] = car->priv.wheel[i].relPos.z-car->_wheelRadius(i)*1.0f+ 0.5f * SMOKE_INIT_SIZE;
					tdble stretchX = 0.1f * (spd_fx + stretch_factor * fabs(car->_speed_X));
					tdble stretchY = 0.1f * (spd_fx + stretch_factor * fabs(car->_speed_Y));
					vtx[0] -= 0.05f*car->_speed_x;

					shd_vtx->add(vtx);
					tmp->smoke = new ssgVtxTableSmoke(shd_vtx,SMOKE_INIT_SIZE,SMOKE_TYPE_TIRE);
					init_speed = urandom()*init_speed;

					tmp->smoke->vvx = -sinCarYaw * car->_wheelSlipSide(i);
					tmp->smoke->vvy = cosCarYaw * car->_wheelSlipSide(i);
					tmp->smoke->vvx += cosCarYaw * car->_wheelSlipAccel(i);
					tmp->smoke->vvy += sinCarYaw * car->_wheelSlipAccel(i);

					tmp->smoke->vvz = init_speed_z;

					tmp->smoke->vvx *= init_speed;
					tmp->smoke->vvy *= init_speed;
					tmp->smoke->setState(mst);
					tmp->smoke->setCullFace(0);

					//printf("%f\n", car->_reaction[i]);
					tmp->smoke->max_life = grSmokeLife *
						(car->_skid[i]*sqrt(spd2)+urandom()*spd_fx)/ smoke_life_coefficient;
					for (int c = 0; c < 3; c++) {
						tmp->smoke->cur_col[c] = cur_clr[c];
					}

					tmp->smoke->cur_life = 0;
					tmp->smoke->sizex = VX_INIT + stretchX;
					tmp->smoke->sizey = VY_INIT + stretchY;
					tmp->smoke->sizez = VZ_INIT + 0.1f * spd_fx;

					tmp->smoke->init_alpha = 1.0/(1.0+0.1*spd_fx);
					tmp->smoke->vexp = V_EXPANSION+(car->_skid[i]+.1*spd_fx)*(((float)rand()/(float)RAND_MAX));
					tmp->smoke->smokeType = SMOKE_TYPE_TIRE;
					tmp->smoke->smokeTypeStep = 0;
					tmp->next = NULL;
					tmp->smoke->lastTime = t;
					tmp->smoke->transform(grCarInfo[car->index].carPos);
					SmokeAnchor->addKid(tmp->smoke);
					smokeManager->number++;
					if (smokeManager->smokeList==NULL) {
						smokeManager->smokeList = tmp;
					} else {
						tmp->next = smokeManager->smokeList;
						smokeManager->smokeList = tmp;
					}
				}
			}
		}
    }

    if (car->_exhaustNb && (spd2 > 10.0)) {
		if (smokeManager->number < grSmokeMaxNumber) {
			index = car->index;	/* current car's index */
			if ((t - timeFire[index]) > grFireDeltaT) {
				timeFire[index] = t;
				curInst = &(grCarInfo[index].instrument[0]);
				val = ((curInst->rawPrev - curInst->minValue) / curInst->maxValue) - ((*(curInst->monitored) - curInst->minValue) / curInst->maxValue);
				curInst->rawPrev = *(curInst->monitored);
				if (val > 0.1) {
					grCarInfo[index].fireCount = (int)(10.0 * val * car->_exhaustPower);
				}

				if (grCarInfo[index].fireCount) {
					grCarInfo[index].fireCount--;
				//if (car->priv.smoke>urandom()) {
			
					//car->priv.smoke = val * car->_exhaustPower;
					for (i = 0; i < car->_exhaustNb; i++) {
						shd_vtx = new ssgVertexArray(1);
						tmp = (tgrSmoke *) malloc(sizeof(tgrSmoke));
						vtx[0] = car->_exhaustPos[i].x;
						vtx[1] = car->_exhaustPos[i].y;
						vtx[2] = car->_exhaustPos[i].z;
		    
						shd_vtx->add(vtx);
						tmp->smoke = new ssgVtxTableSmoke(shd_vtx,SMOKE_INIT_SIZE*4,SMOKE_TYPE_ENGINE);

						tmp->smoke->setState(mstf0);
						tmp->smoke->setCullFace(0);
						tmp->smoke->max_life = grSmokeLife/8;
						tmp->smoke->step0_max_life =  (grSmokeLife)/50.0;
						tmp->smoke->step1_max_life =  (grSmokeLife)/50.0+ tmp->smoke->max_life/2.0;
						tmp->smoke->cur_life = 0;
						//tmp->smoke->init_alpha = 0.9;
						tmp->smoke->sizex = VX_INIT*4;
						tmp->smoke->sizey = VY_INIT*4;
						tmp->smoke->sizez = VZ_INIT*4;
						tmp->smoke->vexp = V_EXPANSION+5.0*rand()/(RAND_MAX+1.0) * car->_exhaustPower / 2.0;
						//tmp->smoke->vexp = V_EXPANSION+5.0*(((float)rand()/(float)RAND_MAX)) * car->_exhaustPower / 2.0;
						tmp->smoke->smokeType = SMOKE_TYPE_ENGINE;
						tmp->smoke->smokeTypeStep = 0;
						tmp->next = NULL;
						tmp->smoke->lastTime = t;
						tmp->smoke->transform(grCarInfo[index].carPos);
						SmokeAnchor->addKid(tmp->smoke);
						smokeManager->number++;
						if (smokeManager->smokeList==NULL) {
							smokeManager->smokeList = tmp;
						} else {
							tmp->next = smokeManager->smokeList;
							smokeManager->smokeList = tmp;
						}
					}
				}
			}
		}
    }
}

/** remove the smoke information for a car */
void grShutdownSmoke ()
{
    tgrSmoke *tmp, *tmp2;

    GfOut("-- grShutdownSmoke\n");

    if (!grSmokeMaxNumber) {
		return;
    }

    SmokeAnchor->removeAllKids();
    if (smokeManager) {
		tmp = smokeManager->smokeList;
		while( tmp!=NULL)
			{
				tmp2 = tmp->next;
				/* SmokeAnchor->removeKid(tmp->smoke); */
				free(tmp);
				tmp = tmp2;
			}
		smokeManager->smokeList = NULL;
		free(timeSmoke);
		free(timeFire);
		free(smokeManager);
		smokeManager = 0;
		smokeManager = NULL;
		timeSmoke = NULL;
		timeFire=NULL;
    }
}

void ssgVtxTableSmoke::copy_from ( ssgVtxTableSmoke *src, int clone_flags )
{
    ssgVtxTable::copy_from ( src, clone_flags ) ;
}
ssgBase *ssgVtxTableSmoke::clone ( int clone_flags )
{
    ssgVtxTableSmoke *b = new ssgVtxTableSmoke ;
    b -> copy_from ( this, clone_flags ) ;
    return b ;
}
ssgVtxTableSmoke::ssgVtxTableSmoke ()
{
    ssgVtxTable();
}
ssgVtxTableSmoke:: ssgVtxTableSmoke (ssgVertexArray	*shd_vtx , float initsize, int typ)
{
    sizex = sizey = sizez = initsize;
	
    gltype = GL_TRIANGLE_STRIP;
    type = ssgTypeVtxTable () ;
    stype = typ;
    vertices  = (shd_vtx!=NULL) ? shd_vtx : new ssgVertexArray   () ;
    normals   =  new ssgNormalArray   () ;
    texcoords =  new ssgTexCoordArray () ;
    colours   =  new ssgColourArray   () ;

    vertices  -> ref () ;
    normals   -> ref () ;
    texcoords -> ref () ;
    colours   -> ref () ;
	cur_col[0] = cur_col[1] = cur_col[2] = 0.8;
	vvx = vvy = vvz = 0.0;
	init_alpha = 0.9;
    recalcBSphere () ;
}

ssgVtxTableSmoke::~ssgVtxTableSmoke ()
{
}

void ssgVtxTableSmoke::draw_geometry ()
{
	int num_colours = getNumColours();
	int num_normals = getNumNormals();
	float alpha;
	GLfloat modelView[16];
	sgVec3 A, B, C, D;
	sgVec3 right, up, offset;

	sgVec3 *vx = (sgVec3 *) vertices->get(0);
	sgVec3 *nm = (sgVec3 *) normals->get(0);
    sgVec4 *cl = (sgVec4 *) colours->get(0);
	alpha =  0.9f - ((float)(cur_life/max_life));
	glDepthMask(GL_FALSE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/*glPolygonOffset(-5.0f, +10.0f);*/
	/*glEnable(GL_POLYGON_OFFSET_FILL);*/

	// the principle is to have a right and up vector
	// to determine how the points of the quadri should be placed
	// orthogonaly to the view, parallel to the screen.

	/* get the matrix */
	// TODO: replace that, glGet stalls rendering pipeline (forces flush).
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

	// get the up and right vector from the matrice view

	offset[0] = offset[1] = offset[2] = 0.0f;

	int i;
	for (i = 0; i < 3; i++) {
		int j = i;
		int k;
		for (k = 0; k < 4; k++, j+=4) {
			if (k != 3) {
				offset[i] += modelView[j] * vx[0][k];
			} else {
				offset[i] += modelView[j];
			}
		}
	}
	//printf ("%f %f %f\n", offset[0], offset[1], offset[2]);

	tdble dist = sqrt(offset[0]*offset[0]
		+ offset[1]*offset[1]
		+ offset[2]*offset[2]);

	up[0] = modelView[1];
	up[1] = modelView[5];
	up[2] = modelView[9];

	right[0] = modelView[0];
	right[1] = modelView[4];
	right[2] = modelView[8];

	// compute the coordinates of the four points of the quadri.

	// up and right points
	C[0] = right[0]+up[0];
	C[1] = right[1]+up[1];
	C[2] = right[2]+up[2];

	// left and up
	D[0] = -right[0]+up[0];
	D[1] = -right[1]+up[1];
	D[2] = -right[2]+up[2];

	// down and left
	A[0] = -right[0]-up[0];
	A[1] = -right[1]-up[1];
	A[2] = -right[2]-up[2];

	// right and down
	B[0] = right[0]-up[0];
	B[1] = right[1]-up[1];
	B[2] = right[2]-up[2];

	glBegin ( gltype ) ;

	if (dist < 50.0f) {
		alpha *= (1.0f - exp(-0.1f * dist));
	}

	glColor4f(cur_col[0],cur_col[1],cur_col[2],alpha);
	if (num_colours == 1) {
		glColor4fv(cl[0]);
	}
	if (num_normals == 1) {
		glNormal3fv(nm[0]);
	}

	// the computed coordinates are translated from the smoke position with the x, y, z speed
	glTexCoord2f(0,0);
	glVertex3f(vx[0][0]+sizex*A[0],vx[0][1]+sizey*A[1], vx[0][2]+sizez*A[2]);
	glTexCoord2f(0,1);
	glVertex3f(vx[0][0]+sizex*B[0],vx[0][1]+sizey*B[1], vx[0][2]+sizez*B[2]);
	glTexCoord2f(1,0);
	glVertex3f(vx[0][0]+sizex*D[0],vx[0][1]+sizey*D[1], vx[0][2]+sizez*D[2]);
	glTexCoord2f(1,1);
	glVertex3f(vx[0][0]+sizex*C[0],vx[0][1]+sizey*C[1], vx[0][2]+sizez*C[2]);
	glEnd();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
}
