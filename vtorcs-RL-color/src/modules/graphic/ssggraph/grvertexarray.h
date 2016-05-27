class ssgGrVtxArray : public ssgVtxArray
{
protected:
  ssgIndexArray      *indices;

  virtual void draw_geometry () ;
  virtual void copy_from ( ssgGrVtxArray *src, int clone_flags ) ;

public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgGrVtxArray () ;
  ssgGrVtxArray ( GLenum ty, ssgVertexArray   *vl,
                           ssgNormalArray   *nl,
                           ssgTexCoordArray *tl,
                           ssgColourArray   *cl,
		           ssgIndexArray    *il ) ;

  virtual void drawHighlight ( sgVec4 colour ) ;
  virtual void drawHighlight ( sgVec4 colour, int i ) ;
  virtual void pick ( int baseName ) ;

  void setIndices ( ssgIndexArray *il ) ;

  int getNumIndices () { return indices -> getNum () ; }

  int getNumTriangles () { return ssgVtxArray::getNumTriangles();}
  void getTriangle ( int n, short *v1, short *v2, short *v3 )  { ssgVtxArray::getTriangle(n,v1,v2,v3);}

  int  getNumLines () {return ssgVtxArray::getNumLines();}
  void getLine ( int n, short *v1, short *v2 ) { ssgVtxArray::getLine(n,v1,v2);}

  void getIndexList ( void **list ) { *list = indices  -> get ( 0 ) ; }

  short *getIndex  (int i){ if(i>=getNumIndices())i=getNumIndices()-1;
                             return (getNumIndices()<=0) ?
				      &_ssgIndex0 : indices->get(i);}

  virtual ~ssgVtxArray (void) ;

  virtual char *getTypeName(void) { return ssgVtxArray::getTypeName();}

  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2 ) { ssgVtxArray::print(fd,indent,how_much);}
  virtual int load ( FILE *fd ) {return  ssgVtxArray::load(fd);}
  virtual int save ( FILE *fd ) {return  ssgVtxArray::save(fd);}
} ;
