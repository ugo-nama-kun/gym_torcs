/***************************************************************************

    file                 : img_server.cpp
    created              : Tue Jun 12 13:41:04 CEST 2012
    copyright            : (C) 2012 Giuseppe Cuccu

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <math.h>
//#include <iostream>
//#include <sstream>
//#include <ctime>

#include <tgf.h> 
#include <track.h> 
#include <car.h> 
#include <raceman.h> 
#include <robottools.h>
#include <robot.h>

#include "driver.h"
#include "img_server.h"

//#include "sensors.h"
#include "../scr_server/SimpleParser.h"
#include "../scr_server/CarControl.h"
//#include "CarControl.h"
//#include "ObstacleSensors.h"


/* 
 * Module entry point  
 */ 
extern "C" int 
img_server(tModInfo *modInfo) 
{
    /* clear all structures */
    memset(modInfo, 0, 10*sizeof(tModInfo));

    for (int i = 0; i < NBBOTS; i++) {
        modInfo[i].name    = botname[i];  /* name of the module (short) */
        modInfo[i].desc    = "";          /* description of the module */
        modInfo[i].fctInit = InitFuncPt;  /* init function */
        modInfo[i].gfId    = ROB_IDENT;   /* supported framework version */
        modInfo[i].index   = i;           /* indices from 0 to 9 */
    }

    return 0; 
} 


/* Module interface initialization. */
static int 
InitFuncPt(int index, void *pt) 
{ 
    tRobotItf *itf  = (tRobotItf *)pt; 

    /* create robot instance for index */
    driver[index] = new Driver(index);
    
    itf->rbNewTrack = initTrack; /* Give the robot the track view called */ 
				 /* for every track change or new race */ 
    itf->rbNewRace  = newrace; 	 /* Start a new race */
    itf->rbDrive    = drive;	 /* Drive during race */
    itf->rbPitCmd   = pitcmd;    /* Pit commands */
    itf->rbEndRace  = endrace;	 /* End of the current race */
    itf->rbShutdown = shutdown;	 /* Called before the module is unloaded */
    itf->index      = index; 	 /* Index used if multiple interfaces */
    return 0;     
} 


/* Called for every track change or new race. */
static void  
initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s) 
{ 
    driver[index]->initTrack(track, carHandle, carParmHandle, s);
} 


/* Start a new race. */
static void  
newrace(int index, tCarElt* car, tSituation *s) 
{ 
    driver[index]->newRace(car, s);

    /***********************************************************************************
    ************************* UDP client identification ********************************
    ***********************************************************************************/

    bool identified=false;
    char line[UDP_MSGLEN];

    // Set timeout
    if (getTimeout()>0)
    	UDP_TIMEOUT = getTimeout();

//    __SENSORS_RANGE__ = 200;

    listenSocket[index] = socket(AF_INET, SOCK_DGRAM, 0);
    if (listenSocket[index] < 0)
    {
        std::cerr << "Error: cannot create listenSocket!";
        exit(1);
    }

    // GIUSE: WTH is that for???
    srand(time(NULL));

    // Bind listen socket to listen port.
    serverAddress[index].sin_family = AF_INET;
    serverAddress[index].sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress[index].sin_port = htons(getUDPListenPort()+index);

    if (bind(listenSocket[index],
             (struct sockaddr *) &serverAddress[index],
             sizeof(serverAddress[index])) < 0)
    {
        std::cerr << "cannot bind socket";
        exit(1);
    }

    // Wait for connections from clients.
    listen(listenSocket[index], 5);

    std::cout << "Waiting for request on port " << getUDPListenPort()+index << "\n";

    clientAddressLength[index] = sizeof(clientAddress[index]);
//    char line[UDP_MSGLEN];

    // Loop until a client identifies correctly
    while (!identified)
    {

        // Set line to all zeroes
        memset(line, 0x0, UDP_MSGLEN);

        if (recvfrom(listenSocket[index], line, UDP_MSGLEN, 0,
                     (struct sockaddr *) &clientAddress[index],
                     &clientAddressLength[index]) < 0)
        {
            std::cerr << "Error: problem in receiving from the listen socket";
            exit(1);
        }

        #ifdef __UDP_SERVER_VERBOSE__
        std::cout << "  from " << inet_ntoa(clientAddress[index].sin_addr); // show the client's IP address
        std::cout << ":" << ntohs(clientAddress[index].sin_port) << "\n";   // show the client's port number.
        std::cout << "  Received: " << line << "\n";                        // show the line
        #endif

        // compare received string with the ID
        if (strncmp(line,UDP_ID,3)==0)
        {
            std::string initStr(line);
            sprintf(line,"***identified***");

//            if (SimpleParser::parse(initStr,std::string("init"),trackSensAngle[index],19)==false)
//            	for (int i = 0; i < 19; ++i)
//            		trackSensAngle[index][i] =-90 + 10*i;

            // Sending the car state to the client
            if (sendto(listenSocket[index], line, strlen(line) + 1, 0,
                       (struct sockaddr *) &clientAddress[index],
                       sizeof(clientAddress[index])) 
                < 0)
                std::cerr << "Error: cannot send identification message";

            identified=true;
        }
    }


    /***************************************************************************************
    ************************* UDP client identification END ********************************
    ***************************************************************************************/

} 


