/***************************************************************************

    file                 : collide.cpp
    created              : Sun Mar 19 00:06:19 CET 2000
    copyright            : (C) 2000-2005 by Eric Espie, Bernhard Wymann
    email                : torcs@free.fr
    version              : $Id: collide.cpp,v 1.32 2006/02/20 20:15:15 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sim.h"

#define CAR_DAMMAGE	0.1


void SimCarCollideZ(tCar *car)
{
	int i;
	t3Dd normal;
	tdble dotProd;
	tWheel *wheel;
	const float CRASH_THRESHOLD = -5.0f;

	if (car->carElt->_state & RM_CAR_STATE_NO_SIMU) {
		return;
	}

	for (i = 0; i < 4; i++) {
		wheel = &(car->wheel[i]);
		if (wheel->state & SIM_SUSP_COMP) {
			car->DynGCg.pos.z += wheel->susp.spring.packers - wheel->rideHeight;
			RtTrackSurfaceNormalL(&(wheel->trkPos), &normal);
			dotProd = (car->DynGCg.vel.x * normal.x + car->DynGCg.vel.y * normal.y + car->DynGCg.vel.z * normal.z) * wheel->trkPos.seg->surface->kRebound;
			if (dotProd < 0) {
				if (dotProd < CRASH_THRESHOLD) {
					car->collision |= SEM_COLLISION_Z_CRASH;
				}
				car->collision |= SEM_COLLISION | SEM_COLLISION_Z;
				car->DynGCg.vel.x -= normal.x * dotProd;
				car->DynGCg.vel.y -= normal.y * dotProd;
				car->DynGCg.vel.z -= normal.z * dotProd;
				if ((car->carElt->_state & RM_CAR_STATE_FINISH) == 0) {
					if (getDamageLimit())
						car->dammage += (int)(wheel->trkPos.seg->surface->kDammage * fabs(dotProd) * simDammageFactor[car->carElt->_skillLevel]);
					else
						car->fakeDammage += (int)(wheel->trkPos.seg->surface->kDammage * fabs(dotProd) * simDammageFactor[car->carElt->_skillLevel]);

					
				}
			}
		}
	}
}

const tdble BorderFriction = 0.00;

// Collision of car/track borders.
// Be aware that it does not work for convex edges (e.g. e-track-2, end of the straight, left),
// because under these conditions it is possible that all car corners are on the track and
// the car body is partially outside the track.
void SimCarCollideXYScene(tCar *car)
{
	tTrackSeg *seg = car->trkPos.seg;
	tTrkLocPos trkpos;
	int i;
	tDynPt *corner;
	tdble initDotProd;
	tdble dotProd, cx, cy, dotprod2;
	tTrackBarrier *curBarrier;
	tdble dmg;

	if (car->carElt->_state & RM_CAR_STATE_NO_SIMU) {
		return;
	}

	corner = &(car->corner[0]);
	for (i = 0; i < 4; i++, corner++) {
		seg = car->trkPos.seg;
		RtTrackGlobal2Local(seg, corner->pos.ax, corner->pos.ay, &trkpos, TR_LPOS_TRACK);
		seg = trkpos.seg;
		tdble toSide;

		if (trkpos.toRight < 0.0) {
			// collision with right border.
			curBarrier = seg->barrier[TR_SIDE_RGT];
			toSide = trkpos.toRight;
		} else if (trkpos.toLeft < 0.0) {
			// collision with left border.
			curBarrier = seg->barrier[TR_SIDE_LFT];
			toSide = trkpos.toLeft;
		} else {
			continue;
		}

		const tdble& nx = curBarrier->normal.x;
		const tdble& ny = curBarrier->normal.y;

		car->DynGCg.pos.x -= nx * toSide;
		car->DynGCg.pos.y -= ny * toSide;

		// Corner position relative to center of gravity.
		cx = corner->pos.ax - car->DynGCg.pos.x;
		cy = corner->pos.ay - car->DynGCg.pos.y;

		car->blocked = 1;
		car->collision |= SEM_COLLISION;

		// Impact speed perpendicular to barrier (of corner).
		initDotProd = nx * corner->vel.x + ny * corner->vel.y;

		// Compute dmgDotProd (base value for later damage) with a heuristic.
		tdble absvel = MAX(1.0, sqrt(car->DynGCg.vel.x*car->DynGCg.vel.x + car->DynGCg.vel.y*car->DynGCg.vel.y));
		tdble GCgnormvel = car->DynGCg.vel.x*nx + car->DynGCg.vel.y*ny;
		tdble cosa = GCgnormvel/absvel;
		tdble dmgDotProd = GCgnormvel*cosa;

		dotProd = initDotProd * curBarrier->surface->kFriction;
		car->DynGCg.vel.x -= nx * dotProd;
		car->DynGCg.vel.y -= ny * dotProd;
		dotprod2 = (nx * cx + ny * cy);

		// Angular velocity change caused by friction of collisding car part with wall.
		static tdble VELSCALE = 10.0f;
		static tdble VELMAX = 6.0f;
		car->DynGCg.vel.az -= dotprod2 * dotProd / VELSCALE;
		if (fabs(car->DynGCg.vel.az) > VELMAX) {
			car->DynGCg.vel.az = SIGN(car->DynGCg.vel.az) * VELMAX;
		}

		// Dammage.
		dotProd = initDotProd;
		if (dotProd < 0.0f && (car->carElt->_state & RM_CAR_STATE_FINISH) == 0) {
			dmg = curBarrier->surface->kDammage * fabs(0.5*dmgDotProd*dmgDotProd) * simDammageFactor[car->carElt->_skillLevel];
			if (getDamageLimit())
				car->dammage += (int)dmg;
			else
				car->fakeDammage += (int)dmg;
		} else {
			dmg = 0.0f;
		}

		dotProd *= curBarrier->surface->kRebound;

		// If the car moves toward the barrier, rebound.
		if (dotProd < 0.0f) {
			car->collision |= SEM_COLLISION_XYSCENE;
			car->normal.x = nx * dmg;
			car->normal.y = ny * dmg;
			car->collpos.x = corner->pos.ax;
			car->collpos.y = corner->pos.ay;
			car->DynGCg.vel.x -= nx * dotProd;
			car->DynGCg.vel.y -= ny * dotProd;
		}
    }
}


static void SimCarCollideResponse(void * /*dummy*/, DtObjectRef obj1, DtObjectRef obj2, const DtCollData *collData)
{
	sgVec2 n;		// Collision normal delivered by solid: Global(point1) - Global(point2)
	tCar *car[2];	// The cars.
	sgVec2 p[2];	// Collision points delivered by solid, in body local coordinates.
	sgVec2 r[2];	// Collision point relative to center of gravity.
	sgVec2 vp[2];	// Speed of collision point in world coordinate system.
	sgVec3 pt[2];	// Collision points in global coordinates.

	int i;

	car[0] = (tCar*)obj1;
	car[1] = (tCar*)obj2;

	// Handle cars collisions during pit stops as well.
	static const int NO_SIMU_WITHOUT_PIT = RM_CAR_STATE_NO_SIMU & ~RM_CAR_STATE_PIT;

	if ((car[0]->carElt->_state & NO_SIMU_WITHOUT_PIT) ||
		(car[1]->carElt->_state & NO_SIMU_WITHOUT_PIT))
	{
		return;
	}

    if (car[0]->carElt->index < car[1]->carElt->index) {
		// vector conversion from double to float.
		p[0][0] = (float)collData->point1[0];
		p[0][1] = (float)collData->point1[1];
		p[1][0] = (float)collData->point2[0];
		p[1][1] = (float)collData->point2[1];
		n[0]  = (float)collData->normal[0];
		n[1]  = (float)collData->normal[1];
    } else {
		// swap the cars (not the same for the simu).
		car[0] = (tCar*)obj2;
		car[1] = (tCar*)obj1;
		p[0][0] = (float)collData->point2[0];
		p[0][1] = (float)collData->point2[1];
		p[1][0] = (float)collData->point1[0];
		p[1][1] = (float)collData->point1[1];
		n[0]  = -(float)collData->normal[0];
		n[1]  = -(float)collData->normal[1];
	}

	sgNormaliseVec2(n);

	sgVec2 rg[2];	// raduis oriented in global coordinates, still relative to CG (rotated aroung CG).
	tCarElt *carElt;

	for (i = 0; i < 2; i++) {
		// vector GP (Center of gravity to collision point). p1 and p2 are delivered from solid as
		// points in the car coordinate system.
		sgSubVec2(r[i], p[i], (const float*)&(car[i]->statGC));

		// Speed of collision points, linear motion of center of gravity (CG) plus rotational
		// motion around the CG.
		carElt = car[i]->carElt;
		float sina = sin(carElt->_yaw);
		float cosa = cos(carElt->_yaw);
		rg[i][0] = r[i][0]*cosa - r[i][1]*sina;
		rg[i][1] = r[i][0]*sina + r[i][1]*cosa;

		vp[i][0] = car[i]->DynGCg.vel.x - car[i]->DynGCg.vel.az * rg[i][1];
		vp[i][1] = car[i]->DynGCg.vel.y + car[i]->DynGCg.vel.az * rg[i][0];
	}

	// Relative speed of collision points.
	sgVec2 v1ab;
	sgSubVec2(v1ab, vp[0], vp[1]);

	// try to separate the cars. The computation is necessary because dtProceed is not called till
	// the collision is resolved. 
	for (i = 0; i < 2; i++) {
		sgCopyVec2(pt[i], r[i]);
		pt[i][2] = 0.0f;
		// Transform points relative to cars local coordinate system into global coordinates.
		sgFullXformPnt3(pt[i], car[i]->carElt->_posMat);
	}

	// Compute distance of collision points.
	sgVec3 pab;
	sgSubVec2(pab, pt[1], pt[0]);
	float distpab = sgLengthVec2(pab);

	sgVec2 tmpv;
	
	sgScaleVec2(tmpv, n, MIN(distpab, 0.05));
	// No "for" loop here because of subtle difference AddVec/SubVec. 
	if (car[0]->blocked == 0 && !(car[0]->carElt->_state & RM_CAR_STATE_NO_SIMU)) {
		sgAddVec2((float*)&(car[0]->DynGCg.pos), tmpv);
		car[0]->blocked = 1;
    }
	if (car[1]->blocked == 0 && !(car[1]->carElt->_state & RM_CAR_STATE_NO_SIMU)) {
		sgSubVec2((float*)&(car[1]->DynGCg.pos), tmpv);
		car[1]->blocked = 1;
    }

	// Doing no dammage and correction if the cars are moving out of each other.
	if (sgScalarProductVec2(v1ab, n) > 0) {
		return;
	}

	// impulse.
	float rpn[2];
	rpn[0] = sgScalarProductVec2(rg[0], n);
	rpn[1] = sgScalarProductVec2(rg[1], n);

	// Pesudo cross product to find out if we are left or right.
	// TODO: SIGN, scrap value?
	float rpsign[2];
	rpsign[0] =  n[0]*rg[0][1] - n[1]*rg[0][0];
	rpsign[1] = -n[0]*rg[1][1] + n[1]*rg[1][0];

	const float e = 1.0f;	// energy restitution

	float j = -(1.0f + e) * sgScalarProductVec2(v1ab, n) /
		((car[0]->Minv + car[1]->Minv) +
		rpn[0] * rpn[0] * car[0]->Iinv.z + rpn[1] * rpn[1] * car[1]->Iinv.z);

	for (i = 0; i < 2; i++) {
		if (car[i]->carElt->_state & RM_CAR_STATE_NO_SIMU) {
			continue;
		}

		// Damage.
		tdble damFactor, atmp;
		atmp = atan2(r[i][1], r[i][0]);
		if (fabs(atmp) < (PI / 3.0)) {
			// Front collision gives more damage.
			damFactor = 1.5f;
		} else {
			// Rear collision gives less damage.
			damFactor = 1.0f;
		}

		if ((car[i]->carElt->_state & RM_CAR_STATE_FINISH) == 0) {
			if (getDamageLimit())
				car[i]->dammage += (int)(CAR_DAMMAGE * fabs(j) * damFactor * simDammageFactor[car[i]->carElt->_skillLevel]);
			else
				car[i]->fakeDammage += (int)(CAR_DAMMAGE * fabs(j) * damFactor * simDammageFactor[car[i]->carElt->_skillLevel]);
		}

		// Compute collision velocity.
		const float ROT_K = 1.0f;

		float js = (i == 0) ? j : -j;
		sgScaleVec2(tmpv, n, js * car[i]->Minv);
		sgVec2 v2a;

		if (car[i]->collision & SEM_COLLISION_CAR) {
			sgAddVec2(v2a, (const float*)&(car[i]->VelColl.x), tmpv);
			car[i]->VelColl.az = car[i]->VelColl.az + js * rpsign[i] * rpn[i] * car[i]->Iinv.z * ROT_K;
		} else {
			sgAddVec2(v2a, (const float*)&(car[i]->DynGCg.vel), tmpv);
			car[i]->VelColl.az = car[i]->DynGCg.vel.az + js * rpsign[i] * rpn[i] * car[i]->Iinv.z * ROT_K;
		}

		static float VELMAX = 3.0f;
		if (fabs(car[i]->VelColl.az) > VELMAX) {
			car[i]->VelColl.az = SIGN(car[i]->VelColl.az) * VELMAX;
		}

		sgCopyVec2((float*)&(car[i]->VelColl.x), v2a);

		// Move the car for the collision lib.
		tCarElt *carElt = car[i]->carElt;
		sgMakeCoordMat4(carElt->pub.posMat, car[i]->DynGCg.pos.x, car[i]->DynGCg.pos.y,
						car[i]->DynGCg.pos.z - carElt->_statGC_z, RAD2DEG(carElt->_yaw),
						RAD2DEG(carElt->_roll), RAD2DEG(carElt->_pitch));
		dtSelectObject(car[i]);
		dtLoadIdentity();
		dtTranslate(-carElt->_statGC_x, -carElt->_statGC_y, 0.0f);
		dtMultMatrixf((const float *)(carElt->_posMat));

		car[i]->collision |= SEM_COLLISION_CAR;
	}
}


