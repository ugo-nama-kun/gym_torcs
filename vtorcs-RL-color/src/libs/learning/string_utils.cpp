// Copyright (C) 2004 Johnny Mariethoz (Johnny.Mariethoz@idiap.ch)
//                and Samy Bengio (bengio@idiap.ch)
//                and Ronan Collobert (collober@iro.umontreal.ca)
//                and Christos Dimitrakakis (dimitrak@idiap.ch)
//
// This file is based on code from Torch. Release II.
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <learning/string_utils.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <portability.h>

static char msgbuf[10000];
static FILE* msgport = stderr;

char *strBaseName(char *filename) {
   char *p = strrchr(filename, '/');
   return p ? (p+1) : filename;
}

char *strRemoveSuffix(char *filename, char c)
{
  char *copy = NULL;
  int len = strlen(filename);
  char *p = filename + len - 1;
  int i=len;
  while (*p != c && i-- >=0) p--;
  if (i>0) {
    //*p = '\0';
    copy = (char*)malloc(sizeof(char)*i);
    strncpy(copy,filename,i-1);
    copy[i-1] = '\0';
  } else {
    copy = (char*)malloc(sizeof(char)*(len+1));
    strcpy(copy,filename);
  }
  return copy;
}

char *strConcat(int n, ...)
{
  char **strs = (char **)malloc(sizeof(char *)*n);

  int taille = 0;
  va_list args;
  va_start(args, n);
  int i;
  for(i = 0; i < n; i++)
  {
    strs[i] = va_arg(args, char *);
    taille += strlen(strs[i]);
  }
  va_end(args);
  taille++; // Pour le truc de fin

  char *the_concat = (char *)malloc(sizeof(char)*taille);
  the_concat[0] = '\0';

  for(i = 0; i < n; i++)
    strcat(the_concat, strs[i]);

  free(strs);

  return(the_concat);
}



void message(const char* msg, ...)
{
  va_list args;
  va_start(args,msg);
  vsprintf(msgbuf, msg, args);
  fprintf(msgport, "# %s\n", msgbuf);
  fflush(stdout);
  va_end(args);
}
#define INIT_MSG_LEN 100


//==================================================================== 
// read_string()                             Dynamically read a string
//--------------------------------------------------------------------
StringBuffer* read_string (FILE* f, StringBuffer* s)
{
     unsigned int l;
     fpos_t pos;

     if (s==NULL) {
	  s = NewStringBuffer (10);
     }

     while (1) {
	  l = s->length;
	  if (fgetpos (f, &pos)) {
	       printf ("Error getting position\n");
	  }
	  if ((s->string = fgets (s->c, l, f)) == NULL) {
	       return s;
	  }
	  if (strlen(s->string)<(l-1)) {
	       return s;
	  }

	  if (fsetpos (f, &pos)) {
	       printf ("Error setting position\n");
	  }
	  s->length += l;
	  if ((s->c = (char*) realloc (s->c, s->length)) == NULL) {
	       fprintf (stderr, "Oops, out of RAM\n");
	       FreeStringBuffer (&s);
	       return NULL;
	  }
     }
	  
     
}



StringBuffer* SetStringBufferLength (StringBuffer* s, unsigned int l)
{
	if (l > s->length) {
		s->length = l;
		if ((s->c = (char*) realloc (s->c, s->length)) == NULL) {
			fprintf (stderr, "Oops, out of RAM\n");
			FreeStringBuffer (&s);
			return NULL;
		}
	}
	return s;
}

StringBuffer* NewStringBuffer (int length)
{
     StringBuffer* s = AllocM (StringBuffer, 1);
     if (s==NULL) {
	  return NULL;
     }
     s->length = length;
     if ((s->c = AllocM (char, length))==NULL) {
	  FreeStringBuffer (&s);
     }
     return s;
}

void FreeStringBuffer (StringBuffer** s)
{
     if ((*s)->c) {
	  FreeM ((*s)->c);
     }
     FreeM ((*s));
     s = NULL;
}

char* string_copy (char* c)
{
     char* r;
     if (c==NULL) return NULL;
     r = AllocM (char, (strlen(c)+1));
     strcpy (r, c);
     return r;
}
