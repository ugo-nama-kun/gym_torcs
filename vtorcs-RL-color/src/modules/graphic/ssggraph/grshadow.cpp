/***************************************************************************

    file                 : grshadow.cpp
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grshadow.cpp,v 1.14 2005/06/03 23:51:20 berniw Exp $

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
#include "grshadow.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

void ssgVtxTableShadow::copy_from ( ssgVtxTableShadow *src, int clone_flags )
{
  ssgVtxTable::copy_from ( src, clone_flags ) ;
  factor=src->factor;
  unit=src->unit;
}
ssgBase *ssgVtxTableShadow::clone ( int clone_flags )
{
  ssgVtxTableShadow *b = new ssgVtxTableShadow ;
  b -> copy_from ( this, clone_flags ) ;
  return b ;
}
ssgVtxTableShadow::ssgVtxTableShadow ()
{
  ssgVtxTableShadow(0,0);
}



ssgVtxTableShadow::ssgVtxTableShadow (float f, float u) : ssgVtxTable(), factor(f), unit(u)
{
  /*factor=f;
  unit=u;
  ssgVtxTable();*/
}
ssgVtxTableShadow::ssgVtxTableShadow ( GLenum ty, ssgVertexArray   *vl,
				       ssgNormalArray   *nl,
				       ssgTexCoordArray *tl,
				       ssgColourArray   *cl ) : ssgVtxTable( ty, vl, nl, tl, cl )
{
  type = ssgTypeVtxTable () ;
  factor=0;
  unit=0;
}

ssgVtxTableShadow::~ssgVtxTableShadow ()
{
  /*  ssgDeRefDelete ( vertices  ) ;
      ssgDeRefDelete ( normals   ) ;
      ssgDeRefDelete ( texcoords ) ;
      ssgDeRefDelete ( colours   ) ; */
} 


void ssgVtxTableShadow::setOffset(float f, float u)
{
  factor=f;
  unit=f;
}



void ssgVtxTableShadow::draw_geometry ()
{
  int num_colours   = getNumColours   () ;
  int num_normals   = getNumNormals   () ;
  int num_vertices  = getNumVertices  () ;
  int num_texcoords = getNumTexCoords () ;


  sgVec3 *vx = (sgVec3 *) vertices  -> get(0) ;
  sgVec3 *nm = (sgVec3 *) normals   -> get(0) ;
  sgVec2 *tx = (sgVec2 *) texcoords -> get(0) ;
  sgVec4 *cl = (sgVec4 *) colours   -> get(0) ;

  glDepthMask(GL_FALSE);
  glPolygonOffset(-15.0f, -20.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);
  //glEnable(GL_CULL_FACE);

  glBegin ( gltype ) ;

  if ( num_colours == 0 ) glColor4f   ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  if ( num_colours == 1 ) glColor4fv  ( cl [ 0 ] ) ;
  if ( num_normals == 1 ) glNormal3fv ( nm [ 0 ] ) ;

  for ( int i = 0 ; i < num_vertices ; i++ )
  {
    if ( num_colours   > 1 ) glColor4fv    ( cl [ i ] ) ;
    if ( num_normals   > 1 ) glNormal3fv   ( nm [ i ] ) ;
    if ( num_texcoords > 1 ) glTexCoord2fv ( tx [ i ] ) ;

    glVertex3fv ( vx [ i ] ) ;
  }
 
  glEnd () ;
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDepthMask(GL_TRUE);
}
