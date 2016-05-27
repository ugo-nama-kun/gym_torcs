/***************************************************************************
                 params.cpp -- configuration parameters management
                             -------------------
    created              : Fri Aug 13 22:27:57 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: params.cpp,v 1.30.2.3 2008/11/09 17:50:22 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
    		This is the parameters manipulation API.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: params.cpp,v 1.30.2.3 2008/11/09 17:50:22 berniw Exp $
    @ingroup	params
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <math.h>

#include <xmlparse.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgf.h>

#include <assert.h>


#define LINE_SZ		1024

#define PARAM_CREATE	0x01

struct within
{
    char				*val;
    GF_TAILQ_ENTRY (struct within)	linkWithin;
};

GF_TAILQ_HEAD (withinHead, struct within);

#define P_NUM 0
#define P_STR 1

/** Parameter header structure */
struct param
{
    char				*name;		/**< Name of the parameter  */
    char				*fullName;	/**< Name of the parameter including the full section name ('/' separated) */
    char				*value;		/**< Value of the parameter */
    tdble				valnum;
    int					type;
    char				*unit;		/* for output only */
    tdble				min;
    tdble				max;
    struct withinHead			withinList;
    GF_TAILQ_ENTRY (struct param)	linkParam;	/**< Next parameter in the same section */
};

GF_TAILQ_HEAD (paramHead, struct param);
struct section;
GF_TAILQ_HEAD (sectionHead, struct section);


/** Section header structure */
struct section
{
    char *fullName;	/**< Name of the section including full path ('/' separated) */
    struct paramHead			paramList;	/**< List of the parameters of this section */
    GF_TAILQ_ENTRY (struct section)	linkSection;	/**< Next section at the same level */
    struct sectionHead			subSectionList;	/**< List of sub-sections (linked by linkSection)*/
    struct section			*curSubSection;
    struct section			*parent;	/**< Upper section */
};


#define PARM_MAGIC	0x20030815

/** Configuration header structure */
struct parmHeader
{
    char				*filename;	/**< Name of the configuration file */
    char				*name;		/**< Name of the data */
    char				*dtd;		/**< Optional DTD location */
    char				*header;	/**< Optional header (comment, xsl...) */
    int					refcount;	/**< Use counter (number of conf handle) */
    struct section			*rootSection;	/**< List of sections at the first level */
    void				*paramHash;	/**< Hash table for parameter access */
    void				*sectionHash;	/**< Hash table for section access */
};

#define PARM_HANDLE_FLAG_PRIVATE	0x01
#define PARM_HANDLE_FLAG_PARSE_ERROR	0x02


/** Ouput control structure */
struct parmOutput
{
    int			state;
    struct section	*curSection;
    struct param	*curParam;
    char		*filename;	/**< Name of the output configuration file */
    char		*indent;
};


/** Configuration handle structure */
struct parmHandle
{
	int magic;
	struct parmHeader *conf;
	char *val;
	int flag;
	XML_Parser parser;
	struct section *curSection;
	struct parmOutput outCtrl;
	GF_TAILQ_ENTRY (struct parmHandle)	linkHandle;	/**< Next configuration handle */
};


GF_TAILQ_HEAD (parmHead, struct parmHandle);

static struct parmHead	parmHandleList;

static char *getFullName(const char *sectionName, const char *paramName);
static struct param *getParamByName (struct parmHeader *conf, const char *sectionName, const char *paramName, int flag);
static void removeParamByName (struct parmHeader *conf, char *sectionName, char *paramName);
static void removeParam (struct parmHeader *conf, struct section *section, struct param *param);
static struct param *addParam (struct parmHeader *conf, struct section *section, const char *paramName, const char *value);
static void removeSection (struct parmHeader *conf, struct section *section);
static struct section *addSection (struct parmHeader *conf, const char *sectionName);
static void parmClean (struct parmHeader *conf);
static void parmReleaseHandle (struct parmHandle *parmHandle);
static void parmReleaseHeader (struct parmHeader *conf);
static struct section *getParent (struct parmHeader *conf, const char *sectionName);
static void cleanUnusedSection (struct parmHeader *conf, struct section *section);


// GIUSE - let's free the struct param* properly when needed
static void freeparam(struct param* param){
  if( param ){
    freez (param->name);
    freez (param->fullName);
    freez (param->value);
    freez (param->unit);
    freez (param);
  }
}

/** Configuration initialization.
    @ingroup	conf
    @return	none.
*/
void
gfParamInit (void)
{
    GF_TAILQ_INIT (&parmHandleList);
}

/** Configuration shutdown.
    @ingroup	conf
    @return	none
*/
void
GfParmShutdown (void)
{
    struct parmHandle	*parmHandle;

    while ((parmHandle = GF_TAILQ_FIRST (&parmHandleList)) != GF_TAILQ_END (&parmHandleList)) {
	parmReleaseHandle (parmHandle);
    }
}

/* Compute parameter full name           */
/* Caller must release the returned name */
static char *
getFullName (const char *sectionName, const char *paramName)
{
	char *fullName;

	fullName = (char *) malloc (strlen (sectionName) + strlen (paramName) + 2);
	if (!fullName) {
		GfError ("getFullName: malloc (%d) failed", strlen (sectionName) + strlen (paramName) + 2);
		return NULL;
	}
	sprintf (fullName, "%s/%s", sectionName, paramName);

	return fullName;
}

/* Get a parameter by section/param names */
static struct param *
getParamByName (struct parmHeader *conf, const char *sectionName, const char *paramName, int flag)
{
	char *fullName;
	struct param *param;
	struct section *section;

	fullName = getFullName (sectionName, paramName);
	if (!fullName) {
		GfError ("getParamByName: getFullName failed\n");
		return NULL;
	}

	param = (struct param *)GfHashGetStr (conf->paramHash, fullName);
	free (fullName);
	if (param || ((flag & PARAM_CREATE) == 0)) {
		return param;
	}

	/* Parameter not found CREATE it */
	section = (struct section *)GfHashGetStr (conf->sectionHash, sectionName);
	if (!section) {
		section = addSection (conf, sectionName);
		if (!section) {
			GfError ("getParamByName: addSection failed\n");
			return NULL;
		}
	}
	param = addParam (conf, section, paramName, "");

	return param;
}

/* Remove a parameter */
static void
removeParamByName (struct parmHeader *conf, char *sectionName, char *paramName)
{
    char		*fullName;
    struct param	*param;
    struct section	*section;

    section = (struct section *)GfHashGetStr (conf->sectionHash, sectionName);
    if (!section) {
	return;
    }

    fullName = getFullName (sectionName, paramName);
    if (!fullName) {
	GfError ("removeParamByName: getFullName failed\n");
	return;
    }

    param = (struct param *)GfHashGetStr (conf->paramHash, fullName);
    freez (fullName);
    if (param) {
	removeParam (conf, section, param);
    }

    cleanUnusedSection (conf, section);
}

/* Clean up unused sections and parents */
static void
cleanUnusedSection (struct parmHeader *conf, struct section *section)
{
    struct section	*parent;

    if (!section->fullName ||
	(!GF_TAILQ_FIRST (&(section->paramList)) &&
	 !GF_TAILQ_FIRST (&(section->subSectionList)))) {
	parent = section->parent;
	removeSection (conf, section);
	if (parent) {
	    /* check if the parent is unused */
	    cleanUnusedSection (conf, parent);
	}
    }
}

/* Remove a parameter */
static void
removeParam (struct parmHeader *conf, struct section *section, struct param *param)
{
	GfHashRemStr (conf->paramHash, param->fullName);
	GF_TAILQ_REMOVE (&(section->paramList), param, linkParam);

	struct within *within;
	while ((within = GF_TAILQ_FIRST (&param->withinList)) != GF_TAILQ_END (&param->withinList)) {
		GF_TAILQ_REMOVE (&param->withinList, within, linkWithin);
		freez(within->val);
		free(within);
	}

	// GIUSE - free for all! trying to limit the memory leaks...
  freeparam(param);
}

/* Add a parameter anywhere, does not check for duplicate. */
static struct param *
addParam (struct parmHeader *conf, struct section *section, const char *paramName, const char *value)
{
	char		*fullName;
	struct param	*param = NULL;
	char		*tmpVal = NULL;

	tmpVal = strdup (value);
	if (!tmpVal) {
		GfError ("addParam: strdup (%s) failed\n", value);
		goto bailout;
	}

	param = (struct param *) calloc (1, sizeof (struct param));
	if (!param) {
		GfError ("addParam: calloc (1, %d) failed\n", sizeof (struct param));
		goto bailout;
	}

	param->name = strdup (paramName);
	if (!param->name) {
		GfError ("addParam: strdup (%s) failed\n", paramName);
		goto bailout;
	}

	fullName = getFullName (section->fullName, paramName);
	if (!fullName) {
		GfError ("addParam: getFullName failed\n");
		goto bailout;
	}

	param->fullName = fullName;
	if (GfHashAddStr (conf->paramHash, param->fullName, param)) {
		goto bailout;
	}

	GF_TAILQ_INIT (&(param->withinList));

	/* Attach to section */
	GF_TAILQ_INSERT_TAIL (&(section->paramList), param, linkParam);

	freez (param->value);
	param->value = tmpVal;

	return param;

 bailout:
	if (param) {
		freez (param->name);
		freez (param->fullName);
		freez (param->value);
		free  (param);
	}
	freez (tmpVal);

	return NULL;
}


