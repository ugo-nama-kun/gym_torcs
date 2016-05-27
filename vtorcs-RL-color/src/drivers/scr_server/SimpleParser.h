/***************************************************************************
 
    file                 : SimpleParser.h
    copyright            : (C) 2007 Daniele Loiacono
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SIMPLEPARSER_H_
#define SIMPLEPARSER_H_

#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;


class SimpleParser
{
public:
        static bool  parse(string sensors, string tag, float &value);

        static bool  parse(string sensors, string tag, int &value);

        static bool  parse(string sensors, string tag, float *value, int size);

        static string stringify(string tag, float value);

        static string  stringify(string tag, int value);

        static string  stringify(string tag, float *value, int size);

        // GIUSE - stringify images
        static string  stringify(string tag, unsigned char *value, int size); 

};

#endif /*SIMPLEPARSER_H_*/
