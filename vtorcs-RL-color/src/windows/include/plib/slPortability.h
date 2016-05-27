/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net

     $Id: slPortability.h,v 1.3 2005/02/01 15:36:05 berniw Exp $
*/


#ifndef __SLPORTABILITY_H__
#define __SLPORTABILITY_H__ 1

/* ------------------------------------------------------------- */
/* OS specific includes and defines ...                          */
/* ------------------------------------------------------------- */

#include "ul.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef UL_MACINTOSH
  #include <Sound.h>
  #include <Timer.h>
  #ifdef __MWERKS__
  #include <unix.h>
  #endif
#endif

#ifdef UL_MAC_OSX
  #include <Carbon/Carbon.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#if (defined(UL_LINUX) || defined(UL_BSD)) && !defined(__NetBSD__)
#define SL_USING_OSS_AUDIO 1
#endif

#ifdef SL_USING_OSS_AUDIO
#  if defined(UL_LINUX)
#    include <linux/soundcard.h>
#    include <sys/ioctl.h>
#  elif defined(__FreeBSD__)
#    include <machine/soundcard.h>
#  else
    /*
      Tom thinks this file may be <sys/soundcard.h> under some
      unixen - but that isn't where the OSS manuals say it
      should be.

      If you ever find out the truth, please email me:
       Steve Baker <sjbaker1@airmail.net>
    */
#    include <soundcard.h>
#  endif
#endif

#ifdef UL_BSD
#ifndef __FreeBSD__
#  include <sys/audioio.h>
#endif
#endif

/* Tom */

#ifdef UL_IRIX
#  include <audio.h>
#endif

#ifdef UL_SOLARIS
#  include <sys/audioio.h>
#  include <sys/stropts.h>
#endif

#endif

