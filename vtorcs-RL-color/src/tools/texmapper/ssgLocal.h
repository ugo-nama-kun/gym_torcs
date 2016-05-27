
#define _SSG_PUBLIC  public

#include <plib/ssg.h>

extern void (*__ssgEnableTable[64])() ;
extern void (*__ssgDisableTable[64])() ;

extern char  *_ssgModelPath ;
extern char  *_ssgTexturePath ;

extern sgMat4 _ssgOpenGLAxisSwapMatrix ;
extern int    _ssgIsHotTest ;
extern int    _ssgFileVersionNumber ;

void _ssgForceLineState () ;

void _ssgDrawDList () ;
void _ssgPushMatrix ( sgMat4 m ) ;
void _ssgPopMatrix  () ;
void _ssgLoadMatrix ( sgMat4 m ) ;
void _ssgLoadTexMatrix ( sgMat4 m ) ;
void _ssgUnloadTexMatrix () ;
void _ssgDrawLeaf   ( ssgLeaf *l ) ;

void _ssgAddHit ( ssgLeaf *l, int trinum, sgMat4 mat, sgVec4 pl ) ;
void _ssgPushPath ( ssgEntity *l ) ;
void _ssgPopPath () ;

extern int stats_num_vertices    ;
extern int stats_num_leaves      ;
extern int stats_isect_triangles ;
extern int stats_cull_test       ;
extern int stats_isect_test      ;
extern int stats_bind_textures   ;

extern int stats_hot_triangles   ;
extern int stats_hot_test        ;
extern int stats_hot_no_trav     ;
extern int stats_hot_radius_reject ;
extern int stats_hot_triv_accept ;
extern int stats_hot_straddle    ;

extern ssgState *( *_ssgGetAppState)( char *) ;

void _ssgReadFloat   ( FILE *fd,                float *var ) ;
void _ssgWriteFloat  ( FILE *fd, const          float  var ) ;
void _ssgReadUInt    ( FILE *fd,       unsigned int   *var ) ;
void _ssgWriteUInt   ( FILE *fd, const unsigned int    var ) ;
void _ssgReadInt     ( FILE *fd,                int   *var ) ;
void _ssgWriteInt    ( FILE *fd, const          int    var ) ;
void _ssgReadUShort  ( FILE *fd,       unsigned short *var ) ;
void _ssgWriteUShort ( FILE *fd, const unsigned short  var ) ;
void _ssgReadShort   ( FILE *fd,                short *var ) ;
void _ssgWriteShort  ( FILE *fd, const          short  var ) ;

void _ssgReadFloat   ( FILE *fd, const unsigned int n,                float *var ) ;
void _ssgWriteFloat  ( FILE *fd, const unsigned int n, const          float *var ) ;
void _ssgReadUInt    ( FILE *fd, const unsigned int n,       unsigned int   *var ) ;
void _ssgWriteUInt   ( FILE *fd, const unsigned int n, const unsigned int   *var ) ;
void _ssgReadInt     ( FILE *fd, const unsigned int n,                int   *var ) ;
void _ssgWriteInt    ( FILE *fd, const unsigned int n, const          int   *var ) ;
void _ssgReadUShort  ( FILE *fd, const unsigned int n,       unsigned short *var ) ;
void _ssgWriteUShort ( FILE *fd, const unsigned int n, const unsigned short *var ) ;
void _ssgReadShort   ( FILE *fd, const unsigned int n,                short *var ) ;
void _ssgWriteShort  ( FILE *fd, const unsigned int n, const          short *var ) ;

void _ssgReadString  ( FILE *fd,       char **var ) ;
void _ssgWriteString ( FILE *fd, const char  *var ) ;

void _ssgReadVec2    ( FILE *fd, sgVec2 var ) ;
void _ssgWriteVec2   ( FILE *fd, const sgVec2 var ) ;
void _ssgReadVec3    ( FILE *fd, sgVec3 var ) ;
void _ssgWriteVec3   ( FILE *fd, const sgVec3 var ) ;
void _ssgReadVec4    ( FILE *fd, sgVec4 var ) ;
void _ssgWriteVec4   ( FILE *fd, const sgVec4 var ) ;

void _ssgReadMat4    ( FILE *fd, sgMat4 var ) ;
void _ssgWriteMat4   ( FILE *fd, const sgMat4 var ) ;

int _ssgReadError    ( void ) ;
int _ssgWriteError   ( void ) ;

int _ssgStrNEqual ( const char *s1, const char *s2, int len ) ;

ssgBase *_ssgGetFromList ( int key ) ;
void     _ssgAddToList   ( int key, ssgBase *b ) ;
int      _ssgGetNextInstanceKey  () ;