// Collision response for walls.
// TODO: Separate common code with car-car collision response.
static void SimCarWallCollideResponse(void *clientdata, DtObjectRef obj1, DtObjectRef obj2, const DtCollData *collData)
{
	tCar* car;		// The car colliding with the wall.
	float nsign;	// Normal direction correction for collision plane.
	sgVec2 p;		// Cars collision point delivered by solid.

	// TODO: If other movable objects are added which could collide with the wall, it will be
	// necessary to validate if the object is actually a car.

	if (obj1 == clientdata) {
		car = (tCar*) obj2;
		nsign = -1.0f;
		p[0] = (float) collData->point2[0];
		p[1] = (float) collData->point2[1];
	} else {
		car = (tCar*) obj1;
		nsign = 1.0f;
		p[0] = (float) collData->point1[0];
		p[1] = (float) collData->point1[1];
	}

	sgVec2 n;		// Collision normal delivered by solid, corrected such that it points away from the wall.
	n[0] = nsign * (float) collData->normal[0];
	n[1] = nsign * (float) collData->normal[1];
	float pdist = sgLengthVec2(n);	// Distance of collision points.
	sgNormaliseVec2(n);

	sgVec2 r;
	sgSubVec2(r, p, (const float*)&(car->statGC));

	tCarElt *carElt = car->carElt;

	sgVec2 vp;		// Speed of car collision point in global frame of reference.
	sgVec2 rg;		// raduis oriented in global coordinates, still relative to CG (rotated aroung CG).

	float sina = sin(carElt->_yaw);
	float cosa = cos(carElt->_yaw);
	rg[0] = r[0]*cosa - r[1]*sina;
	rg[1] = r[0]*sina + r[1]*cosa;

	vp[0] = car->DynGCg.vel.x - car->DynGCg.vel.az * rg[1];
	vp[1] = car->DynGCg.vel.y + car->DynGCg.vel.az * rg[0];

	sgVec2 tmpv;
	static const float CAR_MIN_MOVEMENT = 0.02f;
	static const float CAR_MAX_MOVEMENT = 0.05f;
	sgScaleVec2(tmpv, n, MIN(MAX(pdist, CAR_MIN_MOVEMENT), CAR_MAX_MOVEMENT));
	if (car->blocked == 0) {
		sgAddVec2((float*)&(car->DynGCg.pos), tmpv);
		car->blocked = 1;
    }

	// Doing no dammage and correction if the cars are moving out of each other.
	if (sgScalarProductVec2(vp, n) > 0) {
		return;
	}

	float rp = sgScalarProductVec2(rg, n);

	// Pesudo cross product to find out if we are left or right.
	// TODO: SIGN, scrap value?
	float rpsign = n[0]*rg[1] - n[1]*rg[0];

	const float e = 1.0f;	// energy restitution
	float j = -(1.0f + e) * sgScalarProductVec2(vp, n) / (car->Minv + rp * rp * car->Iinv.z);
	const float ROT_K = 0.5f;

	// Damage.
	tdble damFactor, atmp;
	atmp = atan2(r[1], r[0]);
	if (fabs(atmp) < (PI / 3.0)) {
		// Front collision gives more damage.
		damFactor = 1.5f;
	} else {
		// Rear collision gives less damage.
		damFactor = 1.0f;
	}

	static const float DMGFACTOR = 0.00002f;
	if ((car->carElt->_state & RM_CAR_STATE_FINISH) == 0) {
		if (getDamageLimit())
			car->dammage += (int)(CAR_DAMMAGE * (DMGFACTOR*j*j) * damFactor * simDammageFactor[car->carElt->_skillLevel]);
		else
			car->fakeDammage += (int)(CAR_DAMMAGE * (DMGFACTOR*j*j) * damFactor * simDammageFactor[car->carElt->_skillLevel]);
	}

	sgScaleVec2(tmpv, n, j * car->Minv);
	sgVec2 v2a;

	if (car->collision & SEM_COLLISION_CAR) {
		sgAddVec2(v2a, (const float*)&(car->VelColl.x), tmpv);
		car->VelColl.az = car->VelColl.az + j * rp * rpsign * car->Iinv.z * ROT_K;
	} else {
		sgAddVec2(v2a, (const float*)&(car->DynGCg.vel), tmpv);
		car->VelColl.az = car->DynGCg.vel.az + j * rp * rpsign * car->Iinv.z * ROT_K;
	}

	static float VELMAX = 3.0f;
	if (fabs(car->VelColl.az) > VELMAX) {
		car->VelColl.az = SIGN(car->VelColl.az) * VELMAX;
	}

	sgCopyVec2((float*)&(car->VelColl.x), v2a);

	// Move the car for the collision lib.
	sgMakeCoordMat4(carElt->pub.posMat, car->DynGCg.pos.x, car->DynGCg.pos.y,
					car->DynGCg.pos.z - carElt->_statGC_z, RAD2DEG(carElt->_yaw),
					RAD2DEG(carElt->_roll), RAD2DEG(carElt->_pitch));
	dtSelectObject(car);
	dtLoadIdentity();
	dtTranslate(-carElt->_statGC_x, -carElt->_statGC_y, 0.0f);
	dtMultMatrixf((const float *)(carElt->_posMat));

	car->collision |= SEM_COLLISION_CAR;
}