/* Remove a section */
static void
removeSection (struct parmHeader *conf, struct section *section)
{
	struct param *param;
	struct section *subSection;

	while ((subSection = GF_TAILQ_FIRST (&(section->subSectionList))) != NULL) {
		removeSection (conf, subSection);
	}

	if (section->fullName) {
		/* not the root section */
		GfHashRemStr (conf->sectionHash, section->fullName);
		GF_TAILQ_REMOVE (&(section->parent->subSectionList), section, linkSection);
		while ((param = GF_TAILQ_FIRST (&(section->paramList))) != GF_TAILQ_END (&(section->paramList))) {
			removeParam (conf, section, param);
		}
		freez (section->fullName);
	}
	freez (section);
}

/* Get or create parent section */
static struct section *
getParent (struct parmHeader *conf, const char *sectionName)
{
	struct section	*section;
	char		*tmpName;
	char		*s;

	tmpName = strdup (sectionName);
	if (!tmpName) {
		GfError ("getParent: strdup (\"%s\") failed\n", sectionName);
		return NULL;
	}

	s = strrchr (tmpName, '/');
	if (s) {
		*s = '\0';
		section = (struct section *)GfHashGetStr (conf->sectionHash, tmpName);
		if (section) {
			goto end;
		}
		section = addSection (conf, tmpName);
		goto end;
	} else {
		section = conf->rootSection;
		goto end;
	}

end:
	free (tmpName);
	return section;
}

/* Add a new section */
static struct section *
addSection (struct parmHeader *conf, const char *sectionName)
{
	struct section	*section;
	struct section	*parent;

	if (GfHashGetStr (conf->sectionHash, sectionName)) {
		GfError ("addSection: duplicate section [%s]\n", sectionName);
		return NULL;
	}

	parent = getParent(conf, sectionName);
	if (!parent) {
		GfError ("addSection: Problem with getParent for section [%s]\n", sectionName);
		return NULL;
	}

	section = (struct section *) calloc (1, sizeof (struct section));
	if (!section) {
		GfError ("addSection: calloc (1, %d) failed\n", sizeof (struct section));
		return NULL;
	}

	section->fullName = strdup(sectionName);
	if (!section->fullName) {
		GfError ("addSection: strdup (%s) failed\n", sectionName);
		goto bailout;
	}

	if (GfHashAddStr (conf->sectionHash, sectionName, section)) {
		GfError ("addSection: GfHashAddStr failed\n");
		goto bailout;
	}

	/* no more bailout call */
	section->parent = parent;
	GF_TAILQ_INIT (&(section->paramList));
	GF_TAILQ_INIT (&(section->subSectionList));
	GF_TAILQ_INSERT_TAIL (&(parent->subSectionList), section, linkSection);

	return section;

bailout:
	freez (section->fullName);
	freez (section);
	return NULL;
}

static struct parmHeader *
getSharedHeader (const char *file, int mode)
{
    struct parmHeader	*conf = NULL;
    struct parmHandle	*parmHandle;

    /* Search for existing conf */
    if ((mode & GFPARM_RMODE_PRIVATE) == 0) {

	for (parmHandle = GF_TAILQ_FIRST (&parmHandleList);
	     parmHandle != GF_TAILQ_END (&parmHandleList);
	     parmHandle = GF_TAILQ_NEXT (parmHandle, linkHandle)) {

	    if ((parmHandle->flag & PARM_HANDLE_FLAG_PRIVATE) == 0) {
		conf = parmHandle->conf;
		if (!strcmp(conf->filename, file)) {
		    if (mode & GFPARM_RMODE_REREAD) {
			parmClean (conf);
		    }
		    conf->refcount++;
		    return conf;
		}
	    }
	}
    }

    return NULL;
}

/* Conf header creation */
static struct parmHeader *
createParmHeader (const char *file)
{
    struct parmHeader	*conf = NULL;

    conf = (struct parmHeader *) calloc (1, sizeof (struct parmHeader));
    if (!conf) {
	GfError ("gfParmReadFile: calloc (1, %d) failed\n", sizeof (struct parmHeader));
	return NULL;
    }

    conf->refcount = 1;

    conf->rootSection = (struct section *) calloc (1, sizeof (struct section));
    if (!conf->rootSection) {
	GfError ("gfParmReadFile: calloc (1, %d) failed\n", sizeof (struct section));
	goto bailout;
    }
    GF_TAILQ_INIT (&(conf->rootSection->paramList));
    GF_TAILQ_INIT (&(conf->rootSection->subSectionList));

    conf->paramHash = GfHashCreate (GF_HASH_TYPE_STR);
    if (!conf->paramHash) {
	GfError ("gfParmReadFile: GfHashCreate (paramHash) failed\n");
	goto bailout;
    }

    conf->sectionHash = GfHashCreate (GF_HASH_TYPE_STR);
    if (!conf->sectionHash) {
	GfError ("gfParmReadFile: GfHashCreate (sectionHash) failed\n");
	goto bailout;
    }

    conf->filename = strdup (file);
    if (!conf->filename) {
	GfError ("gfParmReadFile: strdup (%s) failed\n", file);
	goto bailout;
    }

    return conf;

 bailout:
    freez (conf->rootSection);
    if (conf->paramHash) {
	GfHashRelease (conf->paramHash, NULL);
    }
    if (conf->sectionHash) {
	GfHashRelease (conf->sectionHash, NULL);
    }
    freez (conf->filename);
    freez (conf);

    return NULL;
}

static void
addWithin (struct param *curParam, char *s1)
{
	struct within *curWithin;

	if (!s1 || ! strlen (s1)) {
		return;
	}

	curWithin = (struct within *) calloc (1, sizeof (struct within));
	curWithin->val = strdup (s1);
	GF_TAILQ_INSERT_TAIL (&(curParam->withinList), curWithin, linkWithin);
}


/* XML Processing */
static int
myStrcmp(const void *s1, const void * s2)
{
    return strcmp((const char *)s1, (const char *)s2);
}

static tdble
getValNumFromStr (const char *str)
{
    tdble val;

    if (!str || !strlen (str)) {
	return 0.0;
    }

    if (strncmp (str, "0x", 2) == 0) {
	return (tdble)strtol(str, NULL, 0);
    }

    sscanf (str, "%g", &val);
    return val;
}

