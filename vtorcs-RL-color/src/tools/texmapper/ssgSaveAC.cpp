
#include "ssgLocal.h"

struct saveTriangle
{
  int    v [ 3 ] ;
  sgVec2 t [ 3 ] ;
} ;

static FILE *save_fd ;
static sgVec3       *vlist ;
static saveTriangle *tlist ;
static int tcount ;
static int vcount ;

static void getStuff ( ssgEntity *e, sgVec3 *vtab, saveTriangle *ttab )
{
  if ( e -> isAKindOf ( _SSG_TYPE_BRANCH ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      getStuff ( br -> getKid ( i ), vtab, ttab ) ;
  }
  else
  if ( e -> isAKindOf ( _SSG_TYPE_VTXTABLE ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    int nv = vt -> getNumVertices () ;
    int nt = vt -> getNumTriangles () ;
    int sv = vcount ;

    int i ;

    for ( i = 0 ; i < nv ; i++ )
    {
      sgCopyVec3 ( vtab[vcount], vt->getVertex ( i ) ) ;
      vcount++ ;
    }

    for ( i = 0 ; i < nt ; i++ )
    {
      short vv0, vv1, vv2 ;

      vt -> getTriangle ( i, &vv0, &vv1, &vv2 ) ;

      ttab[tcount].v[0]=vv0+sv;
      ttab[tcount].v[1]=vv1+sv;
      ttab[tcount].v[2]=vv2+sv;

      sgCopyVec2 ( ttab[tcount].t[0], vt->getTexCoord ( vv0 ) ) ;
      sgCopyVec2 ( ttab[tcount].t[1], vt->getTexCoord ( vv1 ) ) ;
      sgCopyVec2 ( ttab[tcount].t[2], vt->getTexCoord ( vv2 ) ) ;
      tcount++ ;
    }
  }
}


static int countTriangles ( ssgEntity *e )
{
  int count = 0 ;

  if ( e -> isAKindOf ( _SSG_TYPE_BRANCH ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      count += countTriangles ( br -> getKid ( i ) ) ;
  }
  else
  if ( e -> isAKindOf ( _SSG_TYPE_VTXTABLE ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    count += vt -> getNumTriangles () ;
  }

  return count ;
}



static int countVertices ( ssgEntity *e )
{
  int count = 0 ;

  if ( e -> isAKindOf ( _SSG_TYPE_BRANCH ) )
  {
    ssgBranch *br = (ssgBranch *) e ;

    for ( int i = 0 ; i < br -> getNumKids () ; i++ )
      count += countVertices ( br -> getKid ( i ) ) ;
  }
  else
  if ( e -> isAKindOf ( _SSG_TYPE_VTXTABLE ) )
  {
    ssgVtxTable *vt = (ssgVtxTable *) e ;
    count += vt -> getNumVertices () ;
  }

  return count ;
}


int myssgSaveAC ( const char *filename, ssgEntity *ent, const char *skinfilename )
{
  save_fd = fopen ( filename, "wa" ) ;

  if ( save_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgSaveAC: Failed to open '%s' for writing", filename ) ;
    return FALSE ;
  }

  int num_verts = countVertices  ( ent ) ;
  int num_tris  = countTriangles ( ent ) ;
  vlist = new sgVec3 [ num_verts ] ;
  tlist = new saveTriangle [ num_tris ] ;
  tcount = vcount = 0 ;
  getStuff  ( ent, vlist, tlist ) ;

  fprintf ( save_fd, "AC3Db\n" ) ;
  fprintf ( save_fd, "MATERIAL \"\" rgb 1 1 1  amb 1 1 1  emis 0 0 0  spec 0 0 0  shi 0  trans 0\n" ) ;
  fprintf ( save_fd, "OBJECT world\n" ) ;
  fprintf ( save_fd, "kids 1\n" ) ;
  fprintf ( save_fd, "OBJECT poly\n" ) ;
  fprintf ( save_fd, "name \"%s\"\n", ent->getPrintableName() ) ;
  fprintf ( save_fd, "texture \"%s\"\n", skinfilename);
  fprintf ( save_fd, "numvert %d\n", num_verts ) ;

  int i ;

  for ( i = 0 ; i < num_verts ; i++ )
    fprintf ( save_fd, "%g %g %g\n", vlist[i][0],vlist[i][2],-vlist[i][1] ) ;

  fprintf ( save_fd, "numsurf %d\n", num_tris ) ;

  for ( i = 0 ; i < num_tris ; i++ )
  {
    fprintf ( save_fd, "SURF 0x30\n" ) ;
    fprintf ( save_fd, "mat 0\n" ) ;
    fprintf ( save_fd, "refs 3\n" ) ;
    fprintf ( save_fd, "%d %g %g\n", tlist[i].v[0],tlist[i].t[0][0],tlist[i].t[0][1] ) ;
    fprintf ( save_fd, "%d %g %g\n", tlist[i].v[1],tlist[i].t[1][0],tlist[i].t[1][1] ) ;
    fprintf ( save_fd, "%d %g %g\n", tlist[i].v[2],tlist[i].t[2][0],tlist[i].t[2][1] ) ;
  } 
  fprintf ( save_fd, "kids 0\n" ) ;
  fclose ( save_fd ) ;
  delete[] vlist ;
  delete tlist ;

  return TRUE ;
}


