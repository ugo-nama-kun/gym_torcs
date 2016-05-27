/************************************************************************************************

    file                 : easymesh.cpp
    author		 : Bojan NICENO 
    Original Location    : http://www-dinma.univ.trieste.it/~nirftc/research/easymesh/easymesh.html
    modified             : Eric Espie (eric.espie@torcs.org)
    copyright            : Bojan NICENO & Eric Espie (parts)
    version              : $Id: easymesh.cpp,v 1.11.2.1 2008/11/09 17:50:23 berniw Exp $

 ************************************************************************************************/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <tgf.h>
#include <track.h>
#include <robottools.h>

#include "trackgen.h"
#include "elevation.h"
#include "easymesh.h"
#include "objects.h"
#include "relief.h"
#include "ac3d.h"
#include "easymesh.h"


static tdble 	Margin;
static tdble 	ExtHeight;
static tdble 	GridStep;
static tdble	TrackStep;
static char	buf[1024];
static char	*TexName;
static tdble	TexSize;
static tdble	TexRand;

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef PI
#define PI    3.14159265359
#endif

#define SMALL 1e-10
#define GREAT 1e+10

#define ON      0 
#define OFF    -1       /* element is switched off */
#define WAIT   -2       /* node is waiting (it is a corner node) */
#define A       3
#define D       4
#define W       5

#define MAX_NODES 15000

/*-----------------------------+
|  definitions for the chains  |
+-----------------------------*/
#define CLOSED 0
#define OPEN   1
#define INSIDE 2


struct ele
{
    int i,  j,  k;
    int ei, ej, ek;
    int si, sj, sk;

    int mark;             /* is it off (ON or OFF) */
    int state;            /* is it (D)one, (A)ctive or (W)aiting */
    int material;

    double xv, yv, xin, yin, R, r, Det;

    int new_numb;         /* used for renumeration */
}
elem[MAX_NODES*2];


struct sid
{
    int ea, eb;           /* left and right element */
    int a, b, c, d;       /* left, right, start and end point */

    int mark;             /* is it off, is on the boundary */

    double s;

    int new_numb;         /* used for renumeration */
}
side[MAX_NODES*3];


struct nod node[MAX_NODES], *point;


struct seg *segment;
 
struct chai
{
    int s0, s1, type;
} *chain;


int Ne, Nn, Ns, Nc, Fl;             /* number of: elements, nodes, sides */
int ugly;                       /* mora li biti globalna ??? */

double xmax, xmin, ymax, ymin;

struct vtx
{
    double	x;
    double	y;
    double	z;
};

struct ref
{
    int		vtxidx;
    double	u;
    double	v;
};


struct surf
{
    struct ref	ref[3];
};


struct group
{
    int		nbvtx;
    int		maxvtx;
    struct vtx	*vertices;
    
    int		nbsurf;
    int		maxsurf;
    struct surf	*surfaces;
};

#define SURF_INCR	100
#define VTX_INCR	100

static struct group	*Groups;
static int		ActiveGroups;
static float		GroupSize;
static float		XGroupOffset;
static float		YGroupOffset;
static int		XGroupNb;
static int		GroupNb;


void swap_side(int s);


/*=========================================================================*/
double area(struct nod *na, struct nod *nb, struct nod *nc)
{
    return 0.5 * (((*nb).x - (*na).x) * ((*nc).y - (*na).y) 
		  - ((*nb).y - (*na).y) * ((*nc).x - (*na).x));
}
/*-------------------------------------------------------------------------*/


/*=========================================================================*/
double dist(struct nod *na, struct nod *nb)
{
    return sqrt(((*nb).x-(*na).x)*((*nb).x-(*na).x)
		+ ((*nb).y-(*na).y)*((*nb).y-(*na).y) );
}
/*-------------------------------------------------------------------------*/


/*=========================================================================*/
int in_elem(struct nod *n)
{
    int e;
 
    for (e = 0; e < Ne; e++) {   /* This must search through all elements ?? */
	if (area(n, &node[elem[e].i], &node[elem[e].j]) >= 0.0
	    && area(n, &node[elem[e].j], &node[elem[e].k]) >= 0.0
	    && area(n, &node[elem[e].k], &node[elem[e].i]) >= 0.0) {
	    break; 
	}
    }
    return e;
}
/*-in_elem-----------------------------------------------------------------*/


/*=========================================================================*/
void bowyer(int n, int spac)
{
    int e, s, swap;
    struct nod vor;

    do { 
	swap = 0;
	for (s = 0; s < Ns; s++) {
	    if (side[s].mark == 0) {
		if (side[s].a == n) {
		    e = side[s].eb; 
		    if (e != OFF) {
			vor.x = elem[e].xv; 
			vor.y=  elem[e].yv;
			if (dist(&vor, &node[n]) < elem[e].R) {
			    swap_side(s);
			    swap=1;
			}
		    }
		} else {
		    if (side[s].b == n) {
			e = side[s].ea;
			if (e != OFF) {
			    vor.x = elem[e].xv; 
			    vor.y = elem[e].yv;
			    if (dist(&vor, &node[n]) < elem[e].R) {
				swap_side(s);
				swap=1;
			    }
			}
		    }
		}
	    }
	}
    } while (swap == 1);

}
/*-bowyer------------------------------------------------------------------*/


/*=========================================================================*/
/*---------------------------------------------------+
|  This function calculates radii of inscribed and   |
|  circumscribed circle for a given element (int e)  |
+---------------------------------------------------*/
void circles(int e)
{
    double x, y, xi, yi, xj, yj, xk, yk, xij, yij, xjk, yjk, num, den;
    double si, sj, sk, O;

    xi = node[elem[e].i].x;
    yi = node[elem[e].i].y;
    xj = node[elem[e].j].x;
    yj = node[elem[e].j].y;
    xk = node[elem[e].k].x;
    yk = node[elem[e].k].y;
   
    xij = 0.5 * (xi + xj);
    yij = 0.5 * (yi + yj);
    xjk = 0.5 * (xj + xk);
    yjk = 0.5 * (yj + yk);

    num = (xij - xjk) * (xj - xi) + (yij - yjk) * (yj - yi);
    den = (xj - xi) * (yk - yj) - (xk - xj) * (yj - yi);

    if (den > 0) {
	elem[e].xv = x = xjk + num / den * (yk - yj);
	elem[e].yv = y = yjk - num / den * (xk - xj);
	elem[e].R  = sqrt((xi - x) * (xi - x) + (yi - y) * (yi - y));
    }
    

    si = side[elem[e].si].s;
    sj = side[elem[e].sj].s;
    sk = side[elem[e].sk].s;
    O = si + sj + sk;
    elem[e].Det = xi * (yj - yk) - xj * (yi - yk) + xk * (yi - yj);

    elem[e].xin = (xi * si + xj * sj + xk * sk ) / O;
    elem[e].yin = (yi * si + yj * sj + yk * sk ) / O;

    elem[e].r   = elem[e].Det / O;
}
/*-circles-----------------------------------------------------------------*/


/*=========================================================================*/
/*----------------------------------------------------------------+
|  This function calculates the value of the spacing function in  |
|  a new node 'n' which is inserted in element 'e' by a linear    |
|  approximation from the values of the spacing function in the   |
|  elements nodes.                                                |
+----------------------------------------------------------------*/
void spacing(int e, int n)
{
    double dxji, dxki, dyji, dyki, dx_i, dy_i, det, a, b;

    dxji = node[elem[e].j].x - node[elem[e].i].x;
    dyji = node[elem[e].j].y - node[elem[e].i].y;
    dxki = node[elem[e].k].x - node[elem[e].i].x;
    dyki = node[elem[e].k].y - node[elem[e].i].y;
    dx_i = node[n].x - node[elem[e].i].x;
    dy_i = node[n].y - node[elem[e].i].y;

    det = dxji*dyki - dxki*dyji;

    a = (+dyki * dx_i - dxki * dy_i) / det;
    b = (-dyji * dx_i + dxji * dy_i) / det;

    node[n].F = node[elem[e].i].F + 
	a * (node[elem[e].j].F - node[elem[e].i].F) +
	b * (node[elem[e].k].F - node[elem[e].i].F);
}
/*-spacing-----------------------------------------------------------------*/