/* XML Processing */
static void xmlStartElement (void *userData , const char *name, const char **atts)
{
	struct parmHandle *parmHandle = (struct parmHandle *)userData;
	struct parmHeader *conf = parmHandle->conf;
	struct param *curParam;

	int	nAtts;
	int	len;
	const char **p;
	const char *s1, *s2;
	char *fullName;
	const char *shortName;
	const char *val;
	const char *min;
	const char *max;
	const char *unit;
	char *within;
	char *sa, *sb;

	if (parmHandle->flag & PARM_HANDLE_FLAG_PARSE_ERROR) {
		// parse error occured, ignore.
		return;
	}

	p = atts;
	while (*p) {
		++p;
	}

	nAtts = (p - atts) >> 1;
	if (nAtts > 1) {
		qsort ((void *)atts, nAtts, sizeof(char *) * 2, myStrcmp);
	}

	if (!strcmp(name, "params")) {

		parmHandle->curSection = conf->rootSection;
		parmHandle->curSection->fullName = strdup ("");

		if (!parmHandle->curSection->fullName) {
			GfError ("xmlStartElement: strdup (\"\") failed\n");
			goto bailout;
		}

		while (*atts) {
			s1 = *atts++;
			s2 = *atts++;
			if (!strcmp(s1, "name")) {
				FREEZ (conf->name);
				conf->name = strdup(s2);
				if (!conf->name) {
					GfError ("xmlStartElement: strdup (\"%s\") failed\n", s2);
					goto bailout;
				}
				break;
			}
		}

		if (!conf->name) {
			GfOut ("xmlStartElement: Syntax error, missing \"name\" field in params definition\n");
			goto bailout;
		}

    } else if (!strcmp(name, "section")) {

		if (!parmHandle->curSection) {
			GfError ("xmlStartElement: Syntax error, missing \"params\" tag\n");
			goto bailout;
		}

		shortName = NULL;

		while (*atts) {
			s1 = *atts++;
			s2 = *atts++;
			if (!strcmp(s1, "name")) {
				shortName = s2;
				break;
			}
		}

		if (!shortName) {
			GfError ("xmlStartElement: Syntax error, missing \"name\" field in section definition\n");
			goto bailout;
		}

		if (strlen(parmHandle->curSection->fullName)) {
			len = strlen (shortName) + strlen (parmHandle->curSection->fullName) + 2;
			fullName = (char *) malloc (len);
			if (!fullName) {
				GfError ("xmlStartElement: malloc (%d) failed\n", len);
				goto bailout;
			}
		    sprintf (fullName, "%s/%s", parmHandle->curSection->fullName, shortName);
		} else {
			fullName = strdup (shortName);
		}

		parmHandle->curSection = addSection(conf, fullName);
		free(fullName);

		if (!parmHandle->curSection) {
			GfError ("xmlStartElement: addSection failed\n");
			goto bailout;
		}

	} else if (!strcmp(name, "attnum")) {

		if ((!parmHandle->curSection) || (!strlen (parmHandle->curSection->fullName))) {
	    	GfError ("xmlStartElement: Syntax error, missing \"section\" tag\n");
			goto bailout;
		}

		shortName = NULL;
		val = NULL;
		min = max = unit = NULL;

		while (*atts) {
			s1 = *atts++;
			s2 = *atts++;
			if (!strcmp(s1, "name")) {
				shortName = s2;
			} else if (!strcmp(s1, "val")) {
				val = s2;
			} else if (!strcmp(s1, "min")) {
				min = s2;
			} else if (!strcmp(s1, "max")) {
				max = s2;
			} else if (!strcmp(s1, "unit")) {
				unit = s2;
			}
		}

		if (!shortName) {
			GfError ("xmlStartElement: Syntax error, missing \"name\" field in %s definition\n", name);
			goto bailout;
		}

		if (!val) {
			GfError ("xmlStartElement: Syntax error, missing \"val\" field in %s definition\n", name);
			goto bailout;
		}

		if (!min) {
			min = val;
		}

		if (!max) {
			max = val;
		}

  // GIUSE - this is the first call, no need to free
		curParam = addParam (conf, parmHandle->curSection, shortName, val);
		if (!curParam) {
			GfError ("xmlStartElement: addParam failed\n");
			goto bailout;
		}

		curParam->type = P_NUM;
		curParam->valnum = getValNumFromStr (val);
		curParam->min    = getValNumFromStr (min);
		curParam->max    = getValNumFromStr (max);

		if (curParam->min > curParam->valnum) {
			curParam->min = curParam->valnum;
		}

		if (curParam->max < curParam->valnum) {
			curParam->max = curParam->valnum;
		}

		if (unit) {
			curParam->unit = strdup (unit);
			curParam->valnum = GfParmUnit2SI ((char*)unit, curParam->valnum);
			curParam->min = GfParmUnit2SI ((char*)unit, curParam->min);
			curParam->max = GfParmUnit2SI ((char*)unit, curParam->max);
		}

    } else if (!strcmp(name, "attstr")) {

		if ((!parmHandle->curSection) || (!strlen (parmHandle->curSection->fullName))) {
			GfError ("xmlStartElement: Syntax error, missing \"section\" tag\n");
			goto bailout;
		}

		shortName = NULL;
		val = NULL;
		within = NULL;

		while (*atts) {
			s1 = *atts++;
			s2 = *atts++;
			if (!strcmp(s1, "name")) {
				shortName = s2;
			} else if (!strcmp(s1, "val")) {
				val = s2;
			} else if (!strcmp(s1, "in")) {
				within = (char *)s2;
			}
		}

		if (!shortName) {
			GfError ("xmlStartElement: Syntax error, missing \"name\" field in %s definition\n", name);
			goto bailout;
		}

		if (!val) {
			GfError ("xmlStartElement: Syntax error, missing \"val\" field in %s definition\n", name);
			goto bailout;
		}

		// GIUSE - free for all! trying to limit the memory leaks...
    // this is the second call
		freeparam(curParam);

		curParam = addParam (conf, parmHandle->curSection, shortName, val);
		if (!curParam) {
			GfError ("xmlStartElement: addParam failed\n");
			goto bailout;
		}

		curParam->type = P_STR;
		if (within) {
			sa = within;
			sb = strchr (sa, ',');
			while (sb) {
				*sb = 0;
				addWithin (curParam, sa);
				sa = sb + 1;
				sb = strchr (sa, ',');
			}
			addWithin (curParam, sa);
		}

    }

    return;

 bailout:
    parmHandle->flag |= PARM_HANDLE_FLAG_PARSE_ERROR;
    return;
}

static void
xmlEndElement (void *userData, const XML_Char *name)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)userData;

    if (parmHandle->flag & PARM_HANDLE_FLAG_PARSE_ERROR) {
	/* parse error occured, ignore */
	return;
    }

    if (!strcmp(name, "section")) {
	if ((!parmHandle->curSection) || (!parmHandle->curSection->parent)) {
	    GfError ("xmlEndElement: Syntax error in \"%s\"\n", name);
	    return;
	}
	parmHandle->curSection = parmHandle->curSection->parent;
    }
}

static int
xmlExternalEntityRefHandler (XML_Parser mainparser,
			     const XML_Char *openEntityNames,
			     const XML_Char * /* base */,
			     const XML_Char *systemId,
			     const XML_Char * /* publicId */)
{
    FILE 		*in;
    char		buf[BUFSIZ];
    XML_Parser 		parser;
    int			done;
    char		fin[LINE_SZ];
    char		*s;
    struct parmHandle	*parmHandle;
    struct parmHeader	*conf;

    parmHandle = (struct parmHandle *)XML_GetUserData (mainparser);
    conf = parmHandle->conf;

    parser = XML_ExternalEntityParserCreate (mainparser,
					     openEntityNames,
					     (const XML_Char *)NULL);

    if (systemId[0] == '/') {
	strncpy (fin, systemId, sizeof (fin));
	fin[LINE_SZ - 1] = 0;
    } else {
	/* relative path */
	strncpy (fin, conf->filename, sizeof (fin));
	fin[LINE_SZ - 1] = 0;
	s = strrchr (fin, '/');
	if (s) {
	    s++;
	} else {
	    s = fin;
	}
	strncpy (s, systemId, sizeof (fin) - (s - fin));
	fin[LINE_SZ - 1] = 0;
    }

	in = fopen (fin, "r");
    if (in == NULL) {
		perror (fin);
		GfError ("GfReadParmFile: file %s has pb\n", systemId);
		return 0;
    }

	XML_SetElementHandler (parser, xmlStartElement, xmlEndElement);
	do {
		size_t len = fread (buf, 1, sizeof(buf), in);
		done = len < sizeof (buf);
		if (!XML_Parse (parser, buf, len, done)) {
			GfError ("file: %s -> %s at line %d\n",
				systemId,
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser));
	    	fclose (in);
			return 0;
		}
	} while (!done);
	XML_ParserFree (parser);
	fclose(in);

	return 1; /* ok (0 for failure) */
}


/* xml type parameters line parser */
static int
parseXml (struct parmHandle *parmHandle, char *buf, int len, int done)
{
	if (!XML_Parse(parmHandle->parser, buf, len, done)) {
		GfError ("parseXml: %s at line %d\n",
			(char*)XML_ErrorString (XML_GetErrorCode (parmHandle->parser)),
			XML_GetCurrentLineNumber (parmHandle->parser));
		return 1;
	}

	if (done) {
		XML_ParserFree(parmHandle->parser);
		parmHandle->parser = 0;
	}

	return 0;
}


static int
parserXmlInit (struct parmHandle *parmHandle)
{
    parmHandle->parser = XML_ParserCreate((XML_Char*)NULL);
    XML_SetElementHandler(parmHandle->parser, xmlStartElement, xmlEndElement);
    XML_SetExternalEntityRefHandler(parmHandle->parser, xmlExternalEntityRefHandler);
    XML_SetUserData(parmHandle->parser, parmHandle);

    return 0;
}

/** Read a configuration buffer.
    @ingroup	conf
    @param	logHandle	log handle
    @param	buf		input buffer.
    @return	handle on the configuration data
    <br>0 if Error
*/
void *
GfParmReadBuf (char *buffer)
{
    struct parmHeader	*conf;
    struct parmHandle	*parmHandle = NULL;

    /* Conf Header creation */
    conf = createParmHeader ("");
    if (!conf) {
	GfError ("gfParmReadBuf: conf header creation failed\n");
	goto bailout;
    }

    /* Handle creation */
    parmHandle = (struct parmHandle *) calloc (1, sizeof (struct parmHandle));
    if (!parmHandle) {
	GfError ("gfParmReadBuf: calloc (1, %d) failed\n", sizeof (struct parmHandle));
	goto bailout;
    }

    parmHandle->magic = PARM_MAGIC;
    parmHandle->conf = conf;
    parmHandle->val = NULL;
    parmHandle->flag = PARM_HANDLE_FLAG_PRIVATE;

    /* Parsers Initialization */
    if (parserXmlInit (parmHandle)) {
	GfError ("gfParmReadBuf: parserInit failed\n");
	goto bailout;
    }

    /* Parameters reading in buffer */
    if (parseXml (parmHandle, buffer, strlen (buffer), 1)) {
	GfError ("gfParmReadBuf: Parse failed for buffer\n");
	goto bailout;
    }

    GF_TAILQ_INSERT_HEAD (&parmHandleList, parmHandle, linkHandle);

    return parmHandle;

 bailout:
    freez (parmHandle);
    if (conf) {
	parmReleaseHeader (conf);
    }

    return NULL;
}

