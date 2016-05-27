/***************************************************************************

    file                 : xml.cpp
    created              : Sat Mar 18 23:50:46 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: xml.cpp,v 1.4.2.1 2008/11/09 17:50:22 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 *	This set of function is used to store and retrieve
 *	values in parameters files written in XML.
 */

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include "xmlparse.h"
#include <xml.h>

#define BUFMAX	256

/*
 * NAME:
 *	NewElt
 *
 * FUNCTION:
 *	Creates a new element
 *
 * PARAMETERS:	
 *	name and attributes
 *
 * RETURNS:	
 *	new element created
 *
 */
static txmlElement *
NewElt(const char *name, const char **atts)
{
    int			nAtts;
    const char		**p;
    const char		*s1, *s2;
    txmlElement		*newElt;
    txmlAttribute	*newAttr;
    
    
    /* Create a new element */
    if ((newElt = (txmlElement*)malloc(sizeof(txmlElement))) == NULL) {
	return (txmlElement*)NULL;
    }
    newElt->name   = strdup(name);
    newElt->pcdata = (char*)NULL;
    newElt->attr   = (txmlAttribute*)NULL;
    newElt->sub    = (txmlElement*)NULL;
    newElt->up     = (txmlElement*)NULL;
    newElt->next   = newElt;
    newElt->level  = 0;

    /* attributes */
    p = atts;
    while (*p)
	++p;
    nAtts = (p - atts) >> 1;
    if (nAtts > 1) {
	qsort((void *)atts, nAtts, sizeof(char *) * 2, (int (*)(const void *, const void *))strcmp);
    }

    while (*atts) {
	s1 = *atts++;
	s2 = *atts++;
	if ((newAttr = (txmlAttribute*)malloc(sizeof(txmlAttribute))) == NULL) {
	    return (txmlElement*)NULL;
	}
	newAttr->name = strdup(s1);
	newAttr->value = strdup(s2);
	/* insert in attributes ring */
	if (newElt->attr == NULL) {
	    newElt->attr = newAttr;
	    newAttr->next = newAttr;
	} else {
	    newAttr->next = newElt->attr->next;
	    newElt->attr->next = newAttr;
	    newElt->attr = newAttr;
	}
    }

    return newElt;
}


/*
 * NAME:
 *	xmlInsertElt
 *
 * FUNCTION:
 *	Create and Insert a new element in the sub-list of
 *	the element given in parameter
 *
 * PARAMETERS:	
 *	curElt	element
 *	name	name of the new element
 *	atts	attribute list of the new element
 *
 * RETURNS:	
 *	the new element created.
 *
 * NOTE:
 *	Use NULL as current element to create a new tree
 */
txmlElement *
xmlInsertElt(txmlElement *curElt, const char *name, const char **atts)
{
    txmlElement		*newElt;
    
    newElt = NewElt(name, atts);
    
    if (curElt) {
	if (curElt->sub == NULL) {
	    curElt->sub = newElt;
	    newElt->next = newElt;
	} else {
	    newElt->next = curElt->sub->next;
	    curElt->sub->next = newElt;
	    curElt->sub = newElt;
	}
	newElt->up = curElt;
	newElt->level = curElt->level + 1;
    }
    
    return newElt;
}

/*
 * Function
 *	startElement
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 */
static void 
startElement(void *userData, const char *name, const char **atts)
{
    txmlElement **curElt = (txmlElement **)userData;
    
    *curElt = xmlInsertElt(*curElt, name, atts);
}

/*
 * Function
 *	endElement
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	none
 */
static void 
endElement(void *userData, const char * /* name */)
{
    txmlElement **curElt = (txmlElement **)userData;
    
    if ((*curElt)->up != NULL) {
	*curElt = (*curElt)->up;
    }
}

static void 
CharacterData(void *userData, const char *s, int len)
{
    char *s1,*s2,*s3;
    txmlElement **curElt = (txmlElement **)userData;
    
    if ((s1 = (char*)malloc(len+1)) == NULL) {
	return;
    }
    strncpy(s1, s, len);

    /* remove spaces in front */
    s2 = s1;
    while ((*s2 == ' ') || (*s2 == '\t') || (*s2 == '\n')) {
	s2++;
    }
    
    /* remove spaces at the end */
    s3 = s1+len-1;
    while (((*s3 == ' ') || (*s3 == '\t') || (*s3 == '\n')) && (s3 > s2)) {
	s3--;
    }
    
    if (s3 > s2) {
	*(s3+1) = 0;
	(*curElt)->pcdata = strdup(s2);
    }
    free(s1);
}

