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

     $Id: grloadac.cpp,v 1.20 2005/07/21 21:27:15 berniw Exp $
*/

#include <plib/ssg.h>
#include <zlib.h>

#include "grssgext.h"
#include "grvtxtable.h"
#include "grmultitexstate.h"
#include "grmain.h"
#include "grtexture.h"
int inGroup=0;


#define FGETS(buf, len, file) gzgets(file, buf, len)
#define FGETC(file) gzgetc(file)
#define FOPEN(path, mode) gzopen(path, mode)
#define FCLOSE(fd) gzclose(fd)

static double t_xmax;
static double t_ymax;
static double t_xmin;
static double t_ymin;

double shad_xmax;
double shad_ymax;
double shad_xmin;
double shad_ymin;
double carTrackRatioX=0;
double carTrackRatioY=0;


static gzFile loader_fd ;


struct _ssgMaterial
{
  sgVec4 spec ;
  sgVec4 emis ;
  sgVec4 amb ;
  sgVec4 rgb  ;
  float  shi  ;
} ;

static int		num_materials = 0 ;
static sgVec3		*vtab  = NULL ;
static sgVec3		*ntab  = NULL ;
static sgVec2           *t0tab = NULL;
static sgVec2           *t1tab = NULL;
static sgVec2           *t2tab = NULL;
static sgVec2           *t3tab = NULL;
static ssgIndexArray    *vertlist=NULL;
static ssgIndexArray    *striplist=NULL;
static int              totalnv=0;
static int              totalstripe=0;
static int		usenormal = 0;
static int		nv;
static int		isacar = TRUE;
static int              usestrip=TRUE;
static int              usegroup=TRUE;
static int              mapLevel;
static int              numMapLevel;
static int              indexCar;

static int		isaWindow;

static ssgLoaderOptions	*current_options  = NULL ;
static _ssgMaterial	*current_material = NULL ;
static sgVec4		*current_colour   = NULL ;
static ssgBranch	*current_branch   = NULL ;
static char		*current_tfname   = NULL ;
static char		*current_tbase    = NULL ;
static char		*current_ttiled   = NULL ;
static char		*current_tskids   = NULL ;
static char		*current_tshad    = NULL ;
static char		*current_data     = NULL ;
#define NOTEXTURE "empty_texture_no_mapping"
#define MAX_MATERIALS 1000    /* This *ought* to be enough! */
static _ssgMaterial   *mlist    [ MAX_MATERIALS ] ;
static sgVec4         *clist    [ MAX_MATERIALS ] ;

static sgMat4 current_matrix ;
static sgVec2 texrep ;
static sgVec2 texoff ;

static int do_material ( char *s ) ;
static int do_object   ( char *s ) ;
static int do_name     ( char *s ) ;
static int do_data     ( char *s ) ;
static int do_texture  ( char *s ) ;
static int do_texrep   ( char *s ) ;
static int do_texoff   ( char *s ) ;
static int do_rot      ( char *s ) ;
static int do_loc      ( char *s ) ;
static int do_url      ( char *s ) ;
static int do_numvert  ( char *s ) ;
static int do_numsurf  ( char *s ) ;
static int do_surf     ( char *s ) ;
static int do_mat      ( char *s ) ;
static int do_refs     ( char *s ) ;
static int do_kids     ( char *s ) ;

static int do_obj_world ( char *s ) ;
static int do_obj_poly  ( char *s ) ;
static int do_obj_group ( char *s ) ;
static int do_obj_light ( char *s ) ;
/*static void myssgStripify ( ssgEntity *ent );*/
#define PARSE_CONT   0
#define PARSE_POP    1

struct Tag
{
  char *token ;
  int (*func) ( char *s ) ;
} ;

 
static void skip_spaces ( char **s )
{
  while ( **s == ' ' || **s == '\t' )
    (*s)++ ;
}


static void skip_quotes ( char **s )
{
  skip_spaces ( s ) ;

  if ( **s == '\"' )
  {
    (*s)++ ;

    char *t = *s ;

    while ( *t != '\0' && *t != '\"' )
      t++ ;

    if ( *t != '\"' )
      ulSetError ( UL_WARNING, "ac_to_gl: Mismatched double-quote ('\"') in '%s'", *s ) ;

    *t = '\0' ;
  }
  else
    ulSetError ( UL_WARNING, "ac_to_gl: Expected double-quote ('\"') in '%s'", *s ) ;
}



static int search ( Tag *tags, char *s )
{
  skip_spaces ( & s ) ;

  for ( int i = 0 ; tags[i].token != NULL ; i++ )
    if ( ulStrNEqual ( tags[i].token, s, strlen(tags[i].token) ) )
    {
      s += strlen ( tags[i].token ) ;

      skip_spaces ( & s ) ;

      return (*(tags[i].func))( s ) ;
    }

  ulSetError ( UL_FATAL, "ac_to_gl: Unrecognised token '%s' (%d)", s , strlen(s)) ;

  return 0 ;  /* Should never get here */
}

