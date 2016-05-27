/***************************************************************************

    file                 : grskidmarks.cpp
    created              : Fri Mar 22 23:17:24 CET 2002
    copyright            : (C) 2001-2005 by Christophe Guionneau
                                            Christos Dimitrakakis
                                            Bernhard Wymann
    version              : $Id: grskidmarks.cpp,v 1.30 2005/12/25 01:22:06 olethros Exp $

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
#include "grcar.h"
#include "grcam.h"
#include "grscene.h"
#include "grboard.h"
#include "grssgext.h"
#include "grutil.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

static ssgSimpleState *skidState = NULL;

int grSkidMaxStripByWheel;
int grSkidMaxPointByStrip;
double grSkidDeltaT;

/** initialize the skidmak structure for a car*/
void grInitSkidmarks(tCarElt *car)
{
	int i;
	int k;
	ssgNormalArray *shd_nrm;
	sgVec3 nrm;

	grSkidMaxStripByWheel = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL,
							(char*)NULL, MAXSTRIP_BYWHEEL);
	grSkidMaxPointByStrip = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXPOINTBYSTRIP,
							(char*)NULL, MAXPOINT_BY_STRIP);
	grSkidDeltaT = (double)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKIDDELTAT,
					(char*)NULL, DELTATSTRIP);

	if (!grSkidMaxStripByWheel) {
		return;
	}

	shd_nrm = new ssgNormalArray(1);
	nrm[0] = nrm[1] = 0.0;
	nrm[2] = 1.0;
	shd_nrm->add(nrm);

	if (skidState == NULL) {
		skidState = new ssgSimpleState();
		if (skidState) {
			skidState->disable(GL_LIGHTING);
			skidState->enable(GL_BLEND);
			skidState->enable(GL_CULL_FACE);
			// add texture
			skidState->enable(GL_TEXTURE_2D);
			skidState->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);
			skidState->setTexture ("data/textures/grey-tracks.rgb", TRUE, TRUE, TRUE);
		}
	}

	grCarInfo[car->index].skidmarks = (tgrSkidmarks *)malloc(sizeof(tgrSkidmarks));
	for (i = 0; i<4; i++) {
		grCarInfo[car->index].skidmarks->strips[i].vtx   = (ssgVertexArray **)malloc(grSkidMaxStripByWheel * sizeof(ssgVertexArray *));
		grCarInfo[car->index].skidmarks->strips[i].tex   = (ssgTexCoordArray **)malloc(grSkidMaxStripByWheel * sizeof(ssgTexCoordArray *));

		grCarInfo[car->index].skidmarks->strips[i].vta   = (ssgVtxTableShadow **)malloc(grSkidMaxStripByWheel
											* sizeof(ssgVtxTableShadow *));
		grCarInfo[car->index].skidmarks->strips[i].clr   = ( ssgColourArray **)malloc(grSkidMaxStripByWheel
											* sizeof( ssgColourArray *));

		grCarInfo[car->index].skidmarks->strips[i].smooth_colour[0]=0.0;
		grCarInfo[car->index].skidmarks->strips[i].smooth_colour[1]=0.0;
		grCarInfo[car->index].skidmarks->strips[i].smooth_colour[2]=0.0;
		grCarInfo[car->index].skidmarks->strips[i].smooth_colour[3]=0.0;

		grCarInfo[car->index].skidmarks->strips[i].state = (int *)malloc(grSkidMaxStripByWheel * sizeof(int));
		grCarInfo[car->index].skidmarks->strips[i].size  = (int *)malloc(grSkidMaxStripByWheel * sizeof(int));

		for (k = 0; k < grSkidMaxStripByWheel; k++) {
			grCarInfo[car->index].skidmarks->strips[i].state[k] = SKID_UNUSED;
			grCarInfo[car->index].skidmarks->strips[i].vtx[k] = new ssgVertexArray(grSkidMaxPointByStrip + 1);
			grCarInfo[car->index].skidmarks->strips[i].tex[k] = new ssgTexCoordArray(grSkidMaxPointByStrip + 1);
			grCarInfo[car->index].skidmarks->strips[i].clr[k] = new ssgColourArray(grSkidMaxPointByStrip + 1);

			grCarInfo[car->index].skidmarks->strips[i].vta[k] =
				new ssgVtxTableShadow
				(GL_TRIANGLE_STRIP,
				grCarInfo[car->index].skidmarks->strips[i].vtx[k],
				shd_nrm,
				grCarInfo[car->index].skidmarks->strips[i].tex[k],
				grCarInfo[car->index].skidmarks->strips[i].clr[k]
				);
			grCarInfo[car->index].skidmarks->strips[i].vta[k]->setCullFace(0);
			grCarInfo[car->index].skidmarks->strips[i].vta[k]->setState(skidState);
			grCarInfo[car->index].skidmarks->strips[i].timeStrip = 0;
			SkidAnchor->addKid(grCarInfo[car->index].skidmarks->strips[i].vta[k]);
		}
		// no skid is in use
		grCarInfo[car->index].skidmarks->strips[i].running_skid = 0;
		// the next skid to used is the first one
		grCarInfo[car->index].skidmarks->strips[i].next_skid = 0;
		// there was no skid for this wheel during the next shot
		grCarInfo[car->index].skidmarks->strips[i].last_state_of_skid = 0;
		grCarInfo[car->index].skidmarks->strips[i].tex_state = 0.0;
	}
}