/*=========================================================================*/
int insert_node(double x, double y, double z, int spac,
		int prev_n, int prev_s_mark, int mark, int next_s_mark, int next_n)
{
    int    i,j,k,e,ei,ej,ek, s,si,sj,sk;
    double sx, sy;

    Nn++;          /* one new node */
    //if ((Nn % 100) == 0) {
	printf("\r%d Nodes", Nn);
	fflush(stdout);
    //}
    
    if (Nn > MAX_NODES) {
	printf("\nResize MAX_NODES... > %d\n", MAX_NODES);
	exit(1);
    }

    if ((x < xmin) || (x > xmax) || (y < ymin) || (y > ymax)) {
	printf("\nDon't Insert %f %f\n", x, y);
	return 0;
/* 	if (x < xmin) x = xmin; */
/* 	if (x > xmax) x = xmax; */
/* 	if (y < ymin) y = ymin; */
/* 	if (y > ymax) y = ymax; */
    }
 
    node[Nn-1].x = x;
    node[Nn-1].y = y;
    node[Nn-1].z = z;
    node[Nn-1].mark = mark;

    /* find the element which contains new node */ 
    e = in_elem(&node[Nn-1]);

    /* calculate the spacing function in the new node */
    if(spac==ON) {
	spacing(e, Nn-1);
    }
    
    i  = elem[e].i;
    j  = elem[e].j;
    k  = elem[e].k;
    ei = elem[e].ei;
    ej = elem[e].ej;
    ek = elem[e].ek; 
    si = elem[e].si;
    sj = elem[e].sj;
    sk = elem[e].sk; 
 
    Ne += 2;
    Ns += 3;

    /*---------------+
    |  new elements  |
    +---------------*/ 
    elem[Ne-2].i = Nn - 1;
    elem[Ne-2].j = k;   
    elem[Ne-2].k = i;
    elem[Ne-1].i = Nn - 1; 
    elem[Ne-1].j = i; 
    elem[Ne-1].k = j; 
 
    elem[Ne-2].ei = ej;  
    elem[Ne-2].ej = Ne - 1;
    elem[Ne-2].ek = e;
    elem[Ne-1].ei = ek;  
    elem[Ne-1].ej = e; 
    elem[Ne-1].ek = Ne - 2;
 
    elem[Ne-2].si = sj;  
    elem[Ne-2].sj = Ns - 2;
    elem[Ne-2].sk = Ns - 3;
    elem[Ne-1].si = sk;  
    elem[Ne-1].sj = Ns - 1;
    elem[Ne-1].sk = Ns - 2;
 
    /*------------+ 
    |  new sides  |
    +------------*/ 
    side[Ns-3].c  = k; 
    side[Ns-3].d  = Nn - 1;     /* c-d */
    side[Ns-3].a  = j;  
    side[Ns-3].b  = i;        /* a-b */
    side[Ns-3].ea = e; 
    side[Ns-3].eb = Ne - 2;
 
    side[Ns-2].c  = i;  
    side[Ns-2].d  = Nn - 1;     /* c-d */
    side[Ns-2].a  = k;  
    side[Ns-2].b  = j;        /* a-b */
    side[Ns-2].ea = Ne - 2;
    side[Ns-2].eb = Ne - 1;
 
    side[Ns-1].c  = j;  
    side[Ns-1].d  = Nn - 1;     /* c-d */
    side[Ns-1].a  = i;  
    side[Ns-1].b  = k;        /* a-b */
    side[Ns-1].ea = Ne - 1;
    side[Ns-1].eb = e;       

    for (s = 1; s <= 3; s++) {
	sx = node[side[Ns-s].c].x - node[side[Ns-s].d].x;
	sy = node[side[Ns-s].c].y - node[side[Ns-s].d].y;
	side[Ns-s].s = sqrt(sx * sx + sy * sy);
    }

    elem[e].i  = Nn - 1;
    elem[e].ej = Ne - 2;
    elem[e].ek = Ne - 1;
    elem[e].sj = Ns - 3;
    elem[e].sk = Ns - 1;

    if (side[si].a == i) {
	side[si].a  = Nn - 1;
	side[si].ea = e;
    }
    if (side[si].b == i) {
	side[si].b  = Nn - 1;
	side[si].eb = e;
    }
 
    if (side[sj].a == j) {
	side[sj].a  = Nn - 1;
	side[sj].ea = Ne - 2;
    }
    if (side[sj].b == j) {
	side[sj].b  = Nn - 1;
	side[sj].eb = Ne - 2;
    }
 
    if (side[sk].a == k) {
	side[sk].a = Nn - 1;
	side[sk].ea = Ne - 1;
    } 
    if (side[sk].b == k) {
	side[sk].b = Nn - 1;
	side[sk].eb = Ne - 1;
    } 

    if (ej != -1) {
	if (elem[ej].ei == e) {
	    elem[ej].ei = Ne - 2;
	}
	if (elem[ej].ej == e) {
	    elem[ej].ej = Ne-2;
	}
	if (elem[ej].ek == e) {
	    elem[ej].ek = Ne - 2;
	}
    }

    if (ek != -1) {
	if (elem[ek].ei == e) {
	    elem[ek].ei = Ne - 1;
	}
	if (elem[ek].ej == e) {
	    elem[ek].ej = Ne - 1;
	}
	if (elem[ek].ek == e) {
	    elem[ek].ek = Ne - 1;
	}
    }

    /* Find circumenters for two new elements, 
       and for the one who's segment has changed */
    circles(e);
    circles(Ne - 2);
    circles(Ne - 1);

    bowyer(Nn - 1, spac);

    /*-------------------------------------------------+
    |  NEW ! Insert boundary conditions for the sides  |
    +-------------------------------------------------*/
    for (s = 3; s < Ns; s++) {
	if (side[s].c == prev_n && side[s].d == Nn - 1) {
	    side[s].mark = prev_s_mark;
	}
	if (side[s].d == prev_n && side[s].c == Nn - 1) {
	    side[s].mark = prev_s_mark;
	}
	if (side[s].c == next_n && side[s].d == Nn - 1) {
	    side[s].mark = next_s_mark;
	}
	if (side[s].d == next_n && side[s].c == Nn - 1) {
	    side[s].mark = next_s_mark;
	}
    }

    return e;
}
/*-insert_node-------------------------------------------------------------*/


/*=========================================================================*/
void swap_side(int s)
{
    int    a, b, c, d, ea, eb, eac = 0, ead = 0, ebc = 0, ebd = 0, sad = 0, sac = 0, sbc = 0, sbd = 0;
    double sx, sy;
 
    ea=side[s].ea; 
    eb=side[s].eb;
    a = side[s].a;
    b = side[s].b;
    c = side[s].c;
    d = side[s].d;

    if (elem[ea].ei == eb) {
	ead = elem[ea].ej;
	eac = elem[ea].ek; 
	sad = elem[ea].sj;
	sac = elem[ea].sk;
    }
 
    if (elem[ea].ej == eb) {
	ead = elem[ea].ek;
	eac = elem[ea].ei; 
	sad = elem[ea].sk;
	sac = elem[ea].si;
    }   
 
    if (elem[ea].ek == eb) {
	ead = elem[ea].ei;
	eac = elem[ea].ej;
	sad = elem[ea].si;
	sac = elem[ea].sj;
    }

    if (elem[eb].ei == ea) {
	ebc = elem[eb].ej;
	ebd = elem[eb].ek;
	sbc = elem[eb].sj;
	sbd = elem[eb].sk;
    }

    if (elem[eb].ej == ea) {
	ebc = elem[eb].ek;
	ebd = elem[eb].ei;
	sbc = elem[eb].sk; 
	sbd = elem[eb].si;
    }
 
    if (elem[eb].ek == ea) {
	ebc = elem[eb].ei;
	ebd = elem[eb].ej;
	sbc = elem[eb].si;
	sbd = elem[eb].sj;
    }

    elem[ea].i  = a;
    elem[ea].j  = b; 
    elem[ea].k  = d;  
    elem[ea].ei = ebd;
    elem[ea].ej = ead;
    elem[ea].ek = eb;  
    elem[ea].si = sbd;
    elem[ea].sj = sad;
    elem[ea].sk = s;  
  
    elem[eb].i  = a; 
    elem[eb].j  = c; 
    elem[eb].k  = b;  
    elem[eb].ei = ebc;
    elem[eb].ej = ea; 
    elem[eb].ek = eac;  
    elem[eb].si = sbc;
    elem[eb].sj = s;  
    elem[eb].sk = sac;  

    if (eac != -1) {
	if (elem[eac].ei == ea) {
	    elem[eac].ei = eb;
	}
	if (elem[eac].ej == ea) {
	    elem[eac].ej = eb;
	}
	if (elem[eac].ek == ea) {
	    elem[eac].ek = eb; 
	}
    }
 
    if (ebd != -1) {
	if (elem[ebd].ei == eb) {
	    elem[ebd].ei = ea;
	}
	if (elem[ebd].ej == eb) {
	    elem[ebd].ej = ea;
	}
	if (elem[ebd].ek == eb) {
	    elem[ebd].ek = ea; 
	}
    }
 
    if (side[sad].ea == ea) {
	side[sad].a = b;
    }
    if (side[sad].eb == ea) {
	side[sad].b = b;
    }

    if (side[sbc].ea == eb) {
	side[sbc].a = a;
    }
    if (side[sbc].eb == eb) {
	side[sbc].b = a;
    }

    if (side[sbd].ea == eb) {
	side[sbd].ea = ea;
	side[sbd].a = a;
    }
    if (side[sbd].eb == eb) {
	side[sbd].eb = ea;
	side[sbd].b = a;
    }
 
    if (a < b) {
	side[s].c  = a;
	side[s].d  = b;
	side[s].a  = d;
	side[s].b  = c;
	side[s].ea = ea;
	side[s].eb = eb;
    } else {
	side[s].c  = b;
	side[s].d  = a;
	side[s].a  = c;
	side[s].b  = d;
	side[s].ea = eb;
	side[s].eb = ea;
    }

    sx = node[side[s].c].x - node[side[s].d].x;
    sy = node[side[s].c].y - node[side[s].d].y;
    side[s].s = sqrt(sx * sx + sy * sy);

    if (side[sac].ea == ea) {
	side[sac].ea = eb;
	side[sac].a = b;
    }
    if (side[sac].eb == ea) {
	side[sac].eb = eb;
	side[sac].b = b;
    }
 
    if (side[sad].ea == ea) {
	side[sad].a = b;
    }
    if (side[sad].eb == ea) {
	side[sad].b = b;
    }

    if (side[sbc].ea == eb) {
	side[sbc].a = a;
    }
    if (side[sbc].eb == eb) {
	side[sbc].b = a;
    }

    if (side[sbd].ea == eb) {
	side[sbd].ea = ea;
	side[sbd].a = a;
    }
    if (side[sbd].eb == eb) {
	side[sbd].eb = ea;
	side[sbd].b = a;
    }

    circles(ea);
    circles(eb);
}
/*-swap_side---------------------------------------------------------------*/