static Tag top_tags [] =
{
  { "MATERIAL", do_material },
  { "OBJECT"  , do_object   },
} ;


static Tag object_tags [] =
{
  { "name"    , do_name     },
  { "data"    , do_data     },
  { "texture" , do_texture  },
  { "texrep"  , do_texrep   },
  { "texoff"  , do_texoff   },
  { "rot"     , do_rot      },
  { "loc"     , do_loc      },
  { "url"     , do_url      },
  { "numvert" , do_numvert  },
  { "numsurf" , do_numsurf  },
  { "kids"    , do_kids     },
  { NULL, NULL }
} ;

static Tag surf_tag [] =
{
  { "SURF"    , do_surf     },
  { NULL, NULL }
} ;

static Tag surface_tags [] =
{
  { "mat"     , do_mat      },
  { "refs"    , do_refs     },
  { NULL, NULL }
} ;

static Tag obj_type_tags [] = 
 { 
   { "world", do_obj_world }, 
   { "poly" , do_obj_poly  }, 
   { "group", do_obj_group }, 
   { "light", do_obj_light }, 
   { NULL, NULL } 
 } ; 

#define OBJ_WORLD  0
#define OBJ_POLY   1
#define OBJ_GROUP  2
#define OBJ_LIGHT  3

static int do_obj_world ( char * ) { return OBJ_WORLD ; } 
static int do_obj_poly  ( char * ) { return OBJ_POLY  ; }
static int do_obj_group ( char * ) { return OBJ_GROUP ; }
static int do_obj_light ( char * ) { return OBJ_LIGHT ; }
static ssgEntity *myssgLoadAC ( const char *fname, const ssgLoaderOptions* options );


static int last_num_kids    = -1 ;
static int current_flags    = -1 ;

static ssgState *get_state ( _ssgMaterial *mat )
{
#ifdef EEE_PAS_COMPRIS
#warning: HELLO ---------------------
  if (current_tfname != NULL) {
    ssgState *st = current_options -> createState ( current_tfname ) ;
    /* printf("creating texture : %s\n",current_tfname); */
    if ( st != NULL )
      return st ;
  }
#endif

  //ssgSimpleState *st = new ssgSimpleState () ;
  grManagedState *st = grStateFactory();

  st -> setMaterial ( GL_SPECULAR, mat -> spec ) ;
  st -> setMaterial ( GL_EMISSION, mat -> emis ) ;
  st -> setMaterial ( GL_AMBIENT_AND_DIFFUSE, mat -> amb ) ;
  st -> setShininess ( mat -> shi ) ;

  st -> enable ( GL_COLOR_MATERIAL ) ;
  st -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;

  st -> enable  ( GL_LIGHTING ) ;

  st -> setShadeModel ( GL_SMOOTH ) ;
  //st -> setShadeModel ( GL_FLAT ) ;

  st -> setAlphaClamp(0);

  if (isaWindow) {
      st -> enable  ( GL_BLEND );
      st -> setTranslucent () ;
  } else if (isacar) {
      st -> enable  ( GL_BLEND );
      st -> setOpaque () ;
  } else if ( mat -> rgb[3] < 0.99 ) {
      st -> enable ( GL_ALPHA_TEST ) ;
      st -> enable  ( GL_BLEND ) ;
      st -> setTranslucent () ;
  } else {
      st -> disable ( GL_BLEND ) ;
      st -> setOpaque () ;
  }

  if (current_tfname != NULL) {
    st -> setTexture( current_options -> createTexture(current_tfname) ) ;
    st -> enable( GL_TEXTURE_2D ) ;

    if (strstr(current_tfname,"tree")!=NULL || strstr(current_tfname,"trans-")!=NULL || strstr(current_tfname,"arbor")!=NULL)
      {
	st->setAlphaClamp(0.65f);
	st -> enable ( GL_ALPHA_TEST ) ;
	st -> enable  ( GL_BLEND );
      }
  } else {
    st -> disable  ( GL_BLEND ) ;
    st -> disable( GL_TEXTURE_2D ) ;
  }

  return st ;
}

static ssgState *get_state_ext ( char * name)
{
	if (name==NULL) {
		return NULL;
	}
	grMultiTexState *st = new grMultiTexState();
	st->disable(GL_BLEND);
	st->setOpaque();

	if (name != NULL) {
		st->setTexture(current_options->createTexture(name));
		st->enable(GL_TEXTURE_2D) ;
		if (strstr(current_tfname,"tree")!=NULL || strstr(current_tfname,"trans-")!=NULL || strstr(current_tfname,"arbor")!=NULL) {
			st->enable(GL_BLEND);
			st->setAlphaClamp(0.7f);
			st->enable(GL_ALPHA_TEST);
		}
	} else {
		st->disable(GL_BLEND);
		st->disable(GL_TEXTURE_2D);
	}
	return st ;
}


