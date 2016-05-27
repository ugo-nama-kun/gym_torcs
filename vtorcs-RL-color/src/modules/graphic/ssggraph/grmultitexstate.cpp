/***************************************************************************

    file                 : grmultitexstate.cpp
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grmultitexstate.cpp,v 1.4 2004/11/26 15:37:47 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <plib/ssg.h>
#include "grmultitexstate.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif
#include "grmain.h"

void grMultiTexState::apply (int unit)
{
  if (unit==0) {
      glActiveTextureARB ( GL_TEXTURE0_ARB ) ;
      glEnable ( GL_TEXTURE_2D ) ;  /* Enables the second texture map. */
      glBindTexture ( GL_TEXTURE_2D, ssgSimpleState::getTextureHandle() ) ;
  } else if (unit==1) {
      glActiveTextureARB ( GL_TEXTURE1_ARB ) ;
      glEnable ( GL_TEXTURE_2D ) ;  /* Enables the second texture map. */
      /* glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);*/
      /*glBlendFunc(GL_ZERO, GL_SRC_COLOR);*/
      glBindTexture ( GL_TEXTURE_2D, ssgSimpleState::getTextureHandle() ) ;
  } else if (unit==2) {
      glActiveTextureARB ( GL_TEXTURE2_ARB ) ;
      glEnable ( GL_TEXTURE_2D ) ;  /* Enables the second texture map. */
      glBindTexture ( GL_TEXTURE_2D, ssgSimpleState::getTextureHandle() ) ;
  } else if (unit==3) {
      glActiveTextureARB ( GL_TEXTURE3_ARB ) ;
      glEnable ( GL_TEXTURE_2D ) ;  /* Enables the second texture map. */
      glBindTexture ( GL_TEXTURE_2D, ssgSimpleState::getTextureHandle() ) ;
  } else {
      /*glActiveTextureARB ( GL_TEXTURE0_ARB ) ;*/
      glBindTexture ( GL_TEXTURE_2D, getTextureHandle() ) ;
      _ssgCurrentContext->getState()->setTexture ( getTexture () ) ;  
  }
}