/*
 * Function
 *	xmlReadFile
 *
 * Description
 *	Read a config file
 *
 * Parameters
 *	file	name of the config file
 *
 * Return
 *	root of the tree
 *	NULL	error.
 *
 * Remarks
 *	
 */
txmlElement *
xmlReadFile(const char *file)
{
    FILE		*in;
    char		buf[BUFSIZ];
    XML_Parser		parser;
    int			done;
    txmlElement		*retElt;
	
    if ((in = fopen(file, "r")) == NULL) {
        fprintf(stderr, "xmlReadFile: file %s has pb (access rights ?)\n", file);
	return (txmlElement*)NULL;
    }
    
    parser = XML_ParserCreate((XML_Char*)NULL);
    XML_SetUserData(parser, &retElt);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser, CharacterData);
    do {
	size_t len = fread(buf, 1, sizeof(buf), in);
	done = len < sizeof(buf);
	if (!XML_Parse(parser, buf, len, done)) {
	    fprintf(stderr, "file: %s -> %s at line %d\n",
		   file,
		   XML_ErrorString(XML_GetErrorCode(parser)),
		   XML_GetCurrentLineNumber(parser));
	    return (txmlElement*)NULL;
	}
    } while (!done);
    XML_ParserFree(parser);

    return retElt;
}

static void
wr(int indent, char *buf, FILE *out)
{
    char		blank[BUFMAX];
    int			i;
    
    for(i = 0; i < indent*2; i++) blank[i] = ' ';
    blank[i] = 0;
    fprintf(out, "%s%s", blank, buf);
}

static void
wrrec(txmlElement *startElt, FILE *out)
{
    txmlElement		*curElt;
    txmlAttribute	*curAttr;
    char		buf[BUFMAX];
    
    curElt = startElt;
    
    if (curElt) {
	wr(0, "\n", out);
	do {
	    curElt = curElt->next;
	    sprintf(buf, "<%s", curElt->name);
	    wr(curElt->level, buf, out);
	    curAttr = curElt->attr;
	    if (curAttr) {
		do {
		    curAttr = curAttr->next;
		    sprintf(buf, " %s=\"%s\"", curAttr->name, curAttr->value);
		    wr(0, buf, out);
		} while (curAttr != curElt->attr);
	    }
	    sprintf(buf, ">");
	    wr(0, buf, out);
	    if (curElt->pcdata) {
		sprintf(buf, "%s", curElt->pcdata);
		wr(0, buf, out);
	    }
	    /* recurse the nested elements */
	    wrrec(curElt->sub, out);
	    sprintf(buf, "</%s>\n", curElt->name);
	    wr(0, buf, out);
	} while (curElt != startElt);
	wr(curElt->level-1, "", out);
    }
}

/*
 * Function
 *	xmlWriteFile
 *
 * Description
 *	Write a parameter file
 *
 * Parameters
 *	file		name of the config file
 *	startElt	root of the tree to write
 *
 * Return
 *	0	ok
 *	-1	failed
 *
 * Remarks
 *	the file is created if necessary
 */
int
xmlWriteFile(const char *file, txmlElement *startElt, char *dtd)
{
    char		buf[BUFMAX];
    FILE		*out;

    if ((out = fopen(file, "w")) == NULL) {
	fprintf(stderr, "xmlWriteFile: file %s has pb (access rights ?)\n", file);
	return -1;
    }

    sprintf(buf, "<?xml version=\"1.0\" ?>\n");
    wr(0, buf, out);
    sprintf(buf, "\n<!DOCTYPE params SYSTEM \"%s\">\n\n", dtd);
    wr(0, buf, out);

    wrrec(startElt, out);		
    wr(0, "\n", out);
    
    fclose(out);
    return 0;
}

/*
 * NAME:
 *	xmlGetAttr
 *
 * FUNCTION:
 *	Get the attribute value of an element
 *
 * PARAMETERS:	
 *	curElt		element to consider
 *	attrname	name of the attribute
 *
 * RETURNS:	
 *	the attribute value or NULL if attribute is not present
 *
 */
char *
xmlGetAttr(txmlElement *curElt, char *attrname)
{
    txmlAttribute	*cutAttr;

    cutAttr = curElt->attr;
    if (cutAttr) {
	do {
	    cutAttr = cutAttr->next;
	    if (strcmp(cutAttr->name, attrname) == 0) {
		return strdup(cutAttr->value);
	    }
	} while (cutAttr != curElt->attr);
    }
    return (char*)NULL;
}

/*
 * NAME:
 *	xmlNextElt
 *
 * FUNCTION:
 *	Get the next element (same level)
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *
 * RETURNS:	
 *	the next element or NULL if no more element
 *	at the same level
 *
 */