/** Read a configuration file.
    @note	Called by #gfInitThread
    @ingroup	conf
    @param	logHandle	log handle
    @param	file		name of the file to read or content if input is a buffer.
    @param	mode		openning mode is a mask of:
    				#GF_PARM_RMODE_STD
    				#GF_PARM_RMODE_REREAD
    				#GF_PARM_RMODE_CREAT
    				#GF_PARM_RMODE_PRIVATE
    @return	handle on the configuration data
    <br>0 if Error
*/
void *
GfParmReadFile (const char *file, int mode)
{
    FILE		*in = NULL;
    struct parmHeader	*conf;
    struct parmHandle	*parmHandle = NULL;
    char		buf[LINE_SZ];
    int			len;
    int			done;

    /* search for an already openned header & clean the conf if necessary */
    conf = getSharedHeader (file, mode);

    /* Conf Header creation */
    if (conf == NULL) {
	conf = createParmHeader (file);
	if (!conf) {
	    GfError ("gfParmReadFile: conf header creation failed\n");
	    goto bailout;
	}
	mode |= GFPARM_RMODE_REREAD;
    }

    /* Handle creation */
    parmHandle = (struct parmHandle *) calloc (1, sizeof (struct parmHandle));
    if (!parmHandle) {
	GfError ("gfParmReadFile: calloc (1, %d) failed\n", sizeof (struct parmHandle));
	goto bailout;
    }

    parmHandle->magic = PARM_MAGIC;
    parmHandle->conf = conf;
    parmHandle->val = NULL;
    if (mode & GFPARM_RMODE_PRIVATE) {
	parmHandle->flag = PARM_HANDLE_FLAG_PRIVATE;
    }

    /* File openning */
    if (mode & GFPARM_RMODE_REREAD) {
	in = fopen (file, "r");
	if (!in && ((mode & GFPARM_RMODE_CREAT) == 0)) {
	    GfOut ("gfParmReadFile: fopen \"%s\" failed\n", file);
	    goto bailout;
	}

	if (in) {
	    /* Parsers Initialization */
	    if (parserXmlInit (parmHandle)) {
		GfError ("gfParmReadBuf: parserInit failed for file \"%s\"\n", file);
		goto bailout;
	    }
	    /* Parameters reading */
	    do {
		len = fread (buf, 1, sizeof(buf), in);
		done = len < (int)sizeof(buf);
		if (parseXml (parmHandle, buf, len, done)) {
		    GfError ("gfParmReadFile: Parse failed in file \"%s\"\n", file);
		    goto bailout;
		}
		if (parmHandle->flag & PARM_HANDLE_FLAG_PARSE_ERROR) {
		    /* parse error occured, ignore */
		    GfError ("gfParmReadFile: Parse failed in file \"%s\"\n", file);
		    goto bailout;
		}
	    } while (!done);

	    fclose (in);
	    in = NULL;
	}
    }

    GF_TAILQ_INSERT_HEAD (&parmHandleList, parmHandle, linkHandle);

    GfOut ("GfParmReadFile: Openning \"%s\" (%p)\n", file, parmHandle);

    return parmHandle;

 bailout:
    if (in) {
	fclose (in);
    }
    freez (parmHandle);
    if (conf) {
	parmReleaseHeader (conf);
    }

    return NULL;
}



static char* handleEntities(char *s, const char* val)
{
	int i = 0;
	int len = strlen(val);
	for (i = 0; i < len; i++) {
		switch (val[i]) {
			case '<':
				s += sprintf(s, "&lt;"); break;
			case '>':
				s += sprintf(s, "&gt;"); break;
			case '&':
				s += sprintf(s, "&amp;"); break;
			case '\'':
				s += sprintf(s, "&apos;"); break;
			case '"':
				s += sprintf(s, "&quot;"); break;
			default:
				*(s++) = val[i];
				break;
		}
	}

	return s;
}


/**
 *	@image html output-state.png
 */
static int
xmlGetOuputLine (struct parmHandle *parmHandle, char *buffer, int /* size */)
{
    struct parmOutput	*outCtrl = &(parmHandle->outCtrl);
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*curSection;
    struct param	*curParam;
    struct within	*curWithin;
    char		*s, *t;

    while (1) {
	switch (outCtrl->state) {
	case 0:
	    sprintf (buffer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	    outCtrl->indent = (char *) malloc (LINE_SZ);
	    if (!outCtrl->indent) {
		GfError ("xmlGetOuputLine: malloc (%d) failed\n", LINE_SZ);
		return 0;
	    }
	    outCtrl->state = 1;
	    return 1;

	case 1:
	    if (conf->dtd == NULL) {
		conf->dtd = strdup("params.dtd");
	    }
	    if (conf->header == NULL) {
		conf->header = strdup("");
	    }
	    sprintf (buffer, "<!DOCTYPE params SYSTEM \"%s\">\n%s\n", conf->dtd, conf->header);
	    *outCtrl->indent = 0;
	    outCtrl->state = 2;
	    return 1;

	case 2:			/* Start Params */
	    outCtrl->curSection = parmHandle->conf->rootSection;
	    sprintf (buffer, "\n<params name=\"%s\">\n", parmHandle->conf->name);
	    curSection = GF_TAILQ_FIRST (&(outCtrl->curSection->subSectionList));
	    if (curSection) {
		outCtrl->curSection = curSection;
		sprintf (outCtrl->indent + strlen (outCtrl->indent), "  ");
		outCtrl->state = 4;
	    } else {
		outCtrl->state = 3;
	    }
	    return 1;

	case 3:			/* End Params */
	    sprintf (buffer, "</params>\n");
	    free (outCtrl->indent);
	    outCtrl->state = 9;
	    return 1;

	case 4:			/* Parse section attributes list */
		outCtrl->curParam = GF_TAILQ_FIRST (&(outCtrl->curSection->paramList));
		s = strrchr (outCtrl->curSection->fullName, '/');
		if (!s) {
			s = outCtrl->curSection->fullName;
		} else {
			s++;
		}

		t = buffer;
		t += sprintf(t, "%s<section name=\"", outCtrl->indent);
		t = handleEntities(t, s);
		t += sprintf (t, "\">\n");

		sprintf (outCtrl->indent + strlen (outCtrl->indent), "  ");
		outCtrl->state = 5;
		return 1;

	case 5:			/* Parse one attribute */
	    if (!outCtrl->curParam) {
			outCtrl->state = 6;
			break;
		}

		curParam = outCtrl->curParam;
		if (curParam->type == P_STR) {
			s = buffer;
			s += sprintf (s, "%s<attstr name=\"%s\"", outCtrl->indent, curParam->name);
			curWithin = GF_TAILQ_FIRST (&(curParam->withinList));
			if (curWithin) {
				s += sprintf (s, " in=\"%s", curWithin->val);
				while ((curWithin = GF_TAILQ_NEXT (curWithin, linkWithin)) != NULL) {
					s += sprintf (s, ",%s", curWithin->val);
				}
				s += sprintf (s, "\"");
			}

			s += sprintf(s, " val=\"");
			s = handleEntities(s, curParam->value);
			s += sprintf (s, "\"/>\n");

			outCtrl->curParam = GF_TAILQ_NEXT (curParam, linkParam);
			return 1;
	    } else {

			s = buffer;
			s += sprintf (s, "%s<attnum name=\"%s\"", outCtrl->indent, curParam->name);
			if (curParam->unit) {
				if ((curParam->min != curParam->valnum) || (curParam->max != curParam->valnum)) {
					s += sprintf (s, " min=\"%g\" max=\"%g\"",
						GfParmSI2Unit (curParam->unit, curParam->min),
						GfParmSI2Unit (curParam->unit, curParam->max));
				}
				s += sprintf (s, " unit=\"%s\" val=\"%g\"/>\n",
					curParam->unit, GfParmSI2Unit (curParam->unit, curParam->valnum));
			} else {
				if ((curParam->min != curParam->valnum) || (curParam->max != curParam->valnum)) {
					s += sprintf (s, " min=\"%g\" max=\"%g\"",
						curParam->min, curParam->max);
				}
				s += sprintf (s, " val=\"%g\"/>\n", curParam->valnum);
			}

			outCtrl->curParam = GF_TAILQ_NEXT (curParam, linkParam);
			return 1;
	    }

	case 6:			/* Parse sub-section list */
	    curSection = GF_TAILQ_FIRST (&(outCtrl->curSection->subSectionList));
	    if (curSection) {
		outCtrl->curSection = curSection;
		outCtrl->state = 4;
		break;
	    }
	    outCtrl->state = 7;
	    break;

	case 7:			/* End Section */
	    *(outCtrl->indent + strlen (outCtrl->indent) - 2) = 0;
	    sprintf (buffer, "%s</section>\n\n", outCtrl->indent);
	    outCtrl->state = 8;
	    return 1;

	case 8:			/* Parse next section at the same level */
	    curSection = GF_TAILQ_NEXT (outCtrl->curSection, linkSection);
	    if (curSection) {
		outCtrl->curSection = curSection;
		outCtrl->state = 4;
		break;
	    }
	    curSection = outCtrl->curSection->parent;
	    *(outCtrl->indent + strlen (outCtrl->indent) - 2) = 0;
	    if (curSection->parent) {
		outCtrl->curSection = curSection;
		sprintf (buffer, "%s</section>\n\n", outCtrl->indent);
		return 1;
	    }
	    outCtrl->state = 3;
	    break;
	case 9:
	    return 0;
	}
    }
}

/** Write a configuration buffer.
    @ingroup	conf
    @param	logHandle	log handle
    @param	parmHandle	Configuration handle
    @param	buf		buffer to write the configuration
    @param	size		buffer size
    @return	0 if OK
    		<br>1 if Error
*/
int
GfParmWriteBuf (void *handle, char *buf, int size)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf;
    char		line[LINE_SZ];
    int			len;
    int			curSize;
    char		*s;

    conf = parmHandle->conf;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("gfParmWriteBuf: bad handle (%p)\n", parmHandle);
	return 1;
    }

    parmHandle->outCtrl.state = 0;
    parmHandle->outCtrl.curSection = NULL;
    parmHandle->outCtrl.curParam = NULL;
    curSize = size;
    s = buf;

    while (curSize && xmlGetOuputLine (parmHandle, line, sizeof (line))) {
	len = strlen (line);
	if (len > curSize) {
	    len = curSize;
	}
	memcpy (s, line, len);
	s += len;
	curSize -= len;
    }
    buf [size - 1] = 0;

    return 0;
}

