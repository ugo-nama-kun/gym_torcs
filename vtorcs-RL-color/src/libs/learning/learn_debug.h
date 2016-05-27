#ifndef DEBUG_H
#define DEBUG_H

#include <learning/real.h>

#ifdef _MSC_VER
#define Serror   printf("# ERROR (%s:  %d) ", __FILE__, __LINE__); printf
#define Swarning  printf("# WARNING (%s:  %d) ", __FILE__, __LINE__); printf
#else // _MSC_VER
#define Serror   printf("# ERROR (%s - %s:  %d) ", __FUNCTION__,__FILE__, __LINE__); printf
#define Swarning  printf("# WARNING (%s - %s:  %d) ", __FUNCTION__,__FILE__, __LINE__); printf
#endif // _MSC_VER


//#define logmsg (void)
//#define dbgmsg (void)
#define logmsg printf ("# "); printf
#define dbgmsg printf ("# "); printf

#define Smessage printf("# "); printf
//#define error printf("# ERROR: %s:%d ",__FILE__,__LINE__); printf
//#define warning printfww1("# Warning: %s:%d ",__FILE__,__LINE__); printfww2
 


#define ReadFile(f,fs) ((fs=fopen(f,"rb"))!=NULL)
#define WriteFile(f,fs) ((fs=fopen(f,"wb"))!=NULL)
#define CloseFile(f) {assert(f); fclose(f); f = NULL;}

#define AllocM(type,x) ((type *) malloc (sizeof(type)*(x)))
#define FreeM(address) if (address) {free(address); address = NULL;} else fprintf(stderr,"ERROR: pointer already freed\n");

#define ObjCopy(new_obj,cl_nam) (memcpy (new_obj, &cl_nam ## _class, sizeof(cl_nam)))


/* Some cool error codes to use */
#define DEC_MEM_ERROR    0x0001
#define DEC_RES_INVALID  0x0002
#define DEC_SANITY_ERROR 0x0004
#define DEC_ARG_INVALID  0x1000
#define DEC_ARG_CORRUPT  0x2000


#define DEC_MASK 0xffff

//typedef float real;


int ShowError(int ec);


long FileLength(char* fname);

#endif
