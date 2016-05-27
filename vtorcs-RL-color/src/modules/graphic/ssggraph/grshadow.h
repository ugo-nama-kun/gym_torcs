class ssgVtxTableShadow : public ssgVtxTable
{
protected:
  float factor;
  float unit;
  virtual void copy_from ( ssgVtxTableShadow *src, int clone_flags ) ;
public:
  virtual ssgBase *clone ( int clone_flags = 0 ) ;
  ssgVtxTableShadow (float f, float u) ;
  ssgVtxTableShadow () ;
  ssgVtxTableShadow ( GLenum ty, ssgVertexArray   *vl,
                           ssgNormalArray   *nl,
                           ssgTexCoordArray *tl,
                           ssgColourArray   *cl ) ;
  void setOffset(float f, float u);
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


  virtual ~ssgVtxTableShadow (void);

  virtual const char *getTypeName(void)  { return ssgVtxTable::getTypeName();}

  virtual void print ( FILE *fd = stderr, char *indent = "", int how_much = 2) { ssgVtxTable::print(fd,indent,how_much);}
  virtual int load ( FILE *fd )  {return  ssgVtxTable::load(fd);}
  virtual int save ( FILE *fd )  {return  ssgVtxTable::save(fd);}


};
