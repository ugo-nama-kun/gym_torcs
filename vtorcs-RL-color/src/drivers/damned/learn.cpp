/***************************************************************************

    file                 : learn.cpp
    created              : Wed Aug 28 16:36:00 CET 2004
    copyright            : (C) 2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: learn.cpp,v 1.1.2.1 2008/05/28 21:34:44 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "learn.h"
#include <portability.h>

#define MAGIC1		0x34be1f01
// Change MAGIC2 if the learning file format changes.
#define MAGIC2		0x45aa9fbe
#define STRINGID	"TORCS"

SegLearn::SegLearn(tTrack* t, tSituation *s, int driverindex)
{
	int i;
	radius = new float[t->nseg];
	updateid = new int[t->nseg];
	nseg = t->nseg;

	if (!readKarma(t, s, radius, updateid, driverindex)) {

		tTrackSeg *seg = t->seg;

		// Switch seg to seg 0 for sure.
		while (seg->id != 0) {
			seg = seg->prev;
		}

		for (i = 0; i < t->nseg; i++) {
			radius[i] = 0.0f;
			updateid[i] = i;
			// Search the last turn in case of a straight.
			if (seg->type == TR_STR) {
				tTrackSeg *cs = seg;
				while (cs->type == TR_STR) {
					cs = cs->prev;
				}
				updateid[seg->id] = cs->id;
			}
			seg = seg->next;
		}
	}

	check = false;
	rmin = t->width/2.0f;
	prevtype = lastturn = TR_STR;
}


SegLearn::~SegLearn()
{
	writeKarma();
	delete [] radius;
	delete [] updateid;
}


void SegLearn::update(tSituation *s, tTrack *t, tCarElt *car, int alone, float offset, float outside, float *r)
{
	// Still on the same segment, alone, offset near 0, check.
	tTrackSeg *seg = car->_trkPos.seg;

	if (seg->type == lastturn || seg->type == TR_STR) {
		if (fabs(offset) < 0.2f &&
			check == true &&
			alone > 0
		) {
			// + to left, - to right
			float tomiddle = car->_trkPos.toMiddle;
			float dr = 0.0f;
			if (lastturn == TR_RGT) {
				dr = outside - tomiddle;
			} else if (lastturn == TR_LFT) {
				dr = outside + tomiddle;
			}
			if (dr < rmin) {
				rmin = dr;
			}
		} else {
			check = false;
		}
	}

	if (seg->type != prevtype) {
		prevtype = seg->type;
		if (seg->type != TR_STR) {
			if (check == true) {
				tTrackSeg *cs = seg->prev;
				// Skip straights.
				while (cs->type == TR_STR) {
					cs = cs->prev;
				}

				while (cs->type == lastturn) {
					if (radius[updateid[cs->id]] + rmin < 0.0f) {
						rmin = MAX(cs->radius - r[cs->id], rmin);
					}
					radius[updateid[cs->id]] += rmin;
					radius[updateid[cs->id]] = MIN(radius[updateid[cs->id]], 1000.0f);
					cs = cs->prev;
				}
			}
			check = true;
			rmin = MIN(seg->width/2.0f, seg->radius/10.0f);
			lastturn = seg->type;
		}
	}
}


void SegLearn::writeKarma()
{
	// Build the directory name.
	char path[sizeof(filename)];
	strncpy(path, filename, sizeof(path));
	char* end = strrchr(path, '/');
	if (end != NULL) {
		*end = '\0';
	}

	// Create the directory and try to write data.
	if (GfCreateDir(path) == GF_DIR_CREATED) {
		// Try to write data.
		FILE *fd = fopen(filename, "wb");
		if (fd != NULL) {
			// Create header: Magic Number, #segments, string, version.
			int magic = MAGIC1;
			int magic2 = MAGIC2;
			char string[] = STRINGID;
			
			// The magic numbers are used to catch 32/64 bit mismatches and little/big
			// endian mismatches. Here the patterns I expect at the beginning of the files.
			// I call 4 bytes a UNIT, MAGIC1 bytes A1, A2, A3, A4, MAGIC2 bytes B1, B2, B3, B4,
			// a zeroed byte 00):
			// 32bit big endian   : UNIT1-A1A2A3A4; UNIT2-B1B2B3B4; ...
			// 32bit little endian: UNIT1-A4A3A2A1; UNIT2-B4B3B2B1; ...
			// 64bit big endian   : UNIT1-00000000; UNIT2-A1A2A3A4; UNIT3-00000000; UNIT4-B1B2B3B4; ...
			// 64bit little endian: UNIT1-A4A3A2A1; UNIT2-00000000; UNIT3-B4B3B2B1; UNIT4-00000000: ...
			//
			// Like you can see there is created a unique pattern for each architecture in
			// UNIT1 and UNIT2.

			fwrite(&magic, sizeof(magic), 1, fd);			// magic number.
			fwrite(&magic2, sizeof(magic2), 1, fd);			// magic number 2.
			fwrite(&nseg, sizeof(nseg), 1, fd);				// # segments.
			fwrite(string, sizeof(string), 1, fd);			// string.
			
			for (int i = 0; i < nseg; i++) {
				fwrite(&updateid[i], sizeof(updateid[0]), 1, fd);
				fwrite(&radius[i], sizeof(radius[0]), 1, fd);
        	}
        	fclose(fd);
		}
	}
}


bool SegLearn::readKarma(tTrack* track, tSituation *s, float *radius, int *uid, int driverindex)
{
	FILE* fd = getKarmaFilename(track, s, driverindex);

	if (fd != NULL) {
		// Check if the file is valid.
		int magic = 0;
		int magic2 = 0;
		int nseg = 0;
		char string[sizeof(STRINGID)] = "";
	
		fread(&magic, sizeof(magic), 1, fd);
		fread(&magic2, sizeof(magic2), 1, fd);
		fread(&nseg, sizeof(nseg), 1, fd);
		fread(string, sizeof(string), 1, fd);
	
		if (magic == MAGIC1 && magic2 == MAGIC2 &&
			nseg == track->nseg &&
			strncmp(string, STRINGID, sizeof(string)) == 0
			)
		{
			for (int i = 0; i < track->nseg; i++) {
				fread(&uid[i], sizeof(uid[0]), 1, fd);
				fread(&radius[i], sizeof(radius[0]), 1, fd);
        	}
			fclose(fd);
			return true;
		}
		fclose(fd);
	}
	return false;
}


FILE* SegLearn::getKarmaFilename(tTrack* track, tSituation *s, int driverindex)
{
	const int TBUFSIZE = 256;
	char tbuf[TBUFSIZE];
	char* trackname = strrchr(track->filename, '/') + 1;
	char* tracknameend = strchr(trackname, '.') - 1;

	strncpy(tbuf, trackname, tracknameend-trackname+1);
	tbuf[tracknameend-trackname+1] = 0;

	FILE* fd;
	char buffer[sizeof(filename)];

	switch (s->_raceType) {
		case RM_TYPE_RACE:
			fd = tryKarmaFilename(buffer, sizeof(buffer), "%sdrivers/damned/%d/race/%s.karma", driverindex, tbuf, s->_raceType == RM_TYPE_RACE);
			if ( fd != NULL) {
				return fd;
			} // not found, try the next.
		case RM_TYPE_QUALIF:
			fd = tryKarmaFilename(buffer, sizeof(buffer), "%sdrivers/damned/%d/qualifying/%s.karma", driverindex, tbuf, s->_raceType == RM_TYPE_QUALIF);
			if ( fd != NULL) {
				return fd;
			} // not found, try the next.
		case RM_TYPE_PRACTICE:
			fd = tryKarmaFilename(buffer, sizeof(buffer), "%sdrivers/damned/%d/practice/%s.karma", driverindex, tbuf, s->_raceType == RM_TYPE_PRACTICE);
			if ( fd != NULL) {
				return fd;
			} // not found, try the next.		
		default:
			return NULL;
			break;
	}
}


FILE* SegLearn::tryKarmaFilename(char* buffer, int size, const char *path, int driverindex, const char *tbuf, bool storelocalfilename)
{
	// First construct a path to the local directory ($HOME/...).
	snprintf(buffer, size, path, GetLocalDir(), driverindex, tbuf);
	if (storelocalfilename == true) {
		strncpy(filename, buffer, sizeof(filename));
	}

	// Try to open the local file.
	FILE* fd;		
	if ((fd = fopen(buffer, "rb")) != NULL) {
		return fd;
	}

	// Not found, try the global path.
	snprintf(buffer, size, path, GetDataDir(), driverindex, tbuf);
	return fopen(buffer, "rb");
}
