/***************************************************************************

    file        : grcarlight.h
    created     : Sun Oct 26 15:15:11 CET 2003
    copyright   : (C) 2003 by Christophe Guionneau                      
    version     : $Id: grcarlight.h,v 1.4.2.1 2008/08/24 17:07:23 berniw Exp $                                  

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
    		
    @version	$Id: grcarlight.h,v 1.4.2.1 2008/08/24 17:07:23 berniw Exp $
*/

#ifndef _GRCARLIGHT_H_
#define _GRCARLIGHT_H_

class ssgVtxTableCarlight : public ssgVtxTable
{
protected:
  int on ;
  float size;
  sgVec3 pos;
  double factor;
    
  virtual void copy_from ( ssgVtxTableCarlight *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgVtxTableCarlight () ;
  ssgVtxTableCarlight ( ssgVertexArray   *vtx,
			double s, sgVec3 p);
  int setSize (double s) { size=s ; return 0; }
  void setOnOff (int s) { on=s ; }
  int isOnOff () { return(on) ; }
  sgVec3 * getPos() { return(&pos);}
  void setFactor(double f){factor=f;}

  void draw_geometry();
  

  virtual void drawHighlight ( sgVec4 colour ){ssgVtxTable::drawHighlight(colour);} 
  virtual void drawHighlight ( sgVec4 colour, int i ){ssgVtxTable::drawHighlight(colour,i);} 

  virtual void pick ( int baseName )  { ssgVtxTable::pick(baseName);}
  virtual void transform ( const sgMat4 m )  { ssgVtxTable::transform(m);}

  virtual void setVertices  ( ssgVertexArray   *vl ) {  ssgVtxTable::setVertices(vl);}
  virtual void setNormals   ( ssgNormalArray   *nl ) {  ssgVtxTable::setNormals(nl);}
  virtual void setTexCoords ( ssgTexCoordArray *tl ) {  ssgVtxTable::setTexCoords(tl);}
  virtual void setColours   ( ssgColourArray   *cl ) {  ssgVtxTable::setColours(cl);}

  int getNumVertices  () { return vertices  -> getNum () ; }
  int getNumNormals   () { return normals   -> getNum () ; }
  int getNumColours   () { return colours   -> getNum () ; }
  int getNumTexCoords () { return texcoords -> getNum () ; }
  int getNumTriangles ()  { return ssgVtxTable::getNumTriangles();}
  void getTriangle ( int n, short *v1, short *v2, short *v3 )  { ssgVtxTable::getTriangle(n,v1,v2,v3);}
  int  getNumLines ()  {return ssgVtxTable::getNumLines();}
  void getLine ( int n, short *v1, short *v2 )  { ssgVtxTable::getLine(n,v1,v2);}


  virtual ~ssgVtxTableCarlight (void);

  virtual const char *getTypeName(void)  { return ssgVtxTable::getTypeName();}

  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2) { ssgVtxTable::print(fd,indent,how_much);}
  virtual int load ( FILE *fd )  {return  ssgVtxTable::load(fd);}
  virtual int save ( FILE *fd )  {return  ssgVtxTable::save(fd);}


};

#define MAX_NUMBER_LIGHT 4+4+4+2
/*
   4 front 
   4 rear
   4 break
   2 reverse
 */
#define LIGHT_TYPE_FRONT		1
#define LIGHT_TYPE_FRONT2		2
#define LIGHT_TYPE_REAR			3
#define LIGHT_TYPE_REAR2		4
#define LIGHT_TYPE_BRAKE		5
#define LIGHT_TYPE_BRAKE2		6
#define LIGHT_TYPE_REVERSE		7
#define LIGHT_NO_TYPE			0

typedef struct tgrCarlight_t
{
  ssgVtxTableCarlight * lightArray[MAX_NUMBER_LIGHT];
  ssgVtxTableCarlight * lightCurr[MAX_NUMBER_LIGHT];
  int                  lightType[MAX_NUMBER_LIGHT];
  int numberCarlight;
  ssgBranch *lightAnchor;
}tgrCarlight;

extern void grInitCarlight(int index); /* number of cars*/
extern void grAddCarlight(tCarElt *car, int type, sgVec3 pos, double size);
extern void grLinkCarlights(tCarElt *car);

extern void grUpdateCarlight(tCarElt *car, class cGrPerspCamera *curCam, int dispflag);
extern void grShudownCarlight(void);


#endif /* _GRCARLIGHT_H_ */ 