/** Set the dtd path and header if necessary
    @ingroup	conf
    @param	parmHandle	Configuration handle
    @param	dtd		Optional dtd path
    @param	header		Optional header
    @return	none
*/
void
GfParmSetDTD (void *parmHandle, char *dtd, char*header)
{
    struct parmHandle	*handle = (struct parmHandle *)parmHandle;
    struct parmHeader	*conf = handle->conf;

    if (dtd) {
	FREEZ(conf->dtd);
	conf->dtd = strdup(dtd);
    }

    if (header) {
	FREEZ(conf->header);
	conf->header = strdup(header);
    }
}


/** Write a configuration file.
    @ingroup	conf
    @param	parmHandle	Configuration handle
    @param	file		Name of the file to write (NULL if previously read file)
    @param	name	Name of the parameters
    @return	0 if OK
    <br>1 if Error
*/
int
GfParmWriteFile (const char *file, void *parmHandle, char *name)
{
    struct parmHandle	*handle = (struct parmHandle *)parmHandle;
    struct parmHeader	*conf = handle->conf;
    char		line[LINE_SZ];
    FILE		*fout;

    conf = handle->conf;

    if (handle->magic != PARM_MAGIC) {
	GfFatal ("gfParmWriteFile: bad handle (%p)\n", parmHandle);
	return 1;
    }

    if (!file) {
	file = conf->filename;
	if (!file) {
	    GfError ("gfParmWriteFile: bad file name\n");
	    return 1;
	}
    }
    fout = fopen (file, "wb");
    if (!fout) {
	GfError ("gfParmSetStr: fopen (%s, \"wb\") failed\n", file);
	return 1;
    }

    if (name) {
	FREEZ (conf->name);
	conf->name = strdup (name);
    }

    handle->outCtrl.state = 0;
    handle->outCtrl.curSection = NULL;
    handle->outCtrl.curParam = NULL;

    while (xmlGetOuputLine (handle, line, sizeof (line))) {
	fputs (line, fout);
    }

    GfOut ("GfParmWriteFile: %s file written\n", file);

    fclose (fout);

    return 0;
}

/** Remove a parameter.
    @ingroup	conf
    @param	parmHandle	Configuration handle
    @param	sectionName	Parameter section name
    @param	paramName	Parameter name
    @return	none
*/
void
GfParmRemove (void *parmHandle, char *sectionName, char *paramName)
{
    struct parmHandle	*handle = (struct parmHandle *)parmHandle;
    struct parmHeader	*conf;

    conf = handle->conf;

    if (handle->magic != PARM_MAGIC) {
	GfFatal ("gfParmRemove: bad handle (%p)\n", parmHandle);
	return;
    }

    removeParamByName (conf, sectionName, paramName);
}


static void parmClean (struct parmHeader *conf)
{
	struct section	*section;

	while ((section = GF_TAILQ_FIRST (&(conf->rootSection->subSectionList))) !=
		    GF_TAILQ_END (&(conf->rootSection->subSectionList)))
	{
		removeSection (conf, section);
	}
}


/** Clean all the parameters of a set.
    @ingroup	conf
    @param	parmHandle	Configuration handle
    @return	0 if OK
    		<br>-1 if Error
*/
void
GfParmClean (void *parmHandle)
{
    struct parmHandle	*handle = (struct parmHandle *)parmHandle;
    struct parmHeader	*conf;

    conf = handle->conf;

    if (handle->magic != PARM_MAGIC) {
	GfFatal ("gfParmClean: bad handle (%p)\n", parmHandle);
	return;
    }

    parmClean (conf);
}


static void parmReleaseHeader(struct parmHeader *conf)
{
	conf->refcount--;
	if (conf->refcount > 0) {
		return;
	}

	GfOut ("parmReleaseHeader: refcount null free \"%s\"\n", conf->filename);

	parmClean (conf);

	freez (conf->filename);
	if (conf->paramHash) {
		GfHashRelease (conf->paramHash, NULL);
	}

	if (conf->sectionHash) {
		GfHashRelease (conf->sectionHash, NULL);
	}

	freez (conf->rootSection->fullName);
	freez (conf->rootSection);
	freez (conf->dtd);
	freez (conf->name);
	freez (conf->header);
	freez (conf);
}


static void parmReleaseHandle (struct parmHandle *parmHandle)
{
	struct parmHeader *conf = parmHandle->conf;

	GfOut ("parmReleaseHandle: release \"%s\" (%p)\n", conf->filename, parmHandle);

	GF_TAILQ_REMOVE (&parmHandleList, parmHandle, linkHandle);
	parmHandle->magic = 0;
	freez (parmHandle->val);
	freez (parmHandle);

	parmReleaseHeader(conf);
}


/** Clean the parms and release the handle without updating the file
    @note	Called by #gfShutdownThread
    @ingroup	conf
    @param	logHandle	log handle
    @param	parmHandle	Configuration handle
    @return	none
*/
void GfParmReleaseHandle (void *parmHandle)
{
	struct parmHandle *handle = (struct parmHandle *)parmHandle;

	if (handle->magic != PARM_MAGIC) {
		GfFatal ("gfParmReleaseHandle: bad handle (%p)\n", parmHandle);
		return;
	}

	parmReleaseHandle(handle);
}


static void
evalUnit (char *unit, tdble *dest, int flg)
{
    tdble coeff = 1.0;

    if (strcmp(unit, "m") == 0) return;
    if (strcmp(unit, "kg") == 0) return;
    if (strcmp(unit, "s") == 0) return;
    if (strcmp(unit, "rad") == 0) return;
    if (strcmp(unit, "Pa") == 0) return;

    if ((strcmp(unit, "feet") == 0) || (strcmp(unit, "ft") == 0)) {
	coeff = 0.304801f; /* m */
    } else if (strcmp(unit, "deg") == 0) {
	coeff = (float) (M_PI/180.0); /* rad */
    } else if ((strcmp(unit, "h") == 0) || (strcmp(unit, "hour") == 0) || (strcmp(unit, "hours") == 0)) {
	coeff = 3600.0; /* s */
    } else if ((strcmp(unit, "day") == 0) || (strcmp(unit, "days") == 0)) {
	coeff = 24*3600.0; /* s */
    } else if (strcmp(unit, "km") == 0) {
	coeff = 1000.0; /* m */
    } else if (strcmp(unit, "mm") == 0) {
	coeff = 0.001f; /* m */
    } else if (strcmp(unit, "cm") == 0) {
	coeff = 0.01f; /* m */
    } else if ((strcmp(unit, "in") == 0) || (strcmp(unit, "inch") == 0) || (strcmp(unit, "inches") == 0)) {
	coeff = 0.0254f; /* m */
    } else if ((strcmp(unit, "lbs") == 0)  || (strcmp(unit, "lb") == 0)) {
	coeff = 0.45359237f; /* kg */
    } else if ((strcmp(unit, "slug") == 0) || (strcmp(unit, "slugs") == 0)) {
	coeff = 14.59484546f; /* kg */
    } else if (strcmp(unit, "kPa") == 0) {
	coeff = 1000.0; /* Pa */
    } else if (strcmp(unit, "MPa") == 0) {
	coeff = 1000000.0; /* Pa */
    } else if ((strcmp(unit, "PSI") == 0) || (strcmp(unit, "psi") == 0)){
	coeff = 6894.76f; /* Pa */
    } else if ((strcmp(unit, "rpm") == 0) || (strcmp(unit, "RPM") == 0)) {
	coeff = 0.104719755f; /* rad/s */
    } else if ((strcmp(unit, "percent") == 0) || (strcmp(unit, "%") == 0)) {
	coeff = 0.01f;
    } else if ((strcmp(unit, "mph") == 0) || (strcmp(unit, "MPH") == 0)) {
	coeff = 0.44704f; /* m/s */
    }

    if (flg) {
	*dest /= coeff;
    } else {
	*dest *= coeff;
    }

    return;
}

