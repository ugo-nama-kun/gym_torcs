/***************************************************************************

    file                 : driver.h
    created              : Thu Dec 20 01:20:19 CET 2002
    copyright            : (C) 2002 Bernhard Wymann

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// TUTORIAL FOLLOWED FOR THE FIRST TWO CHAPTERS
// CONTINUE FROM HERE: 
// file:///home/giuse/torcs-1.3.1/doc/tutorials/robot/tutorial.html

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//using std::cout;
//using std::endl;

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>


class Driver {

  private:
		/* class constants */
		static const float MAX_UNSTUCK_ANGLE;
		static const float UNSTUCK_TIME_LIMIT;
    static const float MAX_UNSTUCK_SPEED;
    static const float MIN_UNSTUCK_DIST;
    static const float STEERING_CONSTANT;
		int MAX_UNSTUCK_COUNT;
		int INDEX;

		/* class variables */
		tTrack* track;
		/* per robot global data */
		int stuck;
		float trackangle;
		float angle;

		/* driving tools */
		void update(tCarElt* car, tSituation *s);
    bool isStuck(tCarElt* car);

    // GIUSE - drive!
    void followcenter(tCarElt* car);
    void unstuck(tCarElt* car);

	public:
		Driver(int index);
		//~Driver();

		/* callback functions called from TORCS */
		void initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s);
		void newRace(tCarElt* car, tSituation *s);
		void drive(tCarElt* car, tSituation *s);
		int pitCommand(tCarElt* car, tSituation *s);
		void endRace(tCarElt *car, tSituation *s);

};

#endif // _DRIVER_H_