/*=========================================================================*/
void erase()
{
    int s, n, e;

    int a, b, c;

    /*--------------------------+
    |                           |
    |  Negative area check for  |
    |  elimination of elements  |
    |                           |
    +--------------------------*/
    for (e = 0; e < Ne; e++) {
	if ((node[elem[e].i].chain == node[elem[e].j].chain) &&
	    (node[elem[e].j].chain == node[elem[e].k].chain) &&
	    (chain[node[elem[e].i].chain].type == CLOSED)) {
	    a = min(min(elem[e].i, elem[e].j), elem[e].k);
	    c = max(max(elem[e].i, elem[e].j), elem[e].k);
	    b = elem[e].i + elem[e].j + elem[e].k - a - c;

	    if (a < 3) {
		elem[e].mark = OFF;
	    } else {
		if (area(&node[a], &node[b], &node[c]) < 0.0) {
		    elem[e].mark = OFF;
		}
	    }
	}
    }
    

    for (e = 0; e < Ne; e++) {
	if (elem[elem[e].ei].mark == OFF) {
	    elem[e].ei = OFF;
	}
	if (elem[elem[e].ej].mark == OFF) {
	    elem[e].ej = OFF;
	}
	if (elem[elem[e].ek].mark == OFF) {
	    elem[e].ek = OFF;
	}
    }

    /*-----------------------+
    |                        |
    |  Elimination of sides  |
    |                        |
    +-----------------------*/
    for (s = 0; s < 3; s++) {
	side[s].mark = OFF;
    }	

    for (s = 3; s < Ns; s++) {
	if ((elem[side[s].ea].mark == OFF) && (elem[side[s].eb].mark == OFF)) {
	    side[s].mark = OFF;
	}
    }
	

    for (s = 3; s < Ns; s++) {
	if (side[s].mark != OFF) {
	    if (elem[side[s].ea].mark == OFF) {
		side[s].ea = OFF;
		side[s].a = OFF;
	    }
	    if (elem[side[s].eb].mark == OFF) {
		side[s].eb = OFF;
		side[s].b = OFF;
	    }
	}
    }
	

    /*-----------------------+
    |                        |
    |  Elimination of nodes  |
    |                        |
    +-----------------------*/
    for (n = 0; n < 3; n++) {
	node[n].mark = OFF;
    }
	

}
/*-erase-------------------------------------------------------------------*/


/*=========================================================================*/
void diamond(void)
{
    int    ea, eb, eac = 0, ead = 0, ebc = 0, ebd = 0, s;
 
    for (s = 0; s < Ns; s++) {
	
	if (side[s].mark != OFF) {
	    ea = side[s].ea;
	    eb = side[s].eb;

	    if (elem[ea].ei == eb) {
		ead = elem[ea].ej;
		eac = elem[ea].ek;
	    }
	    if (elem[ea].ej == eb) {
		ead = elem[ea].ek;
		eac = elem[ea].ei;
	    }   
	    if (elem[ea].ek == eb) {
		ead = elem[ea].ei;
		eac = elem[ea].ej;
	    }
	    if (elem[eb].ei == ea) {
		ebc = elem[eb].ej;
		ebd = elem[eb].ek;
	    }
	    if (elem[eb].ej == ea) {
		ebc = elem[eb].ek;
		ebd = elem[eb].ei;
	    }
	    if (elem[eb].ek == ea) {
		ebc = elem[eb].ei;
		ebd = elem[eb].ej;
	    }

	    if ((eac == OFF || elem[eac].state == D) &&
		(ebc == OFF || elem[ebc].state == D) &&
		(ead == OFF || elem[ead].state == D) &&
		(ebd == OFF || elem[ebd].state == D)) {
		elem[ea].state = D;
		elem[eb].state = D;
	    }
	}
    }
    
}
/*-diamond-----------------------------------------------------------------*/


/*=========================================================================*/
void classify(void)
/*----------------------------------------------------------+
|  This function searches through all elements every time.  |
|  Some optimisation will definitely bee needed             |
|                                                           |
|  But it also must me noted, that this function defines    |
|  the strategy for insertion of new nodes                  |
|                                                           |
|  It's MUCH MUCH better when the ugliest element is found  |
|  as one with highest ratio of R/r !!! (before it was      |
|  element with greater R)                                  |
+----------------------------------------------------------*/
{
    int e, ei, ej, ek,si,sj,sk;
    double ratio = 0.7, F;

    ugly = OFF;

    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF) {
	    ei = elem[e].ei;
	    ej = elem[e].ej;
	    ek = elem[e].ek;

	    F = (node[elem[e].i].F + node[elem[e].j].F + node[elem[e].k].F) / 3.0;

	    elem[e].state = W;

	    /*--------------------------+
	    |  0.577 is ideal triangle  |
	    +--------------------------*/
	    if (elem[e].R < 0.700*F) {
		elem[e].state = D; /* 0.0866; 0.07 */
	    }

	    /*------------------------+
	    |  even this is possible  |
	    +------------------------*/
	    if (ei != OFF && ej != OFF && ek != OFF) {
		if (elem[ei].state == D && elem[ej].state == D && elem[ek].state == D) {
		    elem[e].state=D;
		}
	    }
	}
    }
    

    /*--------------------------------------+
    |  Diamond check. Is it so important ?  |
    +--------------------------------------*/
    diamond();   

    /*------------------------------------------------+
    |  First part of the trick:                       |
    |    search through the elements on the boundary  |
    +------------------------------------------------*/
    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF && elem[e].state != D) {
	    si = elem[e].si;
	    sj = elem[e].sj;
	    sk = elem[e].sk;

	    if (side[si].mark != 0) {
		elem[e].state = A;
	    }
	    if (side[sj].mark != 0) {
		elem[e].state = A;
	    }
	    if (side[sk].mark != 0) {
		elem[e].state = A;
	    }

	    if (elem[e].state == A && elem[e].R / elem[e].r > ratio) {
		ratio = max(ratio, elem[e].R/elem[e].r);
		ugly = e;
	    }
	}
    }
    

    /*-------------------------------------------------+
      |  Second part of the trick:                       |
      |    if non-acceptable element on the boundary is  |
      |    found, ignore the elements inside the domain  |
      +-------------------------------------------------*/
    if (ugly == OFF) {
	for (e = 0; e < Ne; e++) {
	    if (elem[e].mark != OFF) {
		if (elem[e].state != D) {
		    ei = elem[e].ei;
		    ej = elem[e].ej;
		    ek = elem[e].ek;

		    if (ei != OFF) {
			if (elem[ei].state == D) {
			    elem[e].state = A;
			}
		    }
		    if (ej != OFF) {
			if (elem[ej].state == D) {
			    elem[e].state = A;
			}
		    }
		    if (ek != OFF) {
			if (elem[ek].state == D) {
			    elem[e].state = A;
			}
		    }
		    if (elem[e].state == A && elem[e].R/elem[e].r > ratio) {
			ratio = max(ratio, elem[e].R / elem[e].r);
			ugly = e;
		    }
		}
	    }
	}
    }
}
/*-classify----------------------------------------------------------------*/