/** Convert a value in "units" into SI.
    @ingroup	paramsdata
    @param	unit	unit name
    @param	val	value in units
    @return	the value in corresponding SI unit
    @warning	The supported units are:
    			<br><ul><li><b>feet</b> or <b>ft</b>  converted to <b>m</b></li>
			<li><b>inches</b> or <b>in</b> converted to <b>m</b></li>
			<li><b>lbs</b> converted to <b>kg</b></li>
			<li><b>slug</b> or <b>slugs</b> converted to <b>kg</b></li>
			<li><b>h</b> or <b>hours</b> converted to <b>s</b></li>
			<li><b>day</b> or <b>days</b> converted to <b>s</b></li>
			<li><b>km</b> converted to <b>m</b></li>
			<li><b>cm</b> converted to <b>m</b></li>
			<li><b>mm</b> converted to <b>m</b></li>
			<li><b>kPa</b> converted to <b>Pa</b></li>
			<li><b>deg</b> converted to <b>rad</b></li>
			<li><b>rpm</b> or <b>RPM</b> converted to <b>rad/s</b></li>
			<li><b>percent</b> or <b>%</b> divided by <b>100</b></li></ul>
    @see	GfParmSI2Unit
 */
tdble
GfParmUnit2SI (const char *unit, tdble val)
{
	char buf[256];
	int  idx;
	const char *s;
	int  inv;
	tdble dest = val;

	if ((unit == NULL) || (strlen(unit) == 0)) return dest;

	s = unit;
	buf[0] = 0;
	inv = 0;
	idx = 0;

	while (*s != 0) {
		switch (*s) {
			case '.':
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				break;
			case '/':
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				inv = 1;
				break;
			case '2':
				evalUnit(buf, &dest, inv);
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				break;
			default:
				buf[idx++] = *s;
				buf[idx] = 0;
				break;
		}
		s++;
	}

	evalUnit(buf, &dest, inv);
	return dest;
}

/** Convert a value in SI to "units".
    @ingroup	paramsdata
    @param	unit	unit name to convert to
    @param	val	value in SI units to be converted to units
    @return	converted value to units
    @see	GfParmUnit2SI
 */
tdble
GfParmSI2Unit (const char *unit, tdble val)
{
	char buf[256];
	int  idx;
	const char *s;
	int  inv;
	tdble dest = val;

	if ((unit == NULL) || (strlen(unit) == 0)) return dest;

	s = unit;
	buf[0] = 0;
	inv = 1;
	idx = 0;

	while (*s != 0) {
		switch (*s) {
			case '.':
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				break;
			case '/':
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				inv = 0;
				break;
			case '2':
				evalUnit(buf, &dest, inv);
				evalUnit(buf, &dest, inv);
				buf[0] = 0;
				idx = 0;
				break;
			default:
				buf[idx++] = *s;
				buf[idx] = 0;
				break;
		}
		s++;
	}

	evalUnit(buf, &dest, inv);
	return dest;
}



/** Get the pararmeters name
    @ingroup	paramsdata
    @param	handle	Handle on the parameters
    @return	Name
*/
char *
GfParmGetName (void *handle)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetName: bad handle (%p)\n", parmHandle);
	return NULL;
    }

    return conf->name;
}


/** Get the pararmeters file name
    @ingroup	paramsfile
    @param	handle	Handle on the parameters
    @return	File Name
*/
char *
GfParmGetFileName (void *handle)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetFileName: bad handle (%p)\n", parmHandle);
	return NULL;
    }

    return conf->filename;
}


/** Count the number of section elements of a list.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of list
    @return	element count
 */
int
GfParmGetEltNb (void *handle, char *path)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;
    int			count;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetEltNb: bad handle (%p)\n", parmHandle);
	return 0;
    }

    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if (!section) {
	return 0;
    }

    count = 0;
    section = GF_TAILQ_FIRST (&(section->subSectionList));
    while (section) {
	count++;
	section = GF_TAILQ_NEXT (section, linkSection);
    }

    return count;
}



/** Seek the first section element of a list.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	list path
    @return	0 Ok
    		<br>-1 Failed
    @see	GfParmListSeekNext
    @see	GfParmListGetCurEltName
 */
int
GfParmListSeekFirst (void *handle, char *path)
{

    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmListSeekFirst: bad handle (%p)\n", parmHandle);
	return -1;
    }

    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if (!section) {
	return -1;
    }

    section->curSubSection = GF_TAILQ_FIRST (&(section->subSectionList));
    return 0;
}

/** Go to the next section element in the current list.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of list
    @return	0 Ok
    		<br>1 End of list reached
    		<br>-1 Failed
    @see	GfParmListSeekFirst
    @see	GfParmListGetCurEltName
 */
int
GfParmListSeekNext (void *handle, char *path)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmListSeekNext: bad handle (%p)\n", parmHandle);
	return -1;
    }

    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if ((!section) || (!section->curSubSection)) {
	return -1;
    }

    section->curSubSection = GF_TAILQ_NEXT (section->curSubSection, linkSection);

    if (section->curSubSection) {
	return 0;
    }
    return 1;			/* EOL reached */
}


/** Remove all the section elements of a list.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of list
    @return	0 Ok
		<br>-1 Error
 */
int
GfParmListClean (void *handle, char *path)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*listSection;
    struct section	*section;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmListSeekNext: bad handle (%p)\n", parmHandle);
	return -1;
    }
    listSection = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if (!listSection) {
	GfOut ("GfParmListClean: \"%s\" not found\n", path);
	return -1;
    }
    while ((section = GF_TAILQ_FIRST (&(listSection->subSectionList))) != NULL) {
	removeSection (conf, section);
    }
    return 0;
}


/** Get the current element name.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of list
    @return	Name of the current element in the list
		<br>NULL if failed
    @see	GfParmListSeekFirst
    @see	GfParmListSeekNext
    @note	String MUST be released by called.
 */
char *
GfParmListGetCurEltName (void *handle, char *path)
{
	struct parmHandle *parmHandle = (struct parmHandle *)handle;
	struct parmHeader *conf = parmHandle->conf;
	struct section *section;
	char *s;

	if (parmHandle->magic != PARM_MAGIC) {
		GfFatal ("GfParmListGetCurEltName: bad handle (%p)\n", parmHandle);
		return NULL;
	}

	section = (struct section *)GfHashGetStr (conf->sectionHash, path);
	if ((!section) || (!section->curSubSection)) {
		return NULL;
	}

	//printf("WARNING: EVENTUALLY STRDUP ON USER SIDE REQUIRED!");

	s = strrchr (section->curSubSection->fullName, '/');
	if (s) {
		s++;
		return s;
		//return strdup (s);
	}

	return section->curSubSection->fullName;
	//return strdup (section->curSubSection->fullName);
}


/** Get string parameter value.
    @ingroup	paramsdata
    @param	parmHandle	Configuration handle
    @param	path		Parameter section name
    @param	key		Parameter name
    @param	deflt		Default value if parameter not existing
    @return	Parameter value
    <br>deflt if Error or not found
    @note	The pointer returned is for immediate use, if you plan
    		to keep the value for a long time, it is necessary to
    		copy it elsewhere, because removing the attribute will
    		produce incoherent pointer.
*/
char *
GfParmGetStr (void *parmHandle, const char *path, const char *key, char *deflt)
{
	struct param *param;
	struct parmHandle *handle = (struct parmHandle *)parmHandle;
	struct parmHeader *conf;
	char *val;

	conf = handle->conf;

	if (handle->magic != PARM_MAGIC) {
		GfFatal ("gfParmGetStr: bad handle (%p)\n", parmHandle);
		return deflt;
	}

	param = getParamByName (conf, path, key, 0);
	if (!param || !(param->value) || !strlen (param->value) || (param->type != P_STR)) {
		return deflt;
	}

// GIUSE - in order to free param, we need to duplicate the string value
	val = param->value;
//	val = strdup(param->value);
//	freeparam(param);
// GIUSE - core dumped - FIX THIS, there still is the leak!
// this stuff ought to be tracked somewhere and deallocated at the end of the run (restart)

	return val;
}

/** Get a string parameter in a config file.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	deflt	default string
    @return	parameter value
    @warning	the return value is allocated by the function the caller must free it.
    @see	GfParmListSeekNext
*/
char *
GfParmGetCurStr (void *handle, char *path, char *key, char *deflt)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetCurStr: bad handle (%p)\n", parmHandle);
	return deflt;
    }
    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if ((!section) || (!section->curSubSection)) {
	return deflt;
    }

    param = getParamByName (conf, section->curSubSection->fullName, key, 0);
    if (!param || !(param->value) || !strlen (param->value) || (param->type != P_STR)) {
	return deflt;
    }

    return param->value;
}

/** Get a numerical parameter in a config file.
    @ingroup	paramsdata
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	unit	unit to convert the result to (NULL if SI wanted)
    @param	deflt	default string
    @return	parameter value
 */
tdble
GfParmGetNum (void *handle, const char *path, const char *key, const char *unit, tdble deflt)
{
	struct parmHandle	*parmHandle = (struct parmHandle *)handle;
	struct parmHeader	*conf = parmHandle->conf;
	struct param	*param;

	if (parmHandle->magic != PARM_MAGIC) {
		GfFatal ("GfParmGetNum: bad handle (%p)\n", parmHandle);
		return deflt;
	}

	param = getParamByName (conf, path, key, 0);
	if (!param ||  (param->type != P_NUM)) {
		return deflt;
	}

	if (unit) {
		return GfParmSI2Unit(unit, param->valnum);
	}
	return  param->valnum;
}



/** Get a numerical parameter in a config file.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	unit	unit to convert the result to (NULL if SI wanted)
    @param	deflt	default string
    @return	parameter value
 */