// Remove wrecked cars to avoid glitches in collision detection and to improve performance.
// Called by RemoveCar.
void SimCollideRemoveCar(tCar *car, int nbcars)
{
	// Find the car to remove.
	int i;
	for (i = 0; i < nbcars; i++) {
		if (&SimCarTable[i] == car) {
			break;
		}
	}

	// Remove it.
	if (SimCarTable[i].shape != NULL) {
		dtDeleteObject(&(SimCarTable[i]));
		dtDeleteShape(SimCarTable[i].shape);
		SimCarTable[i].shape = NULL;
	}
}


// Static object and shape list.
// TODO: Dynamic implementation.
static DtShapeRef fixedobjects[100];
// Id to the next free slot in fixedobjects.
static unsigned int fixedid;


void SimCarCollideShutdown(int nbcars)
{
	int i;

	for (i = 0; i < nbcars; i++) {
		// Check if car has not been removed already (wrecked).
		if (SimCarTable[i].shape != NULL) {
			dtDeleteObject(&(SimCarTable[i]));
			dtDeleteShape(SimCarTable[i].shape);
		}
	}

	unsigned int j;
	for (j = 0; j < fixedid; j++) {
		dtClearObjectResponse(&fixedobjects[j]);
		dtDeleteObject(&fixedobjects[j]);
		dtDeleteShape(fixedobjects[j]);
	}
	fixedid = 0;

	dtClearDefaultResponse();
}