/** update if necessary the skidmarks for a car */
void grUpdateSkidmarks(tCarElt *car, double t)
{
	int i = 0;
	sgVec3 vtx;
	sgVec3 *tvtx;
	sgVec2 *tclist;
	sgVec2 TxVtx;
	ssgVertexArray *basevtx = NULL;
	sgVec4 cur_clr;
	ssgTexCoordArray* texcoords = NULL;
	tdble skid_sensitivity = 0.75f;

	if (!grSkidMaxStripByWheel) {
		return;
	}

	cur_clr[0] = cur_clr[1] = cur_clr[2] = 1.0f;

	for (i = 0; i < 4; i++) {

		tdble sling_mud = 1.0f;

		if (car->priv.wheel[i].seg) { // sanity check
			char* s = car->priv.wheel[i].seg->surface->material;
			if (strstr(s, "sand")) {
				cur_clr[0] = 0.8f;
				cur_clr[1] = 0.6f;
				cur_clr[2] = 0.35f;
				skid_sensitivity = 0.9f;
			} else if (strstr(s, "dirt")) {
				cur_clr[0] = 0.7f;
				cur_clr[1] = 0.55f;
				cur_clr[2] = 0.45f;
				skid_sensitivity = 0.9f;
			} else if (strstr(s,"mud")) {
				cur_clr[0] = 0.5f;
				cur_clr[1] = 0.35f;
				cur_clr[2] = 0.15f;
				skid_sensitivity = 1.0f;
			} else if (strstr(s,"grass")) {
				cur_clr[0] = 0.75f;
				cur_clr[1] = 0.5f;
				cur_clr[2] = 0.3f;
				skid_sensitivity = 0.8f;
			} else if (strstr(s,"gravel")) {
				cur_clr[0] = 0.6f;
				cur_clr[1] = 0.6f;
				cur_clr[2] = 0.6f;
				skid_sensitivity = 0.7f;
			} else {
				sling_mud=0.0;
				cur_clr[0] = 0.0f;
				cur_clr[1] = 0.0f;
				cur_clr[2] = 0.0f;
				skid_sensitivity = 0.5f;
			}
		}

		if (car->_skid[i] > 0.1f) {
			cur_clr[3] = tanh(skid_sensitivity*car->_skid[i]);
		} else {
			cur_clr[3] = 0.0f;
		}

		for (int c = 0; c < 3; c++) {
			tdble tmp = grCarInfo[car->index].skidmarks->strips[i].smooth_colour[c];
			grCarInfo[car->index].skidmarks->strips[i].smooth_colour[c] = 0.9f*tmp + 0.1f*cur_clr[c];
			cur_clr[c] = tmp;
		}

		if ((t - grCarInfo[car->index].skidmarks->strips[i].timeStrip) < grSkidDeltaT) {
			continue;
		}

		if ((car->_speed_x * car->_speed_x + car->_speed_y * car->_speed_y) > 1.0f) {
	    	if (cur_clr[3] > 0.1f) {
                
				basevtx = new ssgVertexArray(4 * 2 + 1);
				tdble sling_left = 0.0f;
				tdble sling_right = 0.0f;                
				sling_right = sling_mud;
				sling_left = -sling_mud;

                // TO-DO: Temporary fix, trying to make sure that
                // skids are above the road surface. This is needed
                // because the wheel position depends on the current
                // physical model road height, which is not exactly
                // the same as the gfx road height, even if it is so
                // on average, because the physical model adds some
                // sinewave to the road height to simulate uneveness
                // of the track.  A better fix would be to add a
                // routine grTrackHeightL(tTrkLocPos *p), similar to 
                // TrTrackHeightL(), but which aim to give the height
                // of the graphical track.
                tdble z_adjust = 0.95;
                tdble contact_z = car->priv.wheel[i].relPos.z - car->_wheelRadius(i)*z_adjust; 


				vtx[0] = car->priv.wheel[i].relPos.x - car->_tireHeight(i);
				// Because of backface culling the winding of the triangles matters.
				if (car->_speed_x > 0.0f) {
					vtx[1] = car->priv.wheel[i].relPos.y + (sling_right + 1.0)*car->_tireWidth(i) / 2.0;
				} else {
					vtx[1] = car->priv.wheel[i].relPos.y + (sling_left - 1.0)* car->_tireWidth(i) / 2.0;
				}
				vtx[2] = contact_z;
				basevtx->add(vtx);

				vtx[0] = car->priv.wheel[i].relPos.x - car->_tireHeight(i);
				// Because of backface culling the winding of the triangles matters.
				if (car->_speed_x > 0.0f) {
					vtx[1] = car->priv.wheel[i].relPos.y + (sling_left - 1.0)* car->_tireWidth(i) / 2.0;
				} else {
					vtx[1] = car->priv.wheel[i].relPos.y + (sling_right + 1.0)*car->_tireWidth(i) / 2.0;
				}
				vtx[2] = contact_z;
				basevtx->add(vtx);

				texcoords = new ssgTexCoordArray();

				TxVtx[0] = grCarInfo[car->index].skidmarks->strips[i].tex_state;

				TxVtx[1] = 0.75f + sling_right*0.25f;
				texcoords->add(TxVtx);

				TxVtx[1] =  0.25f+sling_left*0.25f;
				texcoords->add(TxVtx);


				float dt = 0.01f; // should get it from somewhere else really. I could also account for radius, but that's a bit of an overkill.
				grCarInfo[car->index].skidmarks->strips[i].tex_state += dt*car->_wheelSpinVel(i);

				grCarInfo[car->index].skidmarks->base = new ssgVtxTable(GL_TRIANGLE_STRIP, basevtx, NULL, texcoords, NULL);
				grCarInfo[car->index].skidmarks->base->transform(grCarInfo[car->index].carPos);
				grCarInfo[car->index].skidmarks->base->getVertexList((void**)&tvtx);
				grCarInfo[car->index].skidmarks->base->getTexCoordList((void**)&tclist);
				/** this is the begining of a skid or the continue
					of the last one */
				if (grCarInfo[car->index].skidmarks->strips[i].last_state_of_skid == 0) {
					/* begin case */
					grCarInfo[car->index].skidmarks->strips[i].last_state_of_skid = 1;
					/* start a new one */
					grCarInfo[car->index].skidmarks->strips[i].running_skid = grCarInfo[car->index].skidmarks->strips[i].next_skid;
					if (grCarInfo[car->index].skidmarks->strips[i].state[grCarInfo[car->index].skidmarks->strips[i].running_skid] != SKID_UNUSED) {
						grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].running_skid]->removeAll();
						grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->removeAll();
						grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].running_skid]->removeAll();
						grCarInfo[car->index].skidmarks->strips[i].tex_state = 0.0;
						//printf("clearing [%d] %d\n",i,grCarInfo[car->index].skidmarks->strips[i].running_skid);
					}

					grCarInfo[car->index].skidmarks->strips[i].tex_state = 0.0;
					grCarInfo[car->index].skidmarks->strips[i].state[grCarInfo[car->index].skidmarks->strips[i].running_skid] = SKID_BEGIN;

					grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tvtx[0]);
					grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tvtx[1]);
					grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tclist[0]);
					grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tclist[1]);
					grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(cur_clr);
					grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(cur_clr);

					//printf("begin [%d] skid %d x = %.2f y = %.2f \n",i,grCarInfo[car->index].skidmarks->strips[i].running_skid,tvtx[0][0],tvtx[0][1]);
					grCarInfo[car->index].skidmarks->strips[i].vta[grCarInfo[car->index].skidmarks->strips[i].running_skid]->recalcBSphere();
					grCarInfo[car->index].skidmarks->strips[i].size[grCarInfo[car->index].skidmarks->strips[i].running_skid] = 2;
					grCarInfo[car->index].skidmarks->strips[i].timeStrip=t;
					grCarInfo[car->index].skidmarks->strips[i].vta[grCarInfo[car->index].skidmarks->strips[i].running_skid]->setCullFace(TRUE);
				} else {
					/* continue case */
					grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tvtx[0]);
					grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tvtx[1]);
					grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tclist[0]);
					grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(tclist[1]);
					grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(cur_clr);
					grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].running_skid]->add(cur_clr);

					//printf("continue [%d] x = %.2f y = %.2f \n",i,tvtx[0][0],tvtx[0][1]);
					grCarInfo[car->index].skidmarks->strips[i].size[grCarInfo[car->index].skidmarks->strips[i].running_skid] += 2;
					if ( grCarInfo[car->index].skidmarks->strips[i].state[grCarInfo[car->index].skidmarks->strips[i].running_skid] == SKID_BEGIN)
					{
						grCarInfo[car->index].skidmarks->strips[i].state[grCarInfo[car->index].skidmarks->strips[i].running_skid] = SKID_RUNNING;
						/*SkidAnchor->addKid(grCarInfo[car->index].skidmarks->strips[i].vta[grCarInfo[car->index].skidmarks->strips[i].running_skid]);*/
					}
					grCarInfo[car->index].skidmarks->strips[i].vta[grCarInfo[car->index].skidmarks->strips[i].running_skid]->recalcBSphere();
					grCarInfo[car->index].skidmarks->strips[i].timeStrip=t;
				}
				basevtx->removeAll();
				delete grCarInfo[car->index].skidmarks->base;
			} else {
				/** this is the end of a skid or there is nothing to do */
				if (grCarInfo[car->index].skidmarks->strips[i].last_state_of_skid != 0) {
					/* end the running skid */
					grCarInfo[car->index].skidmarks->strips[i].state[grCarInfo[car->index].skidmarks->strips[i].running_skid] = SKID_STOPPED;
					grCarInfo[car->index].skidmarks->strips[i].vta[grCarInfo[car->index].skidmarks->strips[i].running_skid]->recalcBSphere();
					grCarInfo[car->index].skidmarks->strips[i].last_state_of_skid = 0;
					//printf("end skid [%d] %d\n",i,grCarInfo[car->index].skidmarks->strips[i].running_skid);

					grCarInfo[car->index].skidmarks->strips[i].next_skid += 1;
					if (grCarInfo[car->index].skidmarks->strips[i].next_skid >= grSkidMaxStripByWheel) {
						grCarInfo[car->index].skidmarks->strips[i].next_skid = 0;
						/* reset the next skid vertexArray */
						grCarInfo[car->index].skidmarks->strips[i].vtx[grCarInfo[car->index].skidmarks->strips[i].next_skid]->removeAll();
						grCarInfo[car->index].skidmarks->strips[i].tex[grCarInfo[car->index].skidmarks->strips[i].running_skid]->removeAll();
						grCarInfo[car->index].skidmarks->strips[i].clr[grCarInfo[car->index].skidmarks->strips[i].next_skid]->removeAll();
						//printf("remove all in skid [%d] %d\n",i,grCarInfo[car->index].skidmarks->strips[i].next_skid);
					}
				}
	    	}
		}
    }
}


/** remove the skidmarks information for a car */
void grShutdownSkidmarks (void)
{
	int i;
	int z;

	GfOut("-- grShutdownSkidmarks\n");

	if (!grSkidMaxStripByWheel) {
		return;
	}

	SkidAnchor->removeAllKids();
	for (z = 0; z < grNbCars; z++) {
		for (i = 0; i<4; i++) {
			free(grCarInfo[z].skidmarks->strips[i].vtx);
			free(grCarInfo[z].skidmarks->strips[i].vta);
			free(grCarInfo[z].skidmarks->strips[i].tex);
			free(grCarInfo[z].skidmarks->strips[i].state);
			free(grCarInfo[z].skidmarks->strips[i].size);
			free(grCarInfo[z].skidmarks->strips[i].clr);
		}
		free(grCarInfo[z].skidmarks);
		grCarInfo[z].skidmarks = NULL;
	}
	skidState = NULL;
}


void grDrawSkidmarks (tCarElt *car)
{
    /* nothing to do */
  
  
}