tdble
GfParmGetCurNum (void *handle, char *path, char *key, char *unit, tdble deflt)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetCurNum: bad handle (%p)\n", parmHandle);
	return deflt;
    }
    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if ((!section) || (!section->curSubSection)) {
	return deflt;
    }

    param = getParamByName (conf, section->curSubSection->fullName, key, 0);
    if (!param || (param->type != P_NUM)) {
	return deflt;
    }

    if (unit) {
	return GfParmSI2Unit(unit, param->valnum);
    }
    return  param->valnum;
}


/** Set a string parameter in a config file.
    @ingroup	paramsdata
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	val	value (NULL or empty string to remove the parameter)
    @return	0	ok
    		<br>-1	error
    @warning	The key is created is necessary
 */
int
GfParmSetStr(void *handle, char *path, char *key, char *val)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmSetStr: bad handle (%p)\n", parmHandle);
	return -1;
    }

    if (!val || !strlen (val)) {
	/* Remove the entry */
	removeParamByName (conf, path, key);
	return 0;
    }

    param = getParamByName (conf, path, key, PARAM_CREATE);
    if (!param) {
	return -1;
    }
    param->type = P_STR;
    freez (param->value);
    param->value = strdup (val);
    if (!param->value) {
	GfError ("gfParmSetStr: strdup (%s) failed\n", val);
	removeParamByName (conf, path, key);
	return -1;
    }

    return 0;
}

/** Set a string parameter in a config file.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	val	value
    @return	0	ok
    		<br>-1	error
    @warning	The key is created is necessary
 */
int
GfParmSetCurStr(void *handle, char *path, char *key, char *val)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmSetCurStr: bad handle (%p)\n", parmHandle);
	return -1;
    }
    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if ((!section) || (!section->curSubSection)) {
	return -1;
    }

    param = getParamByName (conf, section->curSubSection->fullName, key, PARAM_CREATE);
    if (!param) {
	return -1;
    }
    param->type = P_STR;
    freez (param->value);
    param->value = strdup (val);
    if (!param->value) {
	GfError ("gfParmSetStr: strdup (%s) failed\n", val);
	removeParamByName (conf, path, key);
	return -1;
    }

    return 0;
}


/** Set a numerical parameter in a config file.
    @ingroup	paramsdata
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	unit	unit to convert the result to (NULL if SI wanted)
    @param	val	value to set
    @return	0	ok
    		<br>-1	error
    @warning	The key is created is necessary
 */
int
GfParmSetNum(void *handle, const char *path, const char *key, const char *unit, tdble val)
{
	struct parmHandle	*parmHandle = (struct parmHandle *)handle;
	struct parmHeader	*conf = parmHandle->conf;
	struct param	*param;

	if (parmHandle->magic != PARM_MAGIC) {
		GfFatal ("GfParmSetNum: bad handle (%p)\n", parmHandle);
		return -1;
	}

	param = getParamByName (conf, path, key, PARAM_CREATE);
	if (!param) {
		return -11;
	}

	param->type = P_NUM;
	FREEZ (param->unit);
	if (unit) {
		param->unit = strdup (unit);
	}

	val = GfParmUnit2SI (unit, val);
	param->valnum = val;
	param->min = val;
	param->max = val;

	return 0;
}

/** Set a numerical parameter in a config file.
    @ingroup	paramsdata
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	unit	unit to convert the result to (NULL if SI wanted)
    @param	val	value to set
    @param	min	min value
    @param	max	max value
    @return	0	ok
    		<br>-1	error
    @warning	The key is created is necessary
 */
int
GfParmSetNumEx(void *handle, char *path, char *key, char *unit, tdble val, tdble min, tdble max)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmSetNumEx: bad handle (%p)\n", parmHandle);
	return -1;
    }

    param = getParamByName (conf, path, key, PARAM_CREATE);
    if (!param) {
	return -1;
    }
    param->type = P_NUM;
    FREEZ (param->unit);
    if (unit) {
	param->unit = strdup (unit);
    }

    param->valnum = GfParmUnit2SI (unit, val);
    param->min = GfParmUnit2SI (unit, min);
    param->max = GfParmUnit2SI (unit, max);

    return 0;
}

/** Set a numerical parameter in a config file.
    @ingroup	paramslist
    @param	handle	handle of parameters
    @param	path	path of param
    @param	key	key name
    @param	unit	unit to convert the result to (NULL if SI wanted)
    @param	val	value to set
    @return	0	ok
    		<br>-1	error
    @warning	The key is created is necessary
 */
int
GfParmSetCurNum(void *handle, char *path, char *key, char *unit, tdble val)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*section;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmSetCurNum: bad handle (%p)\n", parmHandle);
	return -1;
    }
    section = (struct section *)GfHashGetStr (conf->sectionHash, path);
    if ((!section) || (!section->curSubSection)) {
	return -1;
    }

    param = getParamByName (conf, section->curSubSection->fullName, key, PARAM_CREATE);
    if (!param) {
	return -1;
    }
    param->type = P_NUM;
    FREEZ (param->unit);
    if (unit) {
	param->unit = strdup (unit);
    }

    val = GfParmUnit2SI (unit, val);
    param->valnum = val;
    param->min = val;
    param->max = val;

    return 0;
}



/** Check a parameter set against another.
    @ingroup	paramsfile
    @param	ref	Contains the min and max values (reference)
    @param	tgt	Contains the parameters to check.
    @return	0 Match
		<br>-1 Values are out of bounds
    @warning	Only the parameters present in tgt and in ref are tested.
    @see	GfParmMergeHandles
 */
int
GfParmCheckHandle(void *ref, void *tgt)
{
    struct parmHandle	*parmHandleRef = (struct parmHandle *)ref;
    struct parmHandle	*parmHandle = (struct parmHandle *)tgt;
    struct parmHeader	*confRef = parmHandleRef->conf;
    struct parmHeader	*conf = parmHandle->conf;
    struct section	*curSectionRef;
    struct section	*nextSectionRef;
    struct param	*curParamRef;
    struct param	*curParam;
    struct within	*curWithinRef;
    int			found;
    int			error = 0;

    if ((parmHandleRef->magic != PARM_MAGIC) || (parmHandle->magic != PARM_MAGIC)) {
	GfFatal ("GfParmCheckHandle: bad handle (%p)\n", parmHandle);
	return -1;
    }

    /* Traverse all the reference tree */
    curSectionRef = GF_TAILQ_FIRST (&(confRef->rootSection->subSectionList));
    while (curSectionRef) {
	curParamRef = GF_TAILQ_FIRST (&(curSectionRef->paramList));
	while (curParamRef) {
	    /* compare params */
	    curParam = getParamByName (conf, curSectionRef->fullName, curParamRef->name, 0);
	    if (curParam) {
		if (curParamRef->type != curParam->type) {
		    GfError("GfParmCheckHandle: type mismatch for parameter \"%s\" in (\"%s\" - \"%s\")\n",
			    curParamRef->fullName, conf->name, conf->filename);
		    error = -1;
		} else if (curParamRef->type == P_NUM) {
		    if ((curParam->valnum < curParamRef->min) || (curParam->valnum > curParamRef->max)) {
			GfError("GfParmCheckHandle: parameter \"%s\" out of bounds: min:%g max:%g val:%g in (\"%s\" - \"%s\")\n",
				curParamRef->fullName, curParamRef->min, curParamRef->max, curParam->valnum, conf->name, conf->filename);
			//error = -1;
		    }
		} else {
		    curWithinRef = GF_TAILQ_FIRST (&(curParamRef->withinList));
		    found = 0;
		    while (!found && curWithinRef) {
			if (!strcmp (curWithinRef->val, curParam->value)) {
			    found = 1;
			} else {
			    curWithinRef = GF_TAILQ_NEXT (curWithinRef, linkWithin);
			}
		    }
		    if (!found && strcmp (curParamRef->value, curParam->value)) {
			GfError("GfParmCheckHandle: parameter \"%s\" value:\"%s\" not allowed in (\"%s\" - \"%s\")\n",
				curParamRef->fullName, curParam->value, conf->name, conf->filename);
			//error = -1;
		    }
		}
	    }
	    curParamRef = GF_TAILQ_NEXT (curParamRef, linkParam);
	}
	nextSectionRef = GF_TAILQ_NEXT (curSectionRef, linkSection);
	while (!nextSectionRef) {
	    nextSectionRef = curSectionRef->parent;
	    if (!nextSectionRef) {
		/* Reached the root */
		break;
	    }
	    curSectionRef = nextSectionRef;
	    nextSectionRef = GF_TAILQ_NEXT (curSectionRef, linkSection);
	}
	curSectionRef = nextSectionRef;
    }

    return error;
}