// Searching backwards for the first non wall segments to start.
// Used for the creation of collision detection model of the wall.
static tTrackSeg *getFirstWallStart(tTrackSeg *start, int side)
{
	tTrackSeg *first = start;

	// Moving backward out of wall.
	do {
		// A wall is a wall on the track if it is not the barrier at the same time.
		if (first->side[side] != NULL && first->side[side]->style == TR_WALL &&
			first->side[side]->side[side] != NULL)
		{
			first = first->prev;
		} else {
			break;
		}
	} while (first != start);

	// Searching forward for the first wall segment.
	start = first;
	do {
		if (first->side[side] != NULL && first->side[side]->style == TR_WALL &&
			first->side[side]->side[side] != NULL)
		{
			return first;
		} else {
			first = first->next;
		}
	} while (first != start);

	return NULL;
}


// Create the left walls, start must point to a segment with a wall on the left and a leading
// non wall segment.
// FIXME: Does not work for a closed ring "by design".
void buildWalls(tTrackSeg *start, int side) {

	if (start == NULL) {
		return;
	}

	tTrackSeg *current = start;
	bool close = false;

	// Going down the wall.
	do {
		tTrackSeg* s = current->side[side];
		tTrackSeg* p = current->prev->side[side];
		tTrackSeg* n = current->next->side[side];

		// Current segment is a wall?
		if (s != NULL && s->style == TR_WALL && s->side[side] != NULL) {

			float h = s->height;
			t3Dd svl = s->vertex[TR_SL];
			t3Dd svr = s->vertex[TR_SR];
			t3Dd evl = s->vertex[TR_EL];
			t3Dd evr = s->vertex[TR_ER];
			static float weps = 0.01f;

			// Close the start with a ploygon?
			if (p == NULL || p->style != TR_WALL ||
				(fabs(p->vertex[TR_EL].x - svl.x) > weps) ||
				(fabs(p->vertex[TR_ER].x - svr.x) > weps) ||
				(fabs(h - p->height) > weps) ||
				fixedid == 0)
			{
				// Enough space in array?
				if (fixedid >= sizeof(fixedobjects)/sizeof(fixedobjects[0])) {
					GfError("fixedobjects full in %s, line %d\n", __FILE__, __LINE__);
					return;
				}

				if (close == true) {
					dtEndComplexShape();
					GfError("Shape not closed %s, line %d\n", __FILE__, __LINE__);
				}

				// Start new shape.
				fixedobjects[fixedid] = dtNewComplexShape();
				fixedid++;
				close = true;

				dtBegin(DT_POLYGON);
					dtVertex(svl.x, svl.y, svl.z);
					dtVertex(svr.x, svr.y, svr.z);
					dtVertex(svr.x, svr.y, svr.z + h);
					dtVertex(svl.x, svl.y, svl.z + h);
				dtEnd();
			}

			// Build sides, left, top, right. Depending on how we want to use it we
			// can remove top perhaps.
			// TODO: do we want the top poly?
			if (close == true) {
				// Left.
				dtBegin(DT_POLYGON);
					dtVertex(svl.x, svl.y, svl.z);
					dtVertex(svl.x, svl.y, svl.z + h);
					dtVertex(evl.x, evl.y, evl.z + h);
					dtVertex(evl.x, evl.y, evl.z);
				dtEnd();
				/*
				// Top.
				dtBegin(DT_POLYGON);
					dtVertex(svl.x, svl.y, svl.z + h);
					dtVertex(svr.x, svr.y, svr.z + h);
					dtVertex(evr.x, evr.y, evr.z + h);
					dtVertex(evl.x, evl.y, evl.z + h);
				dtEnd();*/
				// Right.
				dtBegin(DT_POLYGON);
					dtVertex(svr.x, svr.y, svr.z + h);
					dtVertex(svr.x, svr.y, svr.z);
					dtVertex(evr.x, evr.y, evr.z);
					dtVertex(evr.x, evr.y, evr.z + h);
				dtEnd();
			} else {
				GfError("Shape not open %s, line %d\n", __FILE__, __LINE__);
			}

			// Close the end with a ploygon?
			if (n == NULL || n->style != TR_WALL ||
				(fabs(n->vertex[TR_SL].x - evl.x) > weps) ||
				(fabs(n->vertex[TR_SR].x - evr.x) > weps) ||
				(fabs(h - n->height) > weps))
			{
				if (close == true) {
					dtBegin(DT_POLYGON);
						dtVertex(svl.x, svl.y, svl.z);
						dtVertex(svr.x, svr.y, svr.z);
						dtVertex(svr.x, svr.y, svr.z + h);
						dtVertex(svl.x, svl.y, svl.z + h);
					dtEnd();

					dtEndComplexShape();
					close = false;
				} else {
					GfError("Shape not open %s, line %d\n", __FILE__, __LINE__);
				}
			}

		}

		current = current->next;

	} while (current != start);

}