/*=========================================================================*/
/*---------------------------------------------------+
|  This function is very important.                  |
|  It determines the position of the inserted node.  |
+---------------------------------------------------*/
void new_node()
{
    int    s=OFF, n = 0;
    double xM, yM, zM, p, q, qx, qy, rhoM, rho_M, d;

    struct nod Ca;

    /*-------------------------------------------------------------------------+
      |  It's obvious that elements which are near the boundary, will come into  |
      |  play first.                                                             |
      |                                                                          |
      |  However, some attention has to be payed for the case when two accepted  |
      |  elements surround the ugly one                                          |
      |                                                                          |
      |  What if new points falls outside the domain                             |
      +-------------------------------------------------------------------------*/
    if ((elem[ugly].ei != OFF) &&  (elem[elem[ugly].ei].state == D)) {
	s = elem[ugly].si;
	n = elem[ugly].i;
    }
    if ((elem[ugly].ej != OFF) && (elem[elem[ugly].ej].state == D)) {
	s = elem[ugly].sj;
	n = elem[ugly].j;
    }
    if ((elem[ugly].ek != OFF) && (elem[elem[ugly].ek].state == D)) {
	s = elem[ugly].sk;
	n = elem[ugly].k;
    }
    if ((elem[ugly].si != OFF) && (side[elem[ugly].si].mark > 0)) {
	s = elem[ugly].si;
	n = elem[ugly].i;
    }
    if ((elem[ugly].sj != OFF) && (side[elem[ugly].sj].mark > 0)) {
	s = elem[ugly].sj;
	n = elem[ugly].j;
    }
    if ((elem[ugly].sk != OFF) && (side[elem[ugly].sk].mark > 0)) {
	s = elem[ugly].sk;
	n = elem[ugly].k;
    }
    if (s == OFF) {
	return;
    }
    

    xM  = 0.5 * (node[side[s].c].x + node[side[s].d].x);
    yM  = 0.5 * (node[side[s].c].y + node[side[s].d].y);
    zM  = 0.5 * (node[side[s].c].z + node[side[s].d].z);

    Ca.x = elem[ugly].xv;
    Ca.y = elem[ugly].yv;

    p  = 0.5 * side[s].s;    /* not checked */

    qx = Ca.x - xM;
    qy = Ca.y - yM;
    q  = sqrt(qx * qx + qy * qy);

    rhoM = 0.577 * 0.5 *(node[side[s].c].F + node[side[s].d].F);

    rho_M = min(max(rhoM, p), 0.5 * (p * p + q * q) / q);

    if (rho_M < p) {
	d = rho_M;
    } else {
	d = rho_M + sqrt(rho_M * rho_M - p * p); 
    }

    /*---------------------------------------------------------------------+
    |  The following line check can the new point fall outside the domain. |
    |  However, I can't remember how it works, but I believe that it is    |
    |  still a weak point of the code, particulary when there are lines    |
    |  inside the domain                                                   |
    +---------------------------------------------------------------------*/

    if (area(&node[side[s].c], &node[side[s].d], &Ca) *
	area(&node[side[s].c], &node[side[s].d], &node[n]) > 0.0 ) {
	insert_node(xM + d * qx  / q,  yM + d * qy / q, zM, ON, OFF, 0, 0, 0, OFF);
    }
    return;
}
/*-new_node----------------------------------------------------------------*/


/*=========================================================================*/
void neighbours() 
/*--------------------------------------------------------------+
|  Counting the elements which surround each node.              |
|  It is important for the two functions: 'relax' and 'smooth'  |
+--------------------------------------------------------------*/
{ 
    int s;
 
    for (s = 0; s < Ns; s++) {
	if (side[s].mark == 0) {
	    if (node[side[s].c].mark == 0) {
		node[side[s].c].Nne++;
	    }
	}
	if (node[side[s].d].mark == 0) {
	    node[side[s].d].Nne++;
	}
	
    }
}
/*-neighbours--------------------------------------------------------------*/


/*=========================================================================*/
void materials()
{
    int e, c, mater = 0, over;
    int ei, ej, ek, si, sj, sk;

    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF) {
	    elem[e].material = OFF;
	}
    }
    

    for (c = 0; c < Nc; c++) {
	if (point[c].inserted == 0) {
	    elem[in_elem(&point[c])].material = point[c].mark;
	    mater = ON;
	}
    }

    if (mater == ON) {
	for(;;) {      
	    over = ON;
	    for (e = 0; e < Ne; e++) {
		if (elem[e].mark != OFF && elem[e].material == OFF) {
		    ei = elem[e].ei;
		    ej = elem[e].ej;
		    ek = elem[e].ek;

		    si = elem[e].si;
		    sj = elem[e].sj;
		    sk = elem[e].sk;

		    if (ei != OFF) {
			if (elem[ei].material != OFF && side[si].mark == 0) {
			    elem[e].material = elem[ei].material;
			    over = OFF;
			}
		    }
				

		    if (ej != OFF) {
			if (elem[ej].material != OFF && side[sj].mark == 0) {
			    elem[e].material = elem[ej].material;
			    over = OFF;
			}
		    }
				

		    if (ek != OFF) {
			if (elem[ek].material != OFF && side[sk].mark == 0) {
			    elem[e].material = elem[ek].material;
			    over = OFF;
			}
		    }
		}
	    }
	    if (over == ON) {
		break;
	    }
	} /* for(iter) */
    }
}
/*-materials---------------------------------------------------------------*/


/*=========================================================================*/
void relax()
{
    int s, T, E;
 
    for (T = 6; T >= 3; T--) {
	for (s = 0; s < Ns; s++) {
	    if (side[s].mark == 0) {
		if ((node[side[s].a].mark == 0) &&
		    (node[side[s].b].mark == 0) &&
		    (node[side[s].c].mark == 0) &&
		    (node[side[s].d].mark == 0)) {
		    E = node[side[s].c].Nne + node[side[s].d].Nne 
			- node[side[s].a].Nne - node[side[s].b].Nne;

		    if (E == T) {
			node[side[s].a].Nne++;
			node[side[s].b].Nne++; 
			node[side[s].c].Nne--;
			node[side[s].d].Nne--;  
			swap_side(s);
		    }
		}
	    }
	}
    }
}
/*-relax-------------------------------------------------------------------*/


/*=========================================================================*/
int smooth()
{
    int it, s, n, e;
 
    for (it = 0; it < 10; it++) {    
	for (s = 0; s < Ns; s++) {
		if (side[s].mark == 0) {
		    if (node[side[s].c].mark == 0) {
			node[side[s].c].sumx += node[side[s].d].x;
			node[side[s].c].sumy += node[side[s].d].y;
		    }
		    if (node[side[s].d].mark == 0) {
			node[side[s].d].sumx += node[side[s].c].x;
			node[side[s].d].sumy += node[side[s].c].y;
		    }
		}
	}
	for (n = 0; n < Nn; n++) {
	    if(node[n].mark == 0) {
		node[n].x = node[n].sumx/node[n].Nne;
		node[n].y = node[n].sumy/node[n].Nne;
		node[n].sumx = 0.0;
		node[n].sumy = 0.0;
	    }
	}
    }

    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF) {
	    circles(e);
	}
    }

    return 0;
}
/*-smooth------------------------------------------------------------------*/


