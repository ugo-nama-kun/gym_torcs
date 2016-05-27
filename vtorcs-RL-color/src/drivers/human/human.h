/***************************************************************************

    file        : human.h
    created     : Sat May 10 19:12:46 CEST 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: human.h,v 1.3.2.1 2008/06/01 07:42:14 berniw Exp $                                  

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: human.h,v 1.3.2.1 2008/06/01 07:42:14 berniw Exp $
*/

#ifndef _HUMAN_H_
#define _HUMAN_H_

typedef struct HumanContext
{
	int		NbPitStops;
	int		LastPitStopLap;
	int 	AutoReverseEngaged;
	tdble	shiftThld[MAX_GEARS+1];
	tdble	Gear;
	tdble	distToStart;
	tdble	clutchtime;
	tdble	clutchdelay;
	tdble	ABS;
	tdble	AntiSlip;
	int		lap;
	tdble	prevLeftSteer;
	tdble	prevRightSteer;
	tdble	paccel;
	tdble	pbrake;
	int		manual;
	int		Transmission;
	int		NbPitStopProg;
	int		ParamAsr;
	int		ParamAbs;
	int		RelButNeutral;
	int		SeqShftAllowNeutral;
	int		AutoReverse;
	int		drivetrain;
	int		autoClutch;
	tControlCmd	*CmdControl;
	int		MouseControlUsed;
	int		lightCmd;
} tHumanContext;


extern tHumanContext *HCtx[];

extern int joyPresent;

#endif /* _HUMAN_H_ */ 



