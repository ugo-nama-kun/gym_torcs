/***************************************************************************

    file                 : grvtxtable.h
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
    version              : $Id: grvtxtable.h,v 1.5 2005/02/01 19:08:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* 

   This class is based on the plib ssgVtxTable and the ssgVtxArray class 

*/
/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 2001  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 
     For further information visit http://plib.sourceforge.net
*/

#ifndef _GRVTXTABLE_H_
#define _GRVTXTABLE_H_
#include "grmultitexstate.h"

#define LEVELC  -1     /* a normal map in LEVEL0 and an env map in LEVEL1 for cars
			  texturecoord of the env map are computed by opengl      */
#define LEVELC2  -2     /* a normal map in LEVEL0 and an env map in LEVEL2 for cars
			   precomputed and an env map computed by opengl in LEVEL1  */
#define LEVELC3  -3     /* a normal map in LEVEL0 and an env map in LEVEL2 for cars
			   precomputed and an shadow env  LEVEL1  */
#define LEVEL0  1      /* the normal map level      */
#define LEVEL1  2      /* the high tiled map        */
#define LEVEL2  4      /* skids and lummy texture   */
#define LEVEL3  8      /* shadows and other goodies */



class grVtxTable : public ssgVtxTable
{
  grMultiTexState *state1 ;
  grMultiTexState *state2 ;
  grMultiTexState *state3 ;

#define ARRAY 1
#define TABLE 2
  int internalType;

protected:
  virtual void copy_from ( grVtxTable *src, int clone_flags ) ;
  ssgIndexArray      *indices;
  ssgIndexArray      *stripes;
  int                 numStripes;
  /*virtual void draw_geometry () ;*/
  /*sgVec2 *texcoords1 ; int num_texcoords1 ; unsigned short *t_index1 ;
    sgVec2 *texcoords2 ; int num_texcoords2 ; unsigned short *t_index2 ;
    sgVec2 *texcoords3 ; int num_texcoords3 ; unsigned short *t_index3 ;*/
_SSG_PUBLIC:  
  ssgTexCoordArray *texcoords1 ;
  ssgTexCoordArray *texcoords2 ;
  ssgTexCoordArray *texcoords3 ;
    
public:
  int numMapLevel;
  int mapLevelBitmap;
  int indexCar;
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  /*grVtxTable () ;*/
  grVtxTable (int _numMapLevel,int _mapLevel) ;
  grVtxTable ( GLenum ty, ssgVertexArray   *vl,
                           ssgNormalArray   *nl,
                           ssgTexCoordArray *tl,
                           ssgTexCoordArray *tl1,
                           ssgTexCoordArray *tl2,
                           ssgTexCoordArray *tl3,
		           int _numMapLevel,
                           int _mapLevel,
                           ssgColourArray   *cl, 
	                   int _indexCar ) ;
  grVtxTable ( GLenum ty, ssgVertexArray   *vl,
	       ssgIndexArray    * stripeIndex,
	       int _numstripes,
	       ssgIndexArray    *il,
	       ssgNormalArray   *nl,
	       ssgTexCoordArray *tl,
	       ssgTexCoordArray *tl1,
	       ssgTexCoordArray *tl2,
	       ssgTexCoordArray *tl3,
	       int _numMapLevel,
	       int _mapLevel,
	       ssgColourArray   *cl, 
	       int _indexCar ) ;
  grVtxTable (ssgVertexArray	*shd_vtx , float initsize, int type);

  float *getTexCoord1(int i){ if(i>=getNumTexCoords())i=getNumTexCoords()-1;
                             return (getNumTexCoords()<=0) ?
                                    _ssgTexCoord00  : texcoords1->get(i);}
  float *getTexCoord2(int i){ if(i>=getNumTexCoords())i=getNumTexCoords()-1;
                             return (getNumTexCoords()<=0) ?
                                    _ssgTexCoord00  : texcoords2->get(i);}
  float *getTexCoord3(int i){ if(i>=getNumTexCoords())i=getNumTexCoords()-1;
                             return (getNumTexCoords()<=0) ?
                                    _ssgTexCoord00  : texcoords3->get(i);}

  ssgState *getState1 () { return state1 ; }
  void      setState1 ( ssgState *st );
  ssgState *getState2 () { return state2 ; }
  void      setState2 ( ssgState *st );
  ssgState *getState3 () { return state3 ; }
  void      setState3 ( ssgState *st );

  virtual void draw () ;
  /*void grVtxTable::draw ();*/
  void draw_geometry_array();
  void draw_geometry_multi();
  void draw_geometry_for_a_car () ;
  void draw_geometry_multi_array();
  void draw_geometry_for_a_car_array () ;
  virtual void drawHighlight ( sgVec4 colour ){ssgVtxTable::drawHighlight(colour);}  /* doesn't not work */
  virtual void drawHighlight ( sgVec4 colour, int i ){ssgVtxTable::drawHighlight(colour,i);} /* doesn't work also */

  virtual void pick ( int baseName )  { ssgVtxTable::pick(baseName);}
  virtual void transform ( const sgMat4 m )  { ssgVtxTable::transform(m);}


  int getNumVertices  () { return vertices  -> getNum () ; }
  int getNumNormals   () { return normals   -> getNum () ; }
  int getNumColours   () { return colours   -> getNum () ; }
  int getNumTexCoords () { return texcoords -> getNum () ; }

  /* the following functions doesn't work with arrays */
  int getNumTriangles ()  { return ssgVtxTable::getNumTriangles();}
  void getTriangle ( int n, short *v1, short *v2, short *v3 )  { ssgVtxTable::getTriangle(n,v1,v2,v3);}
  int  getNumLines ()  {return ssgVtxTable::getNumLines();}
  void getLine ( int n, short *v1, short *v2 )  { ssgVtxTable::getLine(n,v1,v2);}


  virtual ~grVtxTable (void);

  virtual const char *getTypeName(void)  { return ssgVtxTable::getTypeName();}

  virtual void setVertices  ( ssgVertexArray   *vl ) {  ssgVtxTable::setVertices(vl);}
  virtual void setNormals   ( ssgNormalArray   *nl ) {  ssgVtxTable::setNormals(nl);}
  virtual void setTexCoords ( ssgTexCoordArray *tl ) {  ssgVtxTable::setTexCoords(tl);}
  virtual void setColours   ( ssgColourArray   *cl ) {  ssgVtxTable::setColours(cl);}

  /* the following functions doesn't work with arrays */
  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2) { ssgVtxTable::print(fd,indent,how_much);}
  virtual int load ( FILE *fd )  {return  ssgVtxTable::load(fd);}
  virtual int save ( FILE *fd )  {return  ssgVtxTable::save(fd);}
};


#endif /* _GRVTXTABLE_H_*/