/*=========================================================================*/
void renum()
{
    int n, s, e, c, d, i, j, k;
    int new_elem=0, new_node=0, new_side=0, next_e, next_s, lowest;

    for (n = 0; n < Nn; n++) {
	node[n].new_numb = OFF;
    }
    for (e = 0; e < Ne; e++) {
	elem[e].new_numb = OFF;
    }
    for (s = 0; s < Ns; s++) {
	side[s].new_numb = OFF;
    }

    /*-------------------------------+
    |  Searching the first element.  |
    |  It is the first which is ON   |
    +-------------------------------*/
    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF) {
	    break;
	}
    }

    /*----------------------------------------------------------+
    |  Assigning numbers 0 and 1 to the nodes of first element  |
    +----------------------------------------------------------*/
    node[elem[e].i].new_numb = new_node;
    new_node++;
    node[elem[e].j].new_numb = new_node;
    new_node++;

    /*%%%%%%%%%%%%%%%%%%%%%%%%%
    %                         %
    %  Renumeration of nodes  %
    %                         % 
    %%%%%%%%%%%%%%%%%%%%%%%%%*/
    printf("\nRenum Nodes, ");
    fflush(stdout);
    do {
	lowest = Nn + Nn;
	next_e = OFF;

	for (e = 0; e < Ne; e++) {
	    if (elem[e].mark != OFF && elem[e].new_numb == OFF) {
		i = node[elem[e].i].new_numb;
		j = node[elem[e].j].new_numb;
		k = node[elem[e].k].new_numb;

		if (i + j + k + 2 == abs(i) + abs(j) + abs(k)) {
		    if ((i == OFF) && (j + k) < lowest) {
			next_e = e;
			lowest = j + k;
		    }
		    if ((j == OFF) && (i + k) < lowest) {
			next_e = e;
			lowest = i + k;
		    }
		    if ((k == OFF) && (i + j) < lowest) {
			next_e = e;
			lowest = i + j;
		    }
		}
	    }
	}
	    

	if (next_e != OFF) {
	    i = node[elem[next_e].i].new_numb;
	    j = node[elem[next_e].j].new_numb;
	    k = node[elem[next_e].k].new_numb;

	    /*----------------------------------+
	    |  Assign a new number to the node  |
	    +----------------------------------*/
	    if (i == OFF) {
		node[elem[next_e].i].new_numb = new_node;
		new_node++;
	    }
	    if (j == OFF) {
		node[elem[next_e].j].new_numb = new_node;
		new_node++;
	    }
	    if (k == OFF) {
		node[elem[next_e].k].new_numb = new_node;
		new_node++;
	    }
	}
    } while (next_e != OFF);

    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %                             %
    %  Renumeration of triangles  %
    %                             %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    printf("Triangles, ");
    fflush(stdout);
    do {
	lowest = Nn + Nn + Nn;
	next_e = OFF;

	for (e = 0; e < Ne; e++) {
	    if (elem[e].mark != OFF && elem[e].new_numb == OFF) {
		i = node[elem[e].i].new_numb;
		j = node[elem[e].j].new_numb;
		k = node[elem[e].k].new_numb;

		if ((i + j + k) < lowest) {
		    next_e = e;
		    lowest = i + j + k;
		}
	    }
	}

	if (next_e != OFF) {
	    elem[next_e].new_numb = new_elem;
	    new_elem++;
	}
    } while (next_e != OFF);



    /*%%%%%%%%%%%%%%%%%%%%%%%%%
    %                         %
    %  Renumeration of sides  %
    %                         %
    %%%%%%%%%%%%%%%%%%%%%%%%%*/
    printf("Sides");
    fflush(stdout);
    do {
	lowest = Nn + Nn;
	next_s = OFF;

	for (s = 0; s < Ns; s++) {
	    if (side[s].mark != OFF && side[s].new_numb == OFF) {
		c = node[side[s].c].new_numb;
		d = node[side[s].d].new_numb;

		if ((c + d) < lowest) {
		    lowest = c + d;
		    next_s = s;
		}
	    }
	}

	if (next_s != OFF) {
	    side[next_s].new_numb = new_side;
	    new_side++;
	}
    } while (next_s != OFF);
    printf("\n");
}
/*-renum-------------------------------------------------------------------*/



