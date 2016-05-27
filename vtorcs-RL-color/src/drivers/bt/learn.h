/***************************************************************************

    file                 : learn.h
    created              : Wed Aug 28 16:36:00 CET 2004
    copyright            : (C) 2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: learn.h,v 1.3.2.1 2008/11/09 17:50:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _SEGLEARN_H_
#define _SEGLEARN_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

class SegLearn {
	public:
		SegLearn(tTrack* t, tSituation *s, int driverindex);
		~SegLearn();

		float getRadius(tTrackSeg *s) { return radius[s->id]; }
		void update(tSituation *s, tTrack *t, tCarElt *car, int alone, float offset, float outside, float *r);

	private:
		float *radius;
		int *updateid;
		int nseg;

		bool check;
		float rmin;
		int lastturn;
		int prevtype;
		static const int BUFSIZE;
		char filename[1024];

		void writeKarma();
		FILE* getKarmaFilename(tTrack* t, tSituation *s, int driverindex);
		FILE* tryKarmaFilename(char* buffer, int size, const char *path, int driverindex, const char *tbuf, bool storelocalfilename);
		bool readKarma(tTrack* track, tSituation *s, float *radius, int *uid, int driverindex);
};


#endif //_SEGLEARN_H_