void
SimCarCollideConfig(tCar *car, tTrack *track)
{
	tCarElt *carElt;

	// Create car collision objects.
	carElt = car->carElt;
	// The current car shape is a box...
	car->shape = dtBox(carElt->_dimension_x, carElt->_dimension_y, carElt->_dimension_z);
	dtCreateObject(car, car->shape);

	car->collisionAware = 1;

	// Create fixed track collision objects (just pit wall for now).
	// TODO: car body/curbs collision.
	// TODO: car body/flat wall collision (e.g. for pavement, sidewalk).
	// TODO: define static objects in XML file/tTrack, collide with them as well.

	fixedid = 0;

	if (track != NULL) {
		tTrackSeg *firstleft = getFirstWallStart(track->seg, TR_SIDE_LFT);
		tTrackSeg *firstright = getFirstWallStart(track->seg, TR_SIDE_RGT);

		buildWalls(firstleft, TR_SIDE_LFT);
		buildWalls(firstright, TR_SIDE_RGT);

		unsigned int i;
		for (i = 0; i < fixedid; i++) {
			dtCreateObject(&fixedobjects[i], fixedobjects[i]);
			dtSetObjectResponse(&fixedobjects[i], SimCarWallCollideResponse, DT_SMART_RESPONSE, &fixedobjects[i]);
		}

	}
}