/*=========================================================================*/
int load(void)
{
    int  c, n, s, M = 0, chains;
    double xt, yt, gab;
    int m;
    double xO, yO, zO, xN, yN, zN, L, Lx, Ly, Lz, dLm, ddL = 0, L_tot;
    int *inserted;

    memset(elem, 0, sizeof(elem));
    memset(side, 0, sizeof(side));
    memset(node, 0, sizeof(node));
    
    xmax = -GREAT;
    xmin = +GREAT;
    ymax = -GREAT;
    ymin = +GREAT;

    printf("Segments = %d\n", Fl);

    inserted = (int *)calloc(Nc, sizeof(int)); 
    chain   = (struct chai *)calloc(Fl + 1, sizeof(struct chai)); /* approximation */

    for (n = 0; n < Nc; n++) {
	xmax = max(xmax, point[n].x);
	ymax = max(ymax, point[n].y);
	xmin = min(xmin, point[n].x);
	ymin = min(ymin, point[n].y);
    }

    printf("xmin = %f   ymin = %f\n", xmin, ymin);
    printf("xmax = %f   ymax = %f\n", xmax, ymax);
    

    /*----------------------+
      counting the chains
     +----------------------*/
    chains = 0;
    chain[chains].s0 = 0;
    for (s = 0; s < Fl; s++) {
	point[segment[s].n0].inserted++;
	point[segment[s].n1].inserted++;

	segment[s].chain = chains;

	if (segment[s].n1 != segment[s+1].n0) {
	    chain[chains].s1 = s;
	    chains++;
	    chain[chains].s0 = s+1;
	}
    }

    printf("Chains = %d\n", chains);

    /*-------------------------------------+
      counting the nodes on each segment
     +-------------------------------------*/
    for (s = 0; s < Fl; s++) {
	xO = point[segment[s].n0].x;
	yO = point[segment[s].n0].y;
	xN = point[segment[s].n1].x;
	yN = point[segment[s].n1].y;

	Lx = (xN - xO);
	Ly = (yN - yO);
	L  = sqrt(Lx * Lx + Ly * Ly);

	if ((point[segment[s].n0].F + point[segment[s].n1].F > L ) &&
	    (segment[s].n0 != segment[s].n1)) {
	    point[segment[s].n0].F = min(point[segment[s].n0].F,L);
	    point[segment[s].n1].F = min(point[segment[s].n1].F,L);
	}
    }

    /*-------------------------------------+
      counting the nodes on each segment
     +-------------------------------------*/
    for (s = 0; s < Fl; s++) {
	xO = point[segment[s].n0].x;
	yO = point[segment[s].n0].y;
	xN = point[segment[s].n1].x;
	yN = point[segment[s].n1].y; 

	Lx = (xN - xO);
	Ly = (yN - yO);
	L  = sqrt(Lx * Lx + Ly * Ly);

	if (point[segment[s].n1].F + point[segment[s].n0].F <= L) {
	    dLm = 0.5 * (point[segment[s].n0].F + point[segment[s].n1].F);
	    segment[s].N = (int)ceil(L / dLm);
	} else {
	    segment[s].N = 1;
	}
    }


    for (n = 0; n < chains; n++) {
	if (segment[chain[n].s0].n0 == segment[chain[n].s1].n1) {
	    chain[n].type = CLOSED;
	}

	if (segment[chain[n].s0].n0 != segment[chain[n].s1].n1) {
	    chain[n].type = OPEN;
	}

	if ((point[segment[chain[n].s0].n0].inserted == 1) &&
	    (point[segment[chain[n].s1].n1].inserted == 1)) {
	    chain[n].type = INSIDE;
	}
    }

    /*------------+
    |             |
    |  Inserting  |
    |             |
    +------------*/
    xt = 0.5 * (xmax + xmin);
    yt = 0.5 * (ymax + ymin);

    gab = max((xmax - xmin), (ymax - ymin));
 
    Nn = 3;
    node[2].x = xt;
    node[2].y = yt + 2.8 * gab;
    node[2].z = point[0].z;

    node[0].x = xt - 2.0 * gab;
    node[0].y = yt - 1.4 * gab;
    node[0].z = point[0].z;

    node[1].x = xt + 2.0 * gab;
    node[1].y = yt - 1.4 * gab;
    node[1].z = point[0].z;

    node[2].inserted = 2;
    node[1].inserted = 2;
    node[0].inserted = 2;

    node[2].next = 1;
    node[1].next = 0;
    node[0].next = 2;

    Ne = 1;
    elem[0].i  = 0;
    elem[0].j  = 1;
    elem[0].k  = 2;
    elem[0].ei = -1;
    elem[0].ej = -1;
    elem[0].ek = -1;
    elem[0].si = 1;
    elem[0].sj = 2;
    elem[0].sk = 0;
 
    Ns = 3;
    side[0].c  = 0;
    side[0].d  = 1;
    side[0].a  = 2;
    side[0].b  = -1;

    side[1].c  = 1;
    side[1].d  = 2;
    side[1].a  = 0;
    side[1].b  = -1;
 
    side[2].c  = 0;
    side[2].d  = 2;
    side[2].a  = -1;
    side[2].b  = 1;

    side[0].ea =  0;
    side[0].eb = -1;

    side[1].ea =  0;
    side[1].eb = -1;

    side[2].ea = -1;
    side[2].eb = 0;


    for (n = 0; n < Nc; n++) {
	point[n].new_numb = OFF;
    }
    

    for (c = 0; c < chains; c++) {
	for (s = chain[c].s0; s <= chain[c].s1; s++) {
	    xO = point[segment[s].n0].x;
	    yO = point[segment[s].n0].y;
	    zO = point[segment[s].n0].z;
	    xN = point[segment[s].n1].x;
	    yN = point[segment[s].n1].y; 
	    zN = point[segment[s].n1].z;

	    /*===============
	    *  first point  *
	     ===============*/
	    if (point[segment[s].n0].new_numb == OFF ) {
		if (s == chain[c].s0) { /*  first segment in the chain */
		    insert_node(xO, yO, zO, OFF,
				OFF, OFF, point[segment[s].n0].mark, OFF, OFF);
		} else {
		    if (s == chain[c].s1 && segment[s].N == 1) {
			insert_node(xO, yO, zO, OFF,
				    Nn-1, segment[s-1].mark,
				    point[segment[s].n0].mark,
				    segment[s].mark, point[segment[chain[c].s0].n0].new_numb);
		    } else {
			insert_node(xO, yO, zO, OFF,
				    Nn-1, segment[s-1].mark, point[segment[s].n0].mark, OFF, OFF);
		    }
		}

		node[Nn-1].next     = Nn;     /* Nn-1 is index of inserted node */
		node[Nn-1].chain    = segment[s].chain;
		node[Nn-1].F        = point[segment[s].n0].F;
		point[segment[s].n0].new_numb = Nn-1;
	    }

	    Lx = (xN - xO);
	    Ly = (yN - yO);
	    Lz = (zN - zO);
	    L  = sqrt(Lx * Lx + Ly * Ly);
	    dLm=L/segment[s].N;

	    if (point[segment[s].n0].F + point[segment[s].n1].F <= L) { 
		if (point[segment[s].n0].F > point[segment[s].n1].F) {
		    M = -segment[s].N / 2;
		    ddL = (point[segment[s].n1].F-dLm) / M;
		} else {
		    M = +segment[s].N / 2;
		    ddL = (dLm - point[segment[s].n0].F) / M;
		}
	    }

	    /*=================
	     *  middle points  *
	     =================*/
	    L_tot = 0;
	    if (point[segment[s].n0].F + point[segment[s].n1].F <= L) {
		for (m = 1; m < abs(segment[s].N); m++) {
		    L_tot += (dLm - M * ddL);
  
		    if (point[segment[s].n0].F > point[segment[s].n1].F) {
			M++;
			if (M == 0 && segment[s].N % 2 == 0) {
			    M++;
			}
		    } else {
			M--;
			if (M == 0 && segment[s].N % 2 == 0) {
			    M--;
			}
		    }

		    if (s == chain[c].s1 && m == (abs(segment[s].N) - 1)) {
			insert_node(xO + Lx / L * L_tot, yO + Ly / L * L_tot, zO + Lz / L * L_tot, OFF,
				    Nn - 1, segment[s].mark, segment[s].mark, segment[s].mark, point[segment[s].n1].new_numb);
			node[Nn - 1].next = Nn;
		    } else {
			if(m == 1) {
			    insert_node(xO + Lx / L * L_tot, yO + Ly / L * L_tot, zO + Lz / L * L_tot, OFF,
					point[segment[s].n0].new_numb, segment[s].mark, segment[s].mark, OFF, OFF);
			    node[Nn - 1].next = Nn;
			} else {
			    insert_node(xO + Lx / L * L_tot, yO + Ly / L * L_tot, zO + Lz / L * L_tot, OFF,
					Nn - 1, segment[s].mark, segment[s].mark, OFF, OFF);
			    node[Nn - 1].next = Nn;
			}
		    }
			    

		    node[Nn - 1].chain    = segment[s].chain;
		    node[Nn - 1].F        = 0.5 * (node[Nn - 2].F + (dLm-M * ddL));
		}
	    }

	    /*==============
	     *  last point  * -> just for the inside chains
	     ==============*/
	    if ((point[segment[s].n1].new_numb == OFF) && (s==chain[c].s1)) {
		insert_node(xN, yN, zN, OFF,
			    Nn - 1, segment[s].mark, point[segment[s].n1].mark, OFF, OFF);
		node[Nn - 1].next     = OFF;
		node[Nn - 1].chain    = segment[s].chain;
		node[Nn - 1].F        = point[segment[s].n1].F;
	    }

	    if (chain[c].type == CLOSED && s == chain[c].s1) {
		node[Nn - 1].next = point[segment[chain[c].s0].n0].new_numb;
	    } 

	    if (chain[c].type == OPEN && s == chain[c].s1) {
		node[Nn-1].next = OFF;
	    }
		    
	}
    }
    
    free(segment);
    free(inserted);

    return 0;
}
/*-load--------------------------------------------------------------------*/

static int
insert_node_in_group(struct nod *nod, struct group *group)
{
    int		i;

    /* find if node is already present in this group */
    for (i = 0; i < group->nbvtx; i++) {
	if ((group->vertices[i].x == nod->x) &&
	    (group->vertices[i].y == nod->y) &&
	    (group->vertices[i].z == nod->z)) {
	    return i;
	}
    }
    /* insert new node */
    if (group->nbvtx == group->maxvtx) {
	group->maxvtx += VTX_INCR;
	group->vertices = (struct vtx	*)realloc(group->vertices, group->maxvtx * sizeof (struct vtx));
    }
    group->vertices[group->nbvtx].x = nod->x;
    group->vertices[group->nbvtx].y = nod->y;
    group->vertices[group->nbvtx].z = nod->z;
    group->nbvtx++;
    return (group->nbvtx - 1);
}

static void
insert_elem_in_group(struct ele *elem, struct nod *nods)
{
    int			grIdx;
    double		xmean, ymean;
    struct group	*curGrp;
    struct surf		*curSurf;

    xmean = (nods[elem->i].x + nods[elem->j].x + nods[elem->k].x) / 3.0;
    ymean = (nods[elem->i].y + nods[elem->j].y + nods[elem->k].y) / 3.0;

    grIdx = (int)((xmean - XGroupOffset) / GroupSize) + 
	XGroupNb * (int)((ymean - YGroupOffset) / GroupSize);
    
    curGrp = &(Groups[grIdx]);
    
    /* insert the surface */
    if (curGrp->nbsurf == curGrp->maxsurf) {
	if (curGrp->nbsurf == 0) {
	    ActiveGroups++;
	}
	curGrp->maxsurf += SURF_INCR;
	curGrp->surfaces = (struct surf	*)realloc(curGrp->surfaces, curGrp->maxsurf * sizeof (struct surf));
    }
    curSurf = &(curGrp->surfaces[curGrp->nbsurf++]);

    curSurf->ref[0].u = nods[elem->i].x / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    curSurf->ref[0].v = nods[elem->i].y / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    curSurf->ref[1].u = nods[elem->j].x / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    curSurf->ref[1].v = nods[elem->j].y / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    curSurf->ref[2].u = nods[elem->k].x / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    curSurf->ref[2].v = nods[elem->k].y / TexSize; // + (TexRand * rand()/(RAND_MAX+1.0));
    
    curSurf->ref[0].vtxidx = insert_node_in_group(&(nods[elem->i]), curGrp);
    curSurf->ref[1].vtxidx = insert_node_in_group(&(nods[elem->j]), curGrp);
    curSurf->ref[2].vtxidx = insert_node_in_group(&(nods[elem->k]), curGrp);
}

