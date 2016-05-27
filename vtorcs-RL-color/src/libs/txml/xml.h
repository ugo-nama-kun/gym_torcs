/*
 * xml.h -- Interface file for XML 
 *
 * @(#) $Id: xml.h,v 1.1.1.1 2001/06/24 18:29:48 torcs Exp $
 */
 
#ifndef _XML_H_
#define _XML_H_

typedef struct xmlAttribute {
    char		*name;
    char		*value;
    struct xmlAttribute	*next;
} txmlAttribute;

typedef struct xmlElement {
    char		*name;		/* element name */
    char		*pcdata;	/* string associated with this element */
    struct xmlAttribute	*attr;		/* attributes of this element */
    int			level;		/* nested level */
    struct xmlElement	*next;		/* next element at the same level */
    struct xmlElement	*sub;		/* next element at the next level (nested) */
    struct xmlElement	*up;		/* upper element */
} txmlElement;    

extern txmlElement *xmlInsertElt(txmlElement *curElt, const char *name, const char **atts);
extern txmlElement *xmlReadFile(const char *file);
extern int          xmlWriteFile(const char *file, txmlElement *startElt, char *dtd);
extern char        *xmlGetAttr(txmlElement *curElt, char *attrname);
extern txmlElement *xmlNextElt(txmlElement *startElt);
extern txmlElement *xmlSubElt(txmlElement *startElt);
extern txmlElement *xmlWalkElt(txmlElement *startElt);
extern txmlElement *xmlWalkSubElt(txmlElement *startElt, txmlElement *topElt);
extern txmlElement *xmlFindNextElt(txmlElement *startElt, char *name);
extern txmlElement *xmlFindEltAttr(txmlElement *startElt, char *name, char *attrname, char *attrvalue);

#endif /* _XML_H_ */ 