txmlElement *
xmlNextElt(txmlElement *startElt)
{
    txmlElement *curElt;
    
    curElt = startElt->next;
    if (curElt->up != NULL) {
	if (curElt->up->sub->next == curElt) {
	    return (txmlElement*)NULL;
	}
	return curElt;
    }
    return (txmlElement*)NULL;
}


/*
 * NAME:
 *	xmlSubElt	
 *
 * FUNCTION:
 *	Get the first sub-element (nested level)
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *
 * RETURNS:	
 *	the first sub-element or NULL if no sub-element
 *
 */
txmlElement *
xmlSubElt(txmlElement *startElt)
{
    if (startElt->sub != NULL) {
	return startElt->sub->next;
    }
    
    return (txmlElement*)NULL;
}


/*
 * NAME:
 *	xmlWalkElt
 *
 * FUNCTION:
 *	Walk all the tree
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *
 * RETURNS:	
 *	the next element in the tree or NULL if all the tree
 *	has been parsed.
 *
 */
txmlElement *
xmlWalkElt(txmlElement *startElt)
{
    txmlElement *curElt;
    
    curElt = startElt;
    /* in depth first */
    if (curElt->sub != NULL) {
	return curElt->sub->next;
    }

    /* go to the next element */
    if ((curElt->up != NULL) && (curElt != curElt->up->sub)) {
	return curElt->next;
    }
    
    /* end of the ring should go upward */
    while (curElt->up != NULL) {
	curElt = curElt->up;
	if ((curElt->up != NULL) && (curElt != curElt->up->sub)) {
	    return curElt->next;
	}
    }

    return (txmlElement*)NULL;
}


/*
 * NAME:
 *	xmlWalkSubElt
 *
 * FUNCTION:
 *	walk a sub-tree
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *	topElt		sub-tree root
 *
 * RETURNS:	
 *	next element in in-depth search or NULL if all sub-tree has been parsed
 *
 */
txmlElement *
xmlWalkSubElt(txmlElement *startElt, txmlElement *topElt)
{
    txmlElement *curElt;
    
    curElt = startElt;
    /* in depth first */
    if (curElt->sub != NULL) {
	return curElt->sub->next;
    }

    /* go to the next element */
    if ((curElt->up != NULL) && (curElt != curElt->up->sub) && (curElt != topElt)) {
	return curElt->next;
    }
    
    /* end of the ring should go upward */
    while ((curElt->up != NULL) && (curElt != topElt)) {
	curElt = curElt->up;
	if ((curElt->up != NULL) && (curElt != curElt->up->sub)) {
	    return curElt->next;
	}
    }

    return (txmlElement*)NULL;
}


/*
 * NAME:
 *	xmlFindNextElt
 *
 * FUNCTION:
 *	Find the next element corresponding to name
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *	name		name of the element to find
 *
 * RETURNS:	
 *	pointer on next element corresponding to name
 *	or NULL if no more.
 *
 */
txmlElement *
xmlFindNextElt(txmlElement *startElt, char *name)
{
    txmlElement		*curElt;
    
    curElt = startElt;
    curElt = xmlWalkElt(curElt);
    while (curElt) {
	if (strcmp(curElt->name, name) == 0) {
	    return curElt;
	}
	curElt = xmlWalkElt(curElt);
    }
    return (txmlElement*)NULL;
}    


/*
 * NAME:
 *	xmlFindEltAttr
 *
 * FUNCTION:
 *	Find an element with its name an an attribute value
 *
 * PARAMETERS:	
 *	startElt	element to start with
 *	name		name of the element to search
 *	attrname	attribute name
 *	attrvalue	attribute value of the searched element
 *
 * RETURNS:	
 *	the corresponding element or NULL if not found
 *
 */
txmlElement *
xmlFindEltAttr(txmlElement *startElt, char *name, char *attrname, char *attrvalue)
{
    txmlElement		*curElt;
    txmlAttribute	*cutAttr;
    
    curElt = startElt;
    curElt = xmlWalkElt(curElt);
    while (curElt) {
	if (strcmp(curElt->name, name) == 0) {
	    cutAttr = curElt->attr;
	    if (cutAttr) {
		do {
		    cutAttr = cutAttr->next;
		    if (strcmp(cutAttr->name, attrname) == 0) {
			if (strcmp(cutAttr->value, attrvalue) == 0) {
			    return curElt;
			} else {
			    cutAttr = curElt->attr;
			}
		    }
		} while (cutAttr != curElt->attr);
	    }
	}
	curElt = xmlWalkElt(curElt);
    }
    return (txmlElement*)NULL;
}