static void
groups(void)
{
    int  e, s, n, r_Nn=0, r_Ns=0, r_Ne=0;

    struct nod *r_node;
    struct ele *r_elem;
    struct sid *r_side;

    r_node = (struct nod *)calloc(Nn, sizeof(struct nod));
    r_elem = (struct ele *)calloc(Ne, sizeof(struct ele));
    r_side = (struct sid *)calloc(Ns, sizeof(struct sid));
    if (r_side == NULL) {
	fprintf(stderr, "Sorry, cannot allocate enough memory !\n");
	return ;
    }

    for (n = 0; n < Nn; n++) {
	if (node[n].mark != OFF && node[n].new_numb != OFF) {
	    r_Nn++;
	    r_node[node[n].new_numb].x    = node[n].x;
	    r_node[node[n].new_numb].y    = node[n].y;
	    if ((node[n].mark == 0) || (node[n].mark == 100000)) {
		r_node[node[n].new_numb].z = GetElevation(node[n].x, node[n].y, node[n].z);
	    } else {
		r_node[node[n].new_numb].z = node[n].z;
	    }
	    r_node[node[n].new_numb].mark = node[n].mark;
	}
    }
 

    for (e = 0; e < Ne; e++) {
	if (elem[e].mark != OFF && elem[e].new_numb != OFF) {
	    r_Ne++;
	    r_elem[elem[e].new_numb].i  = node[elem[e].i].new_numb;
	    r_elem[elem[e].new_numb].j  = node[elem[e].j].new_numb;
	    r_elem[elem[e].new_numb].k  = node[elem[e].k].new_numb;
	    r_elem[elem[e].new_numb].si = side[elem[e].si].new_numb;
	    r_elem[elem[e].new_numb].sj = side[elem[e].sj].new_numb;
	    r_elem[elem[e].new_numb].sk = side[elem[e].sk].new_numb;
	    r_elem[elem[e].new_numb].xv = elem[e].xv;
	    r_elem[elem[e].new_numb].yv = elem[e].yv;
	    r_elem[elem[e].new_numb].material = elem[e].material;

	    if (elem[e].ei != -1)
		r_elem[elem[e].new_numb].ei = elem[elem[e].ei].new_numb;
	    else
		r_elem[elem[e].new_numb].ei = -1;

	    if (elem[e].ej != -1)
		r_elem[elem[e].new_numb].ej = elem[elem[e].ej].new_numb;
	    else
		r_elem[elem[e].new_numb].ej = -1;

	    if (elem[e].ek != -1)
		r_elem[elem[e].new_numb].ek = elem[elem[e].ek].new_numb;
	    else
		r_elem[elem[e].new_numb].ek = -1;
	}
    }
 

    for (s = 0; s < Ns; s++) {
	if (side[s].mark != OFF && side[s].new_numb != OFF) {
	    r_Ns++;
	    r_side[side[s].new_numb].c    = node[side[s].c].new_numb;
	    r_side[side[s].new_numb].d    = node[side[s].d].new_numb;
	    r_side[side[s].new_numb].mark = side[s].mark;

	    if (side[s].a != OFF) {
		r_side[side[s].new_numb].a  = node[side[s].a].new_numb;
		r_side[side[s].new_numb].ea = elem[side[s].ea].new_numb;
	    } else {
		r_side[side[s].new_numb].a  = OFF;
		r_side[side[s].new_numb].ea = OFF;
	    }

	    if (side[s].b != OFF) {
		r_side[side[s].new_numb].b  = node[side[s].b].new_numb;
		r_side[side[s].new_numb].eb = elem[side[s].eb].new_numb;
	    } else {
		r_side[side[s].new_numb].b  = OFF;
		r_side[side[s].new_numb].eb = OFF;
	    }
	}
    }

    for (e = 0; e < r_Ne; e++) {
	insert_elem_in_group(&(r_elem[e]), r_node);
    }
}
/*-groups--------------------------------------------------------------------*/


static void
draw_ac(FILE *ac_file, char *name)
{
    int			i, j, k;
    struct group	*curGrp;

    Ac3dGroup(ac_file, name, ActiveGroups);

    for (i = 0; i < GroupNb; i++) {
	curGrp = &(Groups[i]);
	if (curGrp->nbsurf == 0) {
	    continue;
	}
	
	fprintf(ac_file, "OBJECT poly\n");
	fprintf(ac_file, "name \"%s%d\"\n", name, i);
	fprintf(ac_file, "texture \"%s\"\n", TexName);
	fprintf(ac_file, "numvert %d\n", curGrp->nbvtx);
	for (j = 0; j < curGrp->nbvtx; j++) {
	    fprintf(ac_file, "%g %g %g\n", curGrp->vertices[j].x, curGrp->vertices[j].z, -curGrp->vertices[j].y);
	}
	fprintf(ac_file, "numsurf %d\n", curGrp->nbsurf);
	for (j = 0; j < curGrp->nbsurf; j++) {
	    fprintf(ac_file, "SURF 0x30\n");
	    fprintf(ac_file, "mat 0\n");
	    fprintf(ac_file, "refs 3\n");
	    for (k = 0; k < 3; k++) {
		fprintf(ac_file, "%d %f %f\n", curGrp->surfaces[j].ref[k].vtxidx, curGrp->surfaces[j].ref[k].u, curGrp->surfaces[j].ref[k].v);
	    }
	}
	fprintf(ac_file, "kids 0\n");
    }
    
}
/*-draw_ac--------------------------------------------------------------------*/




static void
generate_mesh(void)
{
    int		Nn0;
    
    printf("Load Chains\n");
    load();
    erase();
    classify();
    
    do {
	Nn0 = Nn;
	new_node();
	classify();
	if (Nn > (MAX_NODES / 2 - 2)) {
	    break;
	}
	if (Nn == Nn0) {
	    break;
	}
    } while (ugly != OFF);

    neighbours();
    relax();
    smooth();
    renum();
    fflush(stdout);
    materials();
    groups();
}


static int
GetTrackOrientation(tTrack *track)
{
    tTrackSeg 	*seg;
    int		i;
    tdble	ang = 0;

    for (i = 0, seg = track->seg->next; i < track->nseg; i++, seg = seg->next) {
	switch (seg->type) {
	case TR_STR:
	    break;
	case TR_LFT:
	    ang += seg->arc;
	    break;
	case TR_RGT:
	    ang -= seg->arc;
	    break;
	}
    }
    if (ang > 0) {
	return ANTICLOCKWISE;
    }
    return CLOCKWISE;
}


#define ADD_POINT(_x,_y,_z,_F,_mark)			\
do {							\
    point[nbvert].x = _x;				\
    point[nbvert].y = _y;				\
    point[nbvert].z = _z;				\
    point[nbvert].F = _F;				\
    point[nbvert].mark = _mark;				\
    nbvert++;						\
    if (nbvert == maxVert) {				\
	maxVert *= 2;					\
	point = (struct nod*)realloc(point, maxVert);	\
	memset(&point[nbvert], 0, maxVert / 2);		\
    }							\
} while (0)