static int do_material ( char *s )
{
  char name [ 1024 ] ;
  sgVec4 rgb  ;
  sgVec4 amb  ;
  sgVec4 emis ;
  sgVec4 spec ;
  int   shi ;
  float trans ;

  if ( sscanf ( s,
  "%s rgb %f %f %f amb %f %f %f emis %f %f %f spec %f %f %f shi %d trans %f",
    name,
    &rgb [0], &rgb [1], &rgb [2],
    &amb [0], &amb [1], &amb [2],
    &emis[0], &emis[1], &emis[2],
    &spec[0], &spec[1], &spec[2],
    &shi,
    &trans ) != 15 )
  {
    ulSetError ( UL_WARNING, "grloadac:do_material: Can't parse this MATERIAL:%s", s ) ;
  }
  else
  {
    char *nm = name ;

    skip_quotes ( &nm ) ;

    amb [ 3 ] = emis [ 3 ] = spec [ 3 ] = 1.0f ;
    rgb [ 3 ] = 1.0f - trans ;

    mlist [ num_materials ] = new _ssgMaterial ;
    clist [ num_materials ] = new sgVec4 [ 1 ] ;

    sgCopyVec4 ( clist [ num_materials ][ 0 ], rgb ) ;

    current_material = mlist [ num_materials ] ;
    sgCopyVec4 ( current_material -> spec, spec ) ;
    sgCopyVec4 ( current_material -> emis, emis ) ;
    sgCopyVec4 ( current_material -> rgb , rgb  ) ;
    sgCopyVec4 ( current_material -> amb , amb  ) ;
    current_material -> shi = (float) shi ;
  }

  num_materials++ ;
  return PARSE_CONT ;
}


static int do_object   ( char * s  )
{
	//ssgBranch *current_branch_g = NULL;
	int obj_type = search(obj_type_tags, s);

	delete [] current_tfname;
	current_tfname = NULL;

	char buffer[1024];

	sgSetVec2(texrep, 1.0f, 1.0f);
	sgSetVec2(texoff, 0.0f, 0.0f);

	sgMakeIdentMat4 ( current_matrix ) ;

	ssgEntity *old_cb = current_branch ;

	if (obj_type == OBJ_GROUP) {
		ssgBranch *current_branch_g = NULL;
		inGroup = 1;
		current_branch_g = new ssgBranchCb();
		current_branch->addKid(current_branch_g);
		current_branch = (ssgTransform*) current_branch_g;

		extern int preScene(ssgEntity *e);
		current_branch_g->setCallback(SSG_CALLBACK_PREDRAW, preScene);
    } else {
		inGroup=0;
	}

	ssgTransform *tr = new ssgTransform () ;

	tr -> setTransform ( current_matrix ) ;

	current_branch -> addKid ( tr ) ;
	current_branch = tr ;

	while ( FGETS ( buffer, 1024, loader_fd ) != NULL )
    	if ( search ( object_tags, buffer ) == PARSE_POP )
      		break ;

	int num_kids = last_num_kids ;

	for ( int i = 0 ; i < num_kids ; i++ ) {
		/* EE: bad hack for buggy .acc format... */
		if (FGETS ( buffer, 1024, loader_fd ) != NULL )
			search ( top_tags, buffer ) ;
    	else
			break;
	}

	current_branch = (ssgBranch *) old_cb ;
	return PARSE_CONT ;
}


static int do_name ( char *s )
{
  char *q=NULL;
  skip_quotes ( &s ) ;

  /* Window flag */
  if (!strncmp(s, "WI", 2)) {
      isaWindow = TRUE;
  } else {
      isaWindow = FALSE;
  }

  if (strstr(s,"__TKMN"))
    usegroup=TRUE;

  if (!strncmp(s, "TKMN",4))
      {
	q=strstr(s,"_g");
	if (q!=NULL)
	  *q='\0';
/* 	if (inGroup!=0) */
/* 	  { */
/* 	    printf("ingroup =%s\n",s); */
/* 	  } */
      }

  if (!strncmp(s, "DR", 2)) {
      current_branch -> setName ( "DRIVER" );
  } else {
      current_branch -> setName ( s ) ;
  }
  
  return PARSE_CONT ;
}


static int do_data     ( char *s )
{
  int len = strtol ( s, NULL, 0 ) ;

  current_data = new char [ len + 1 ] ;

  for ( int i = 0 ; i < len ; i++ )
    current_data [ i ] = FGETC ( loader_fd ) ;

  current_data [ len ] = '\0' ;

  FGETC ( loader_fd ) ;  /* Final RETURN */

  ssgBranch *br = current_options -> createBranch ( current_data ) ;

  if ( br != NULL )
  {
    current_branch -> addKid ( br ) ;
    current_branch = br ;
  }

  /* delete [] current_data ; */
  current_data = NULL ;

  return PARSE_CONT ;
}


