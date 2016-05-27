/***************************************************************************

    file        : main.cpp
    created     : Tue Jul 29 19:45:27 CEST 2003
    copyright   : (C) 2003 by Christophe Guionneau                       
    version     : $Id: mainnfs2ac.cpp,v 1.4 2005/02/01 14:54:58 berniw Exp $                                  

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef WIN32
#include <windows.h>
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>

char  * mem;
typedef struct {
    float x;
    float y;
    float z;
} tVector;


typedef struct {
    int h;
    int s;
    int b;
    int t;
} tColor;

typedef struct {
    int    TexPage;
    int    I1 ;
    int    I2 ;
    int    I3 ;
    char  unknown [12];
    int    Smoothing;
    float   U1;
    float   U2 ;
    float   U3;
    float   V1;
    float   V2;
    float   V3;
} tTriangle;

int main(int argc, char **argv)
{ 
    FILE * in ;
    int ver;
    int num=0 ;
    //char c;
    char *p;
    int i;
    int k;
    int magic;
    int VertTblOffset;
    int TriaTblOffset;
    int NormTblOffset;
    int         NumParts ;
    tVector  * Parts;
    int     * P1stVertices;
    int     * PNumVertices;
    int     * P1stTriangle;
    int     * PNumTriangles;
    char     * PartNames;
    char     * vl, * tl;
    tTriangle * tr;
    tVector   *vi;
    struct stat fstat;
    
	if (argc < 2) {
		printf("Please give the filename as argument.\n");
		return -1;
	}

    if (stat(argv[1], &fstat)) {
		perror(argv[1]);
		return -1;
    }
    
    num = fstat.st_size;
    
    mem= (char *) malloc(num+1);

    in=fopen(argv[1],"rb");
    p=mem;

    if ((int)fread(p,1,num,in) != num)
	exit(-2);
  
    magic         = * ((int *)(mem));

    if (magic == 0x00101014) {
	ver = 4;			/* NFS 4 */
	VertTblOffset = * ((int *)(mem + 0x0014));
	NormTblOffset = * ((int *)(mem + 0x0018));
	TriaTblOffset = * ((int *)(mem + 0x001C));
	NumParts      = * ((int *)(mem + 0x011C));
	Parts         = (tVector *)(mem + 0x0120);
	P1stVertices  =    (int *)(mem + 0x0420);
	PNumVertices  =    (int *)(mem + 0x0520);
	P1stTriangle  =    (int *)(mem + 0x0620);
	PNumTriangles =    (int *)(mem + 0x0720);
	PartNames     =    (char *)(mem + 0x0E28);
	p             =             mem + 0x2038;
    } else {
	ver = 3;			/* NFS 3 */
	VertTblOffset = * ((int *)(mem + 0x0010));
	NormTblOffset = * ((int *)(mem + 0x0014));
	TriaTblOffset = * ((int *)(mem + 0x0018));
	NumParts      = * ((int *)(mem + 0x00F8));
	Parts         = (tVector *)(mem + 0x00FC);
	P1stVertices  =    (int *)(mem + 0x03FC);
	PNumVertices  =    (int *)(mem + 0x04FC);
	P1stTriangle  =    (int *)(mem + 0x05FC);
	PNumTriangles =    (int *)(mem + 0x06FC);
	PartNames     =    (char *)(mem + 0x0E04);
	p             =             mem + 0x1F04;
    }
  


    printf("AC3Db\n");
    printf("MATERIAL \"ac3dmat1\" rgb 0.4 0.4 0.4  amb 0.8 0.8 0.8  emis 0.4 0.4 0.4  spec 0.5 0.5 0.5  shi 50  trans 0\n");
    printf("OBJECT world\n");
    printf("kids 1\n");

    printf("OBJECT group\n");
    printf("name \"ouature%d\"\n", ver);
    printf("kids %d\n", NumParts);

  
    for (i = 0; i < NumParts; i++) {
	vl= (p + P1stVertices[i] * sizeof(tVector) + VertTblOffset);
	tl= (p + P1stTriangle[i] * sizeof(tTriangle) + TriaTblOffset);

	printf("OBJECT poly\n");
	printf("name \"%s\"\n", PartNames + 64 * i);
	printf("texture \"car00.tga\"\n");

	printf("numvert %d\n",PNumVertices[i]);
	vi= (tVector *) vl;
	for (k=0 ; k<PNumVertices[i]; k++)
	    {
		printf("%f %f %f \n", vi[k].z + Parts[i].z, vi[k].y + Parts[i].y, vi[k].x + Parts[i].x);
	    }
	printf("numsurf %d\n",PNumTriangles[i]);
	for (k=0 ; k<PNumTriangles[i]; k ++)
	    {
		printf("SURF 0x10\n");
		printf("mat 1\n");
		printf("refs 3\n");
		tr=(tTriangle *) ( (sizeof(tTriangle)*k) + tl);
		vi= (tVector *) vl;
		printf("%d %f %f\n",tr->I1, tr->U1, 1.0 - tr->V1);
		printf("%d %f %f\n",tr->I2, tr->U2, 1.0 - tr->V2);
		printf("%d %f %f\n",tr->I3, tr->U3, 1.0 - tr->V3);
	    }
	printf("kids 0\n");
    }

    return (0);
}

