/***************************************************************************
                                   TRACE                   
                             -------------------                                         
    created              : Fri Aug 13 22:32:45 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: trace.cpp,v 1.7.2.1 2008/11/09 17:50:22 berniw Exp $                                  
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
    Allow the trace in the file <tt>trace.txt</tt>
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: trace.cpp,v 1.7.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	trace
*/


#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <assert.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgf.h>

/* static FILE *outTrace = (FILE*)NULL; */

/* static char TraceStr[1024]; */

void
gfTraceInit(void)
{
}

/* void GfFatal(char *fmt, ...) */
/* { */
/*     va_list ap; */
/*     va_start(ap, fmt); */
/*     GfTrace(fmt, ap); */
/*     va_end(ap); */
/*     exit(1); */
/* } */


/** Print a message in the trace file.
    The file is openned the first time
    @ingroup	trace
    @param	szTrc	message to trace
*/
/* void GfTrace(char *fmt, ...) */
/* { */
/*     va_list		ap; */
/*     struct tm		*stm; */
/*     time_t		t; */
/*     char		*s = TraceStr; */

/*     fprintf(stderr, "ERROR: "); */
/*     va_start(ap, fmt); */
/*     vfprintf(stderr, fmt, ap); */
/*     va_end(ap); */
/*     fflush(stderr); */

/*     if (outTrace == NULL) { */
/* 	if ((outTrace = fopen("trace.txt", "w+")) == NULL) { */
/* 	    perror("trace.txt"); */
/* 	    return; */
/* 	} */
/*     } */
/*     t = time(NULL); */
/*     stm = localtime(&t); */
/*     s += sprintf(TraceStr, "%4d/%02d/%02d %02d:%02d:%02d ", */
/* 		 stm->tm_year+1900, stm->tm_mon+1, stm->tm_mday, */
/* 		 stm->tm_hour, stm->tm_min, stm->tm_sec); */

/*     va_start(ap, fmt); */
/*     vsnprintf(s, 1023 - strlen(TraceStr), fmt, ap); */
/*     va_end(ap); */

/*     fwrite(TraceStr, strlen(TraceStr), 1, outTrace); */
/*     fflush(outTrace); */
/* } */