static int do_texture  ( char *s )
{
  char *p ;

  if ( s == NULL || s[0] == '\0' )
    current_tfname = NULL ;
  else
  {
    if ((p=strstr(s," base"))!=NULL)
      {
	*p='\0';
	numMapLevel=1;
	mapLevel=LEVEL0;
	delete [] current_tbase ;
	delete [] current_tfname ;
	delete [] current_ttiled ;
	current_ttiled = 0;
	delete [] current_tskids ;
	current_tskids = 0;
	delete [] current_tshad ;
	current_tshad = 0;
	skip_quotes ( &s ) ;
	current_tbase = new char [ strlen(s)+1 ] ;
	current_tfname = new char [ strlen(s)+1 ] ;
	strcpy ( current_tbase, s ) ;
	strcpy ( current_tfname, s ) ;
      }
    else  if ((p=strstr(s," tiled"))!=NULL)
      {
	*p='\0';
	delete [] current_ttiled ;
	current_ttiled=0;
	delete [] current_tskids ;
	current_tskids = 0;
	delete [] current_tshad ;
	current_tshad = 0;
	if (!strstr(s,NOTEXTURE))
	  {
	    numMapLevel++;;
	    mapLevel|=LEVEL1;
	    skip_quotes ( &s ) ;
	    current_ttiled = new char [ strlen(s)+1 ] ;
	    strcpy ( current_ttiled, s ) ;
	  }
      }
    else  if ((p=strstr(s," skids"))!=NULL)
      {
	*p='\0';
	delete [] current_tskids ;
	current_tskids = 0;
	delete [] current_tshad ;
	current_tshad = 0;
	if (!strstr(s,NOTEXTURE))
	  {
	    numMapLevel++;;
	    mapLevel|=LEVEL2;
	    skip_quotes ( &s ) ;
	    current_tskids = new char [ strlen(s)+1 ] ;
	    strcpy ( current_tskids, s ) ;
	  }
      }
    else  if ((p=strstr(s," shad"))!=NULL)
      {
	*p='\0';
	delete [] current_tshad ;
	current_tshad = 0;
	if (!strstr(s,NOTEXTURE))
	  {
	    numMapLevel++;;
	    mapLevel|=LEVEL3;
	    skip_quotes ( &s ) ;
	    current_tshad = new char [ strlen(s)+1 ] ;
	    strcpy ( current_tshad, s ) ;
	  }
      }
    else
      {
	skip_quotes ( &s ) ;
	numMapLevel=1;
	mapLevel=LEVEL0;
	delete [] current_tfname ;
	delete [] current_tbase ;
	current_tbase = 0;
	delete [] current_ttiled ;
	current_ttiled = 0;
	delete [] current_tskids ;
	current_tskids = 0;
	delete [] current_tshad ;
	current_tshad = 0;
	current_tfname = new char [ strlen(s)+1 ] ;
	strcpy ( current_tfname, s ) ;
      }
  }

  return PARSE_CONT ;
}