/* Drive during race. */
static void  
drive(int index, tCarElt* car, tSituation *s) 
{ 
    // local variables for UDP
    struct timeval timeVal;
    fd_set readSet;


    /**********************************************************************
     ****************** Building state string *****************************
     **********************************************************************/


    float wheelSpinVel[4];
    for (int i=0; i<4; ++i)
      wheelSpinVel[i] = car->_wheelSpinVel(i);

    if (prevDist[index]<0)
      prevDist[index] = car->race.distFromStartLine;

// GIUSE TODO: add a TOTAL distance from start line

    float curDistRaced = car->race.distFromStartLine - prevDist[index];
    prevDist[index] = car->race.distFromStartLine;

    if (curDistRaced>100)
      curDistRaced -= curTrack->length;

    if (curDistRaced<-100)
      curDistRaced += curTrack->length;

    distRaced[index] += curDistRaced;

    string stateString;

// GIUSE TODO: check if the string is correctly received on the JAVA side (is interpretation parser- or order-based?)

//    stateString =  SimpleParser::stringify( "angle",         angle );
    stateString += SimpleParser::stringify( "curLapTime",    float(car->_curLapTime) );
    stateString += SimpleParser::stringify( "damage",        ( getDamageLimit() ? car->_dammage : car->_fakeDammage ) );
    stateString += SimpleParser::stringify( "distFromStart", car->race.distFromStartLine );
    stateString += SimpleParser::stringify( "distRaced",     distRaced[index] );
    stateString += SimpleParser::stringify( "fuel",          car->_fuel );
    stateString += SimpleParser::stringify( "gear",          car->_gear );
    stateString += SimpleParser::stringify( "lastLapTime",   float(car->_lastLapTime) );
//    stateString += SimpleParser::stringify( "opponents",     oppSensorOut, 36 );
    stateString += SimpleParser::stringify( "racePos",       car->race.pos );
    stateString += SimpleParser::stringify( "rpm",           car->_enginerpm*10 );
    stateString += SimpleParser::stringify( "speedX",        float(car->_speed_x  * 3.6) );
    stateString += SimpleParser::stringify( "speedY",        float(car->_speed_y  * 3.6) );
    stateString += SimpleParser::stringify( "speedZ",        float(car->_speed_z  * 3.6) );
//    stateString += SimpleParser::stringify( "track",         trackSensorOut, 19 );
//    stateString += SimpleParser::stringify( "trackPos",      dist_to_middle );
    stateString += SimpleParser::stringify( "wheelSpinVel",  wheelSpinVel, 4 );
    stateString += SimpleParser::stringify( "z",             car->_pos_Z  - RtTrackHeightL(&(car->_trkPos)) );
//    stateString += SimpleParser::stringify( "focus",         focusSensorOut, 5 );//ML

    char line[UDP_MSGLEN];
    sprintf(line,"%s",stateString.c_str());


// END STATE STRING


    // Sending the car state to the client
    if (sendto(listenSocket[index], line, strlen(line) + 1, 0,
               (struct sockaddr *) &clientAddress[index],
               sizeof(clientAddress[index])) < 0)
        std::cerr << "Error: cannot send car state";


    // Set timeout for client answer
    FD_ZERO(&readSet);
    FD_SET(listenSocket[index], &readSet);
    timeVal.tv_sec = 0;
    timeVal.tv_usec = UDP_TIMEOUT;
    memset(line, 0x0,1000 );

    if (select(listenSocket[index]+1, &readSet, NULL, NULL, &timeVal))
    {  // Read the client controller action

      memset(line, 0x0,UDP_MSGLEN );  // Zero the buffer
      int numRead = recv(listenSocket[index], line, UDP_MSGLEN, 0);
      if (numRead < 0)
      {
        std::cerr << "Error, cannot get any response from the client!";
        CLOSE(listenSocket[index]);
        exit(1);
      }

#ifdef __UDP_SERVER_VERBOSE__
      std::cout << "Received: " << line << std::endl;
#endif

      std::string lineStr(line);
      CarControl carCtrl(lineStr);
      if (carCtrl.getMeta()==RACE_RESTART)
      {
        RESTARTING[index] = 1;
#ifdef __DISABLE_RESTART__
        char line[UDP_MSGLEN];
        sprintf(line,"***restart***");
        // Sending the car state to the client
        if ( sendto( listenSocket[index], line, strlen(line) + 1, 0,
                     (struct sockaddr *) &clientAddress[index],
                   	 sizeof(clientAddress[index])) 
              < 0)
          std::cerr << "Error: cannot send restart message";
#else
         car->RESTART=1;
#endif
       }

        // Set controls command and store them in variables
        oldAccel[index]  = car->_accelCmd  = carCtrl.getAccel();
        oldBrake[index]  = car->_brakeCmd  = carCtrl.getBrake();
        oldGear[index]   = car->_gearCmd   = carCtrl.getGear();
        oldSteer[index]  = car->_steerCmd  = carCtrl.getSteer();
        oldClutch[index] = car->_clutchCmd = carCtrl.getClutch();
        oldFocus[index]  = car->_focusCmd  = carCtrl.getFocus();//ML
    } // END OF Read the client controller action
    else
    { // Server timeout, use old controls for another timestep
        std::cout << "Timeout for client answer\n";
        car->_accelCmd = oldAccel[index];
        car->_brakeCmd = oldBrake[index];
        car->_gearCmd  = oldGear[index];
        car->_steerCmd = oldSteer[index];
        car->_clutchCmd = oldClutch[index];
        car->_focusCmd = oldFocus[index];//ML
    }

    // Our basic driver
//    driver[index]->drive(car, s);
}


/* Pitstop callback */
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
    return driver[index]->pitCommand(car, s);
}


/* End of the current race */
static void
endrace(int index, tCarElt *car, tSituation *s)
{
    driver[index]->endRace(car, s);
}


/* Called before the module is unloaded */
static void
shutdown(int index)
{
  free(botname[index]);
  delete driver[index];
}