/** Generate the terrain mesh.
    @param	rightside	1 if use the right side
				0 if use the left side
    @param	reverse		1 if reverse the points order
				0 if keep the track order
    @param	exterior	1 if it is the exterior
				0 if it is the interior
    @param	
    @return	None.
*/
static void
GenerateMesh(tTrack *Track, int rightside, int reverse, int exterior)
{
    int		startNeeded;
    int		i, j, nbvert, maxVert;
    tdble	ts, step, anz;
    tTrackSeg 	*seg;
    tTrackSeg 	*mseg;
    tTrkLocPos 	trkpos;
    tdble	x, y;
    tdble 	radiusr, radiusl;
    struct nod	*point2;
    int		nb_relief_vtx, nb_relief_seg;

    printf("GenerateMesh: ");
    if (rightside) {
	printf("right, ");
    } else {
	printf("left, ");
    }
    if (reverse) {
	printf("reverse order, ");
    } else {
	printf("normal order, ");
    }
    if (exterior) {
	printf("exterior\n");
    } else {
	printf("interior\n");
    }

    CountRelief(1 - exterior, &nb_relief_vtx, &nb_relief_seg);

    printf("Relief: %d vtx, %d seg\n", nb_relief_vtx, nb_relief_seg);
    
    /* Estimation of the number of points */
    maxVert = ((int)(Track->length) + nb_relief_vtx) * sizeof(struct nod);
    point = (struct nod*)calloc(1, maxVert);
    
    if (rightside) {
	nbvert = 0;

	if (exterior && !reverse && UseBorder) {
	    ADD_POINT(-Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(-Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	}

	/* Right side */
	startNeeded = 1;
	for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next) {
	    if (mseg->rside != NULL) {
		seg = mseg->rside;
		if (seg->rside != NULL) {
		    seg = seg->rside;
		}
	    } else {
		seg = mseg;
	    }
	    if (startNeeded) {
		ADD_POINT(seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z, GridStep, i+1);
	    }
	    switch (seg->type) {
	    case TR_STR:
		ts = TrackStep;
		trkpos.seg = seg;
		while (ts < seg->length) {
		    trkpos.toStart = ts;
		    trkpos.toRight = 0;
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += TrackStep;
		}
		break;
	    case TR_LFT:
		step = TrackStep / (mseg->radiusr);
		anz = seg->angle[TR_ZS] + step;
		ts = step;
		radiusr = seg->radiusr;
		trkpos.seg = seg;
		trkpos.toRight = 0;
		while (anz < seg->angle[TR_ZE]) {
		    trkpos.toStart = ts;
		    /* right */
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += step;
		    anz += step;
		}
		break;
	    case TR_RGT:
		step = TrackStep / (mseg->radiusl);
		anz = seg->angle[TR_ZS] - step;
		ts = step;
		radiusr = seg->radiusr;
		trkpos.seg = seg;
		trkpos.toRight = 0;
		while (anz > seg->angle[TR_ZE]) {
		    trkpos.toStart = ts;
		    /* right */
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += step;
		    anz -= step;
		}
		break;
	    }
	    if (i != (Track->nseg - 1)) {
		ADD_POINT(seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z, GridStep, i+1);
		startNeeded = 0;
	    }
	}

	if (exterior && reverse && UseBorder) {
	    ADD_POINT(-Margin,Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(-Margin, -Margin, ExtHeight, GridStep, 100000);
	}

    } else {
	nbvert = 0;

	if (exterior && !reverse && UseBorder) {
	    ADD_POINT(-Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(-Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	}

	/* Left Side */
	startNeeded = 1;
	for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next) {
	    if (mseg->lside) {
		seg = mseg->lside;
		if (seg->lside) {
		    seg = seg->lside;
		}
	    } else {
		seg = mseg;
	    }
	    if (startNeeded) {
		ADD_POINT(seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z, GridStep, i+1);
	    }
	    
	    switch (seg->type) {
	    case TR_STR:
		ts = TrackStep;
		trkpos.seg = seg;
		while (ts < seg->length) {
		    trkpos.toStart = ts;
		    trkpos.toRight = RtTrackGetWidth(seg, ts);
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += TrackStep;
		}
		break;
	    case TR_LFT:
		step = TrackStep / (mseg->radiusr);
		anz = seg->angle[TR_ZS] + step;
		ts = step;
		radiusl = seg->radiusl;
		trkpos.seg = seg;
		while (anz < seg->angle[TR_ZE]) {
		    trkpos.toStart = ts;
		    trkpos.toRight = RtTrackGetWidth(seg, ts);
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    /* left */
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += step;
		    anz += step;
		}
		break;
	    case TR_RGT:
		step = TrackStep / (mseg->radiusl);
		anz = seg->angle[TR_ZS] - step;
		ts = step;
		radiusl = seg->radiusl;
		trkpos.seg = seg;
		while (anz > seg->angle[TR_ZE]) {
		    trkpos.toStart = ts;
		    trkpos.toRight = RtTrackGetWidth(seg, ts);
		    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
		    /* left */
		    ADD_POINT(x, y, RtTrackHeightL(&trkpos), GridStep, i+1);
		    ts += step;
		    anz -= step;
		}
		break;
	    }
	    if (i != (Track->nseg - 1)) {
		ADD_POINT(seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z, GridStep, i+1);
		startNeeded = 0;
	    }
	}

	if (exterior && reverse && UseBorder) {
	    ADD_POINT(-Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, Track->max.y + Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(Track->max.x + Margin, -Margin, ExtHeight, GridStep, 100000);
	    ADD_POINT(-Margin, -Margin, ExtHeight, GridStep, 100000);
	}
    }

    Nc = nbvert;
    segment = (struct seg *)calloc(Nc + 1 + nb_relief_seg, sizeof(struct seg));
    segment[Nc].n0 = -1;
    segment[Nc].n1 = -1;

    if (reverse){
	/* reverse order */
	point2 = (struct nod*)calloc(Nc + 1 + nb_relief_vtx, sizeof(struct nod));
	for (i = 0; i < Nc; i++) {
	    memcpy(&(point2[i]), &(point[Nc-i-1]), sizeof(struct nod));
	}
	free(point);
	point = point2;
    }

    Fl = 0;
    if (exterior && !UseBorder) {
	GenRelief(0);
    }
    if (exterior && UseBorder) {
	segment[0].n0 = 0;
	segment[0].n1 = 1;
	segment[0].mark = 100000;
	segment[1].n0 = 1;
	segment[1].n1 = 2;
	segment[1].mark = 100000;
	segment[2].n0 = 2;
	segment[2].n1 = 3;
	segment[2].mark = 100000;
	segment[3].n0 = 3;
	segment[3].n1 = 0;
	segment[3].mark = 100000;

	i = 0;
	j = 0;
	do {
	    segment[j+4].n0 = i+4;
	    i = (i + 1) % (Nc - 4);
	    segment[j+4].n1 = i+4;
	    segment[j+4].mark = 2;
	    j++;
	} while (i != 0);
	
    } else {
	i = 0;
	j = Fl;
	do {
	    segment[j].n0 = i;
	    i = (i + 1) % nbvert;
	    segment[j].n1 = i;
	    segment[j].mark = 1;
	    j++;
	} while (i != 0);
	Fl = j;
    }
    if (exterior) {
	if (UseBorder) {
	    Fl = Nc;
	    GenRelief(0);
	}
    } else {
	Fl = Nc;
	GenRelief(1);
    }
    segment[Fl].n0 = -1;
    segment[Fl].n1 = -1;
    generate_mesh();
}


void
GenerateTerrain(tTrack *track, void *TrackHandle, char *outfile, FILE *AllFd, int noElevation)
{
    char	*FileName;
    char	*mat;
    FILE	*curFd = NULL;

    TrackStep = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_TSTEP, NULL, 10.0);
    GfOut("Track step: %.2f ", TrackStep);
    Margin    = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BMARGIN, NULL, 100.0);
    GridStep  = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BSTEP, NULL, 10.0);
    ExtHeight = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BHEIGHT, NULL, 0.0);
    GfOut("Border margin: %.2f    step: %.2f    height: %.2f", Margin, GridStep, ExtHeight);
    
    GroupSize = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_GRPSZ, NULL, 100.0);
    XGroupOffset = track->min.x - Margin;
    YGroupOffset = track->min.y - Margin;

    XGroupNb = (int)((track->max.x + Margin - (track->min.x - Margin)) / GroupSize) + 1;
    
    GroupNb = XGroupNb * ((int)((track->max.y + Margin - (track->min.y - Margin)) / GroupSize) + 1);
    
    Groups = (struct group *)calloc(GroupNb, sizeof (struct group));
    ActiveGroups = 0;
    
    

    mat = GfParmGetStr(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_SURF, "grass");
    if (track->version < 4) {
	sprintf(buf, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, mat);
    } else {
	sprintf(buf, "%s/%s", TRK_SECT_SURFACES, mat);
    }
    TexName = GfParmGetStr(TrackHandle, buf, TRK_ATT_TEXTURE, "grass.rgb");
    TexSize = GfParmGetNum(TrackHandle, buf, TRK_ATT_TEXSIZE, (char*)NULL, 20.0);
    TexRand = GfParmGetNum(TrackHandle, buf, TRK_ATT_SURFRAND, (char*)NULL, TexSize / 10.0);

    FileName = GfParmGetStr(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_RELIEF, NULL);
    if (FileName) {
	sprintf(buf, "tracks/%s/%s/%s", track->category, track->internalname, FileName);
	LoadRelief(TrackHandle, buf);
    }
    if (noElevation == -1) {
	FileName = GfParmGetStr(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_ELEVATION, NULL);
	if (FileName) {
	    sprintf(buf, "tracks/%s/%s/%s", track->category, track->internalname, FileName);
	    LoadElevation(track, TrackHandle, buf);
	}
    }
    
	if (outfile) {
		// Attempt to fix AC3D (the application) segfault on opening the msh file.
		//curFd = Ac3dOpen(outfile, 2);
		curFd = Ac3dOpen(outfile, 1);
	}

    if (GetTrackOrientation(track) == CLOCKWISE) {

	GenerateMesh(track, 1 /* right */, 1 /* reverse */, 0 /* interior */);
	GenerateMesh(track, 0 /* left */,  0 /* normal */,  1 /* exterior */);
	if (curFd) {
	    draw_ac(curFd, "TERR");
	}
	if (AllFd) {
	    draw_ac(AllFd, "TERR");
	}
    } else {
	GenerateMesh(track, 0 /* left */,  0 /* normal */,  0 /* interior */);
	GenerateMesh(track, 1 /* right */, 1 /* reverse */, 1 /* exterior */);
	if (curFd) {
	    draw_ac(curFd, "TERR");
	}
	if (AllFd) {
	    draw_ac(AllFd, "TERR");
	}
    }
    if (curFd) {
	Ac3dClose(curFd);
    }
    
}