static int do_texrep ( char *s )
{
  if ( sscanf ( s, "%f %f", & texrep [ 0 ], & texrep [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texrep record." ) ;

  return PARSE_CONT ;
}


static int do_texoff ( char *s )
{
  if ( sscanf ( s, "%f %f", & texoff [ 0 ], & texoff [ 1 ] ) != 2 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal texoff record." ) ;

  return PARSE_CONT ;
}

static int do_rot ( char *s )
{
  current_matrix [ 0 ][ 3 ] = current_matrix [ 1 ][ 3 ] = current_matrix [ 2 ][ 3 ] =
    current_matrix [ 3 ][ 0 ] = current_matrix [ 3 ][ 1 ] = current_matrix [ 3 ][ 2 ] = 0.0f ;
  current_matrix [ 3 ][ 3 ] = 1.0f ; 

  if ( sscanf ( s, "%f %f %f %f %f %f %f %f %f",
        & current_matrix [ 0 ] [ 0 ], & current_matrix [ 0 ] [ 1 ], & current_matrix [ 0 ] [ 2 ],
        & current_matrix [ 1 ] [ 0 ], & current_matrix [ 1 ] [ 1 ], & current_matrix [ 1 ] [ 2 ],
        & current_matrix [ 2 ] [ 0 ], & current_matrix [ 2 ] [ 1 ], & current_matrix [ 2 ] [ 2 ] ) != 9 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal rot record." ) ;

  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;
  return PARSE_CONT ;
}

static int do_loc      ( char *s )
{
  if ( sscanf ( s, "%f %f %f", & current_matrix [ 3 ][ 0 ], & current_matrix [ 3 ][ 2 ], & current_matrix [ 3 ][ 1 ] ) != 3 )
    ulSetError ( UL_WARNING, "ac_to_gl: Illegal loc record." ) ;

  current_matrix [ 3 ][ 1 ] = - current_matrix [ 3 ][ 1 ] ;
  current_matrix [ 3 ][ 3 ] = 1.0f ;
  ((ssgTransform *)current_branch) -> setTransform ( current_matrix ) ;

  return PARSE_CONT ;
}

static int do_url      ( char *s )
{
  skip_quotes ( & s ) ;

#ifdef PRINT_URLS
  printf ( "/* URL: \"%s\" */\n", s ) ;
#endif

  return PARSE_CONT ;
}

static int do_numvert  ( char *s )
{
  char buffer [ 1024 ] ;

  nv = strtol ( s, NULL, 0 ) ;
 
  delete [] vtab ;
  delete [] ntab ;
  delete [] t0tab ;
  delete [] t1tab ;
  delete [] t2tab ;
  delete [] t3tab ;
  totalnv=nv;
  totalstripe=0;

  vtab = new sgVec3 [ nv ] ;
  ntab = new sgVec3 [ nv ] ;
  t0tab = new sgVec2 [ nv ] ;
  t1tab = new sgVec2 [ nv ] ;
  t2tab = new sgVec2 [ nv ] ;
  t3tab = new sgVec2 [ nv ] ;
  vertlist=new ssgIndexArray ();
  striplist=new ssgIndexArray ();

  for ( int i = 0 ; i < nv ; i++ )
  {
    FGETS ( buffer, 1024, loader_fd ) ;

    if ( sscanf ( buffer, "%f %f %f %f %f %f",
                          &vtab[i][0], &vtab[i][1], &vtab[i][2],&ntab[i][0], &ntab[i][1], &ntab[i][2] ) != 6 )
      {
	usenormal=0;
	if ( sscanf ( buffer, "%f %f %f",
		      &vtab[i][0], &vtab[i][1], &vtab[i][2] ) != 3 )
	  {
	    ulSetError ( UL_FATAL, "ac_to_gl: Illegal vertex record." ) ;
	  }
      }
    else
      {
	usenormal=1;
	float tmp  =  ntab[i][1] ;
	ntab[i][1] = -ntab[i][2] ;
	ntab[i][2] = tmp ;
      }

    float tmp  =  vtab[i][1] ;
    vtab[i][1] = -vtab[i][2] ;
    vtab[i][2] = tmp ;

    if (vtab[i][0] >t_xmax)
      t_xmax=vtab[i][0];
    if (vtab[i][0] <t_xmin)
      t_xmin=vtab[i][0];

    if (vtab[i][1] >t_ymax)
      t_ymax=vtab[i][1];
    if (vtab[i][1] <t_ymin)
      t_ymin=vtab[i][1];
    
  }

  return PARSE_CONT ;
}

static int do_numsurf  ( char *s )
{
  int ns = strtol ( s, NULL, 0 ) ;

  for ( int i = 0 ; i < ns ; i++ )
  {
    char buffer [ 1024 ] ;

    FGETS ( buffer, 1024, loader_fd ) ;
    search ( surf_tag, buffer ) ;
  }

  return PARSE_CONT ;
}

static int do_surf     ( char *s )
{
  current_flags = strtol ( s, NULL, 0 ) ;

  char buffer [ 1024 ] ;

  while ( FGETS ( buffer, 1024, loader_fd ) != NULL )
    if ( search ( surface_tags, buffer ) == PARSE_POP )
      break ;

  return PARSE_CONT ;
}


static int do_mat ( char *s )
{
  int mat = strtol ( s, NULL, 0 ) ;

  current_material = mlist [ mat ] ;
  current_colour   = clist [ mat ] ;

  return PARSE_CONT ;
}


static int do_refs( char *s )
{
	int nrefs = strtol( s, NULL, 0 );
	char buffer[1024];

	if (nrefs == 0) {
		return PARSE_POP ;
	}

	ssgVertexArray *vlist = new ssgVertexArray(nrefs);
	ssgTexCoordArray *tlist = new ssgTexCoordArray (nrefs);
	ssgTexCoordArray *tlist1 = NULL;
	ssgTexCoordArray *tlist2 = NULL;
	ssgTexCoordArray *tlist3 = NULL;
	//ssgIndexArray *vindices = new ssgIndexArray(nrefs);
	ssgNormalArray *nrm = new ssgNormalArray(nrefs);

	if (numMapLevel > 1) {
		tlist1 = new ssgTexCoordArray(nrefs);
	}
	if (numMapLevel > 2) {
		tlist2 = new ssgTexCoordArray(nrefs);
	}
	if (numMapLevel > 3) {
		tlist3 = new ssgTexCoordArray(nrefs);
	}

	for (int i = 0; i < nrefs; i++) {
		FGETS(buffer, 1024, loader_fd);

		int vtx;
		sgVec2 tc;
		sgVec2 tc1 = {0};
		sgVec2 tc2 = {0};
		sgVec2 tc3 = {0};
		int tn=0;
		tn= sscanf ( buffer, "%d %f %f %f %f %f %f %f %f", &vtx,
			&tc[0],&tc[1],
			&tc1[0],&tc1[1],
			&tc2[0],&tc2[1],
			&tc3[0],&tc3[1]);

		if (tn < 3 )
		{
			ulSetError ( UL_FATAL, "ac_to_gl: Illegal ref record not enough text coord." ) ;
		}

		tc[0] *= texrep[0] ;
		tc[1] *= texrep[1] ;
		tc[0] += texoff[0] ;
		tc[1] += texoff[1] ;

		tlist -> add ( tc ) ;
		t0tab[vtx][0]=tc[0];
		t0tab[vtx][1]=tc[1];

		t1tab[vtx][0]=tc1[0];
		t1tab[vtx][1]=tc1[1];

		t2tab[vtx][0]=tc2[0];
		t2tab[vtx][1]=tc2[1];

		t3tab[vtx][0]=tc3[0];
		t3tab[vtx][1]=tc3[1];

		if (numMapLevel > 1) {
			tlist1->add(tc1);
		}
		if (numMapLevel > 2) {
			tlist2->add(tc2);
		}
		if (numMapLevel > 3) {
			tlist3->add(tc3);
		}

		vlist->add(vtab[vtx]);
		if (usenormal == 1) {
			nrm->add(ntab[vtx]);
		}
		//vindices-> add (i);
		vertlist->add(vtx);
	}
	#ifdef GUIONS
	if (usenormal==1) {
		printf("use normal\n");
	}
	#endif /* GUIONS */

	ssgColourArray *col = new ssgColourArray(1);

	col->add(*current_colour);

	sgVec3 nm;

	if (usenormal == 0) {
		if (nrefs < 3) {
			sgSetVec3 (nm, 0.0f, 0.0f, 1.0f);
		} else {
			sgMakeNormal (nm, vlist->get(0), vlist->get(1), vlist->get(2));
		}
		nrm -> add ( nm ) ;
	}


	int type = ( current_flags & 0x0F ) ;
	if ( type >= 0 && type <= 4 ) {
		GLenum gltype = GL_TRIANGLES ;
		switch ( type )
		{
		case 0 : gltype = GL_TRIANGLE_FAN ;
				break ;
		case 1 : gltype = GL_LINE_LOOP ;
				break ;
		case 2 : gltype = GL_LINE_STRIP ;
				break ;
		case 4 : gltype = GL_TRIANGLE_STRIP ;
			usestrip=TRUE;
				break ;
	}

#ifdef NORMAL_TEST
	/* GUIONS TEST that draw all the normals of a car */
	if(isacar == TRUE) {
		ssgVertexArray *vlinelist = new ssgVertexArray(nv*2);
		for (i = 0; i < nv; i++) {
			sgVec3 tv;
			tv[0] = ntab[i][0]*0.2 + vtab[i][0];
			tv[1] = ntab[i][1]*0.2 + vtab[i][1];
			tv[2] = ntab[i][2]*0.2 + vtab[i][2];
			vlinelist->add(vtab[i]);
			vlinelist->add(tv);
		}
		ssgVtxTable *vline = new ssgVtxTable(GL_LINES, vlinelist, NULL, NULL, NULL);
		current_branch->addKid(current_options->createLeaf(vline, 0));
	}
#endif

	/* check the number of texture units */
	if (numMapLevel > maxTextureUnits) {
		numMapLevel=maxTextureUnits;
	}
	if (isacar == TRUE) {
		mapLevel=LEVELC;
		if (tlist1 && maxTextureUnits > 1) {
			mapLevel = LEVELC2;
			numMapLevel = 2;
		}
		if (tlist2 && maxTextureUnits > 2) {
			mapLevel = LEVELC3;
			numMapLevel = 3;
		}
	}
#define VTXARRAY_GUIONS
#ifdef VTXARRAY_GUIONS
	if (usestrip == FALSE)
#endif
	{
		/* TEST
		if (isacar==FALSE)
		{numMapLevel=1;
		mapLevel=LEVEL0;
		}
		*/
		grVtxTable* vtab = new grVtxTable ( gltype,
		vlist, nrm, tlist,tlist1,tlist2,tlist3,numMapLevel,mapLevel, col, indexCar ) ;
		/* good */
		/*ssgVtxArray* vtab = new ssgVtxArray ( gltype,
		vlist, nrm, tlist, col , vindices) ;*/

		/*striplist-> add (nrefs);
		grVtxTable* vtab = new grVtxTable ( gltype,
							vlist,
							striplist,
							1,vertlist,
							nrm, tlist,tlist1,tlist2,tlist3,numMapLevel,mapLevel, col, indexCar ) ;*/

		/*printf("name ob =%s , numMapLevel =%d , maoLevel=%d \n",	current_branch -> getName (  ) ,numMapLevel, mapLevel);*/

		vtab -> setState ( get_state ( current_material ) ) ;
		vtab -> setCullFace ( ! ( (current_flags>>4) & 0x02 ) ) ;

		if (numMapLevel>1)
			vtab -> setState1 (get_state_ext (current_ttiled ));
		if (numMapLevel>2)
			vtab -> setState2 (get_state_ext (current_tskids ));
		if (numMapLevel>3)
			vtab -> setState3 (get_state_ext (current_tshad ));

		ssgLeaf* leaf = current_options -> createLeaf ( vtab, 0 ) ;

		if (leaf) {
			current_branch->addKid(leaf);
		} else {
			// TODO: delete leaf or vtab?
			delete vtab;
		}
	}
#ifdef VTXARRAY_GUIONS
	else {
		/* memorize the stripe index */
		striplist-> add (nrefs);
		totalstripe++;
		delete vlist;
		vlist = 0;
		delete tlist;
		tlist = 0;
		delete tlist1;
		tlist1 = 0;
		delete tlist2;
		tlist2 = 0;
		delete tlist3;
		tlist3 = 0;
		/*delete vindices;
		vindices = 0;*/
		delete nrm;
		nrm = 0;
		}
	#endif
	}

	if (col->getRef() == 0) {
		delete col;
	}
	return PARSE_POP ;
}

static int do_kids ( char *s )
{
	last_num_kids = strtol(s, NULL, 0);

#ifdef VTXARRAY_GUIONS
	if (last_num_kids == 0 && usestrip == TRUE && inGroup != 1) {
		ssgVertexArray *vlist = new ssgVertexArray(totalnv);
		ssgNormalArray *nrm = new ssgNormalArray(totalnv);
		ssgTexCoordArray *tlist0 = new ssgTexCoordArray(totalnv);
		ssgTexCoordArray *tlist1 = NULL;
		ssgTexCoordArray *tlist2 = NULL;
		ssgTexCoordArray *tlist3 = NULL;
		/* if (numMapLevel>1) */
		tlist1 = new ssgTexCoordArray(totalnv);
		/* if (numMapLevel>2) */
		tlist2 = new ssgTexCoordArray(totalnv);
		/* if (numMapLevel>3) */
		tlist3 = new ssgTexCoordArray(totalnv);
      	for (int i = 0; i < totalnv; i++) {
			tlist0 -> add ( t0tab[i] ) ;
			/* if (numMapLevel>1) */
			tlist1 -> add ( t1tab[i] ) ;
			/* if (numMapLevel>2) */
			tlist2 -> add ( t2tab[i] ) ;
			/* if (numMapLevel>3) */
			tlist3 -> add ( t3tab[i] ) ;
			vlist -> add ( vtab[i] ) ;
			if (usenormal==1) {
				nrm -> add ( ntab[i] ) ;
			}
		}

		ssgColourArray *col = new ssgColourArray ( 1 ) ;
		col -> add ( *current_colour ) ;

		/* int type = ( current_flags & 0x0F ) ; */
		GLenum gltype = GL_TRIANGLE_STRIP ;

		/* check the number of texture units */
		if (numMapLevel>maxTextureUnits)
			numMapLevel=maxTextureUnits;
		if (isacar==TRUE) {
			mapLevel=LEVELC;
			if (tlist1 && maxTextureUnits>2) {
				mapLevel=LEVELC2;
				numMapLevel=2;
			}
			if (tlist2 && maxTextureUnits>2){
				mapLevel=LEVELC3;
				numMapLevel=3;
			}
		}
		/*ssgVtxArray* vtab = new ssgVtxArray ( gltype,
		vlist, nrm, tlist0, col , vertlist) ;*/

		grVtxTable* vtab = new grVtxTable ( gltype,
					  vlist,
					  striplist,
					  totalstripe,
					  vertlist,
					  nrm,
					  tlist0,tlist1,tlist2,tlist3,
					  numMapLevel,mapLevel,
					  col,
					  indexCar ) ;
      vtab -> setState ( get_state ( current_material ) ) ;
      vtab -> setCullFace ( ! ( (current_flags>>4) & 0x02 ) ) ;
      if (numMapLevel>1)
	  vtab -> setState1 (get_state_ext (current_ttiled ));
      if (numMapLevel>2)
	  vtab -> setState2 (get_state_ext (current_tskids ));
      if (numMapLevel>3)
	  vtab -> setState3 (get_state_ext (current_tshad ));
      ssgLeaf* leaf = current_options -> createLeaf ( vtab, 0 ) ;

		if (leaf) {
			current_branch -> addKid ( leaf ) ;
		} else {
			delete vtab;
		}
	}
#endif

	numMapLevel=1;
	mapLevel=LEVEL0;
	return PARSE_POP ;
}


void myssgFlatten(ssgEntity *obj)
{

 if ( obj -> isAKindOf ( ssgTypeBranch() ) )
  {
    ssgBranch *br = (ssgBranch *) obj ;
#ifdef WIN32
    if (!strnicmp(br->getKid(0)->getName(), "tkmn",4))
#else
    if (!strncasecmp(br->getKid(0)->getName(), "tkmn",4))
#endif
      {
	ssgFlatten(br->getKid(0));
      }
    else
      {
	for ( int i = 0 ; i < br -> getNumKids () ; i++ )
	  ssgFlatten( br -> getKid ( i ) );
      }
  }
 return ;

}

ssgEntity *grssgCarLoadAC3D ( const char *fname, const ssgLoaderOptions* options,int index )
{

  isacar=TRUE;
  usestrip=FALSE;
  indexCar=index;
  t_xmax=-999999.0;
  t_ymax=-999999.0;
  t_xmin=+999999.0;
  t_ymin=+999999.0;

  GfOut("CarLoadAC3D loading %s\n", fname);

  ssgEntity *obj = myssgLoadAC ( fname, options ) ;
  
  if ( obj == NULL )
    return NULL ;
  
  /* Do some simple optimisations */

  ssgBranch *model = new ssgBranch () ;
  model -> addKid ( obj ) ;
  if(usestrip==FALSE)
    {
      /*myssgFlatten(obj);*/
      ssgFlatten    ( obj ) ;
      ssgStripify   ( model ) ;
    }
  carTrackRatioX= (t_xmax-t_xmin)/(shad_xmax-shad_xmin);
  carTrackRatioY= (t_ymax-t_ymin)/(shad_ymax-shad_ymin);
  return model ;

}

ssgEntity *grssgLoadAC3D ( const char *fname, const ssgLoaderOptions* options )
{
  isacar=FALSE;
  usegroup=FALSE;
  usestrip=FALSE;

  t_xmax=-999999.0;
  t_ymax=-999999.0;
  t_xmin=+999999.0;
  t_ymin=+999999.0;

  GfOut("LoadAC3D loading %s\n", fname);

  ssgEntity *obj = myssgLoadAC ( fname, options ) ;

  if ( obj == NULL )
    return NULL ;


  /* Do some simple optimisations */

  ssgBranch *model = new ssgBranch () ;
  model -> addKid ( obj ) ;
  if ((usegroup==FALSE) && (usestrip==FALSE))
    {
	ssgFlatten    ( obj ) ;
	ssgStripify   ( model ) ;
    }
  shad_xmax=t_xmax;
  shad_ymax=t_ymax;
  shad_xmin=t_xmin;
  shad_ymin=t_ymin;

  return model ;
}

/*
  Original function for backwards compatibility...
*/
static ssgEntity *myssgLoadAC ( const char *fname, const ssgLoaderOptions* options )
{

  if (maxTextureUnits==0)
    {
      InitMultiTex();
    }

  ssgSetCurrentOptions ( (ssgLoaderOptions*)options ) ;
  current_options = ssgGetCurrentOptions () ;

  char filename [ 1024 ] ;
  current_options -> makeModelPath ( filename, fname ) ;

  num_materials = 0 ;
  vtab = NULL ;

  current_material = NULL ;
  current_colour   = NULL ;
  current_tfname   = NULL ;
  current_branch   = NULL ;

  sgSetVec2 ( texrep, 1.0, 1.0 ) ;
  sgSetVec2 ( texoff, 0.0, 0.0 ) ;

  loader_fd = FOPEN ( filename, "rb" ) ;

  if ( loader_fd == NULL )
  {
    ulSetError ( UL_WARNING, "ssgLoadAC: Failed to open '%s' for reading", filename ) ;
    return NULL ;
  }

  char buffer [ 1024 ] ;
  int firsttime = TRUE ;

  current_branch = new ssgTransform () ;

  while ( FGETS ( buffer, 1024, loader_fd ) != NULL )
  {
    char *s = buffer ;


    /* Skip leading whitespace */

    skip_spaces ( & s ) ;

    /* Skip blank lines and comments */

    if ( *s < ' ' && *s != '\t' ) continue ;
    if ( *s == '#' || *s == ';' ) continue ;

    if ( firsttime )
    {
      firsttime = FALSE ;

      if ( ! ulStrNEqual ( s, "AC3D", 4 ) )
      {
        FCLOSE ( loader_fd ) ;
        ulSetError ( UL_WARNING, "ssgLoadAC: '%s' is not in AC3D format.", filename ) ;
        return NULL ;
      }
    }
    else
      search ( top_tags, s ) ;
  }

  delete [] current_tfname;
  current_tfname = NULL ;
  delete [] vtab ;
  vtab = 0;

	int i;
	for (i = 0; i < num_materials; i++) {
		delete mlist[i];
		delete [] clist[i];
	}

  FCLOSE ( loader_fd ) ;

  return current_branch ;

}