void
SimCarCollideInit(void)
{
	dtSetDefaultResponse(SimCarCollideResponse, DT_SMART_RESPONSE, NULL);
	// Hmm, why is caching disabled, are our objects too fast, so it does not work?
	// TODO: understand this and reconsider caching.
	dtDisableCaching();
	dtSetTolerance(0.001);
}


// TODO: Rename, because actually the dtProceed handles all solid managed objects, e.g. walls.
void
SimCarCollideCars(tSituation *s)
{
	tCar *car;
	tCarElt *carElt;
	int i;

	for (i = 0; i < s->_ncars; i++) {
		carElt = s->cars[i];
		if (carElt->_state & RM_CAR_STATE_NO_SIMU) {
			continue;
		}

		car = &(SimCarTable[carElt->index]);
		dtSelectObject(car);
		// Fit the bounding box around the car, statGC's are the static offsets.
		dtLoadIdentity();
		dtTranslate(-carElt->_statGC_x, -carElt->_statGC_y, 0.0f);
		// Place the bounding box such that it fits the car in the world.
		dtMultMatrixf((const float *)(carElt->_posMat));
		memset(&(car->VelColl), 0, sizeof(tPosd));
	}

	// Running the collision detection. If no collision is detected, call dtProceed.
	// dtProceed just works if all objects are disjoint.
	if (dtTest() == 0) {
		dtProceed();
	}

	for (i = 0; i < s->_ncars; i++) {
		carElt = s->cars[i];
		if (carElt->_state & RM_CAR_STATE_NO_SIMU) {
			continue;
		}
		car = &(SimCarTable[carElt->index]);
		if (car->collision & SEM_COLLISION_CAR) {
			car->DynGCg.vel.x = car->VelColl.x;
			car->DynGCg.vel.y = car->VelColl.y;
			car->DynGCg.vel.az = car->VelColl.az;
		}
	}
}