static void
insertParamMerge (struct parmHandle *parmHandle, char *path, struct param *paramRef, struct param *param)
{
    struct parmHeader	*conf = parmHandle->conf;
    struct param	*paramNew;
    struct within	*withinRef;
    struct within	*within;
    tdble		num;
    char		*str;

    paramNew = getParamByName (conf, path, param->name, PARAM_CREATE);
    if (!paramNew) {
	return;
    }
    if (param->type == P_NUM) {
	paramNew->type = P_NUM;
	FREEZ (paramNew->unit);
	if (param->unit) {
	    paramNew->unit = strdup (param->unit);
	}
	if (param->min < paramRef->min) {
	    num = paramRef->min;
	} else {
	    num = param->min;
	}
	paramNew->min = num;
	if (param->max > paramRef->max) {
	    num = paramRef->max;
	} else {
	    num = param->max;
	}
	paramNew->max = num;
	num = param->valnum;
	if (num < paramNew->min) {
	    num = paramNew->min;
	}
	if (num > paramNew->max) {
	    num = paramNew->max;
	}
	paramNew->valnum = num;
    } else {
	paramNew->type = P_STR;
	FREEZ (paramNew->value);
	within = GF_TAILQ_FIRST (&(param->withinList));
	while (within) {
	    withinRef = GF_TAILQ_FIRST (&(paramRef->withinList));
	    while (withinRef) {
		if (!strcmp (withinRef->val, within->val)) {
		    addWithin (paramNew, within->val);
		    break;
		}
		withinRef = GF_TAILQ_NEXT (withinRef, linkWithin);
	    }
	    within = GF_TAILQ_NEXT (within, linkWithin);
	}
	str = NULL;
	withinRef = GF_TAILQ_FIRST (&(paramRef->withinList));
	while (withinRef) {
	    if (!strcmp (withinRef->val, param->value)) {
		str = param->value;
		break;
	    }
	    withinRef = GF_TAILQ_NEXT (withinRef, linkWithin);
	}
	if (!str) {
	    str = paramRef->value;
	}
	paramNew->value = strdup (str);

    }
}

static void
insertParam (struct parmHandle *parmHandle, char *path, struct param *param)
{
    struct parmHeader	*conf = parmHandle->conf;
    struct param	*paramNew;
    struct within	*within;

    paramNew = getParamByName (conf, path, param->name, PARAM_CREATE);
    if (!paramNew) {
	return;
    }
    if (param->type == P_NUM) {
	paramNew->type = P_NUM;
	FREEZ (paramNew->unit);
	if (param->unit) {
	    paramNew->unit = strdup (param->unit);
	}
	paramNew->valnum = param->valnum;
	paramNew->min = param->min;
	paramNew->max = param->max;
    } else {
	paramNew->type = P_STR;
	FREEZ (paramNew->value);
	paramNew->value = strdup (param->value);
	within = GF_TAILQ_FIRST (&(param->withinList));
	while (within) {
	    addWithin (paramNew, within->val);
	    within = GF_TAILQ_NEXT (within, linkWithin);
	}
    }
}


/** Merge two parameters sets into a new one.
    @ingroup	paramsfile
    @param	ref	reference handle
    @param	tgt	target handle for merge
    @param	mode	merge mode, can be any combination of:
		<br>#GFPARM_MMODE_SRC Use ref and modify existing parameters with tgt
		<br>#GFPARM_MMODE_DST Use tgt and verify ref parameters
		<br>#GFPARM_MMODE_RELSRC Release ref after the merge
		<br>#GFPARM_MMODE_RELDST Release tgt after the merge
    @return	The new handle containing the merge.
    @see	GfParmCheckHandle
 */
void *
GfParmMergeHandles(void *ref, void *tgt, int mode)
{
    struct parmHandle	*parmHandleRef = (struct parmHandle *)ref;
    struct parmHandle	*parmHandleTgt = (struct parmHandle *)tgt;
    struct parmHandle	*parmHandleOut;
    struct parmHeader	*confRef = parmHandleRef->conf;
    struct parmHeader	*confTgt = parmHandleTgt->conf;
    struct parmHeader	*confOut;
    struct section	*curSectionRef;
    struct section	*nextSectionRef;
    struct section	*curSectionTgt;
    struct section	*nextSectionTgt;
    struct param	*curParamRef;
    struct param	*curParamTgt;

    GfOut ("Merging \"%s\" and \"%s\" (%s - %s)\n", confRef->filename, confTgt->filename, ((mode & GFPARM_MMODE_SRC) ? "SRC" : ""), ((mode & GFPARM_MMODE_DST) ? "DST" : ""));

    if (parmHandleRef->magic != PARM_MAGIC) {
	GfFatal ("GfParmMergeHandles: bad handle (%p)\n", parmHandleRef);
	return NULL;
    }
    if (parmHandleTgt->magic != PARM_MAGIC) {
	GfFatal ("GfParmMergeHandles: bad handle (%p)\n", parmHandleTgt);
	return NULL;
    }

        /* Conf Header creation */
    confOut = createParmHeader ("");
    if (!confOut) {
	GfError ("gfParmReadBuf: conf header creation failed\n");
	return NULL;
    }

    /* Handle creation */
    parmHandleOut = (struct parmHandle *) calloc (1, sizeof (struct parmHandle));
    if (!parmHandleOut) {
	GfError ("gfParmReadBuf: calloc (1, %d) failed\n", sizeof (struct parmHandle));
	parmReleaseHeader (confOut);
	return NULL;
    }

    parmHandleOut->magic = PARM_MAGIC;
    parmHandleOut->conf = confOut;
    parmHandleOut->flag = PARM_HANDLE_FLAG_PRIVATE;

    if (mode & GFPARM_MMODE_SRC) {
	/* Traverse all the reference tree */
	curSectionRef = GF_TAILQ_FIRST (&(confRef->rootSection->subSectionList));
	while (curSectionRef) {
	    curParamRef = GF_TAILQ_FIRST (&(curSectionRef->paramList));
	    while (curParamRef) {
		/* compare params */
		curParamTgt = getParamByName (confTgt, curSectionRef->fullName, curParamRef->name, 0);
		if (curParamTgt) {
		    insertParamMerge (parmHandleOut, curSectionRef->fullName, curParamRef, curParamTgt);
		} else {
		    insertParam (parmHandleOut, curSectionRef->fullName, curParamRef);
		}
		curParamRef = GF_TAILQ_NEXT (curParamRef, linkParam);
	    }
	    nextSectionRef = GF_TAILQ_FIRST (&(curSectionRef->subSectionList));
	    if (nextSectionRef) {
		curSectionRef = nextSectionRef;
	    } else {
		nextSectionRef = GF_TAILQ_NEXT (curSectionRef, linkSection);
		while (!nextSectionRef) {
		    nextSectionRef = curSectionRef->parent;
		    if (!nextSectionRef) {
			/* Reached the root */
			break;
		    }
		    curSectionRef = nextSectionRef;
		    nextSectionRef = GF_TAILQ_NEXT (curSectionRef, linkSection);
		}
		curSectionRef = nextSectionRef;
	    }
	}
    }

    if (mode & GFPARM_MMODE_DST) {
	/* Traverse all the target tree */
	curSectionTgt = GF_TAILQ_FIRST (&(confTgt->rootSection->subSectionList));
	while (curSectionTgt) {
	    curParamTgt = GF_TAILQ_FIRST (&(curSectionTgt->paramList));
	    while (curParamTgt) {
		/* compare params */
		curParamRef = getParamByName (confRef, curSectionTgt->fullName, curParamTgt->name, 0);
		if (curParamRef) {
		    insertParamMerge (parmHandleOut, curSectionTgt->fullName, curParamRef, curParamTgt);
		} else {
		    insertParam (parmHandleOut, curSectionTgt->fullName, curParamTgt);
		}
		curParamTgt = GF_TAILQ_NEXT (curParamTgt, linkParam);
	    }
	    nextSectionTgt = GF_TAILQ_FIRST (&(curSectionTgt->subSectionList));
	    if (nextSectionTgt) {
		curSectionTgt = nextSectionTgt;
	    } else {
		nextSectionTgt = GF_TAILQ_NEXT (curSectionTgt, linkSection);
		while (!nextSectionTgt) {
		    nextSectionTgt = curSectionTgt->parent;
		    if (!nextSectionTgt) {
			/* Reached the root */
			break;
		    }
		    curSectionTgt = nextSectionTgt;
		    nextSectionTgt = GF_TAILQ_NEXT (curSectionTgt, linkSection);
		}
		curSectionTgt = nextSectionTgt;
	    }
	}
    }

    if (mode & GFPARM_MMODE_RELSRC) {
	GfParmReleaseHandle(ref);
    }

    if (mode & GFPARM_MMODE_RELDST) {
	GfParmReleaseHandle(tgt);
    }

    GF_TAILQ_INSERT_HEAD (&parmHandleList, parmHandleOut, linkHandle);

    return (void*)parmHandleOut;
}


/** Get the min and max of a numerical parameter.
    @ingroup	paramsdata
    @param	handle	handle of parameters
    @param	path	path of the attribute
    @param	key	key name
    @param	min	Receives the min value
    @param	max	Receives the max value
    @return	0 Ok
		<br>-1 Parameter not existing
 */
int
GfParmGetNumBoundaries(void *handle, char *path, char *key, tdble *min, tdble *max)
{
    struct parmHandle	*parmHandle = (struct parmHandle *)handle;
    struct parmHeader	*conf = parmHandle->conf;
    struct param	*param;

    if (parmHandle->magic != PARM_MAGIC) {
	GfFatal ("GfParmGetNumBoundaries: bad handle (%p)\n", parmHandle);
	return -1;
    }

    param = getParamByName (conf, path, key, 0);
    if (!param || (param->type != P_NUM)) {
	return -1;
    }

    *min = param->min;
    *max = param->max;

    return 0;
}
