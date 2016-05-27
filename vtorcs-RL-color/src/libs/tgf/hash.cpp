/***************************************************************************

    file        : hash.cpp
    created     : Sat Dec 14 16:40:15 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: hash.cpp,v 1.4.2.2 2008/06/01 07:43:24 berniw Exp $                                  

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
    		This is the hash computation API.
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: hash.cpp,v 1.4.2.2 2008/06/01 07:43:24 berniw Exp $
    @ingroup	hash
*/

#include <tgf.h>

typedef struct HashElem
{
    char				*key;
    int					size;
    void				*data;
    GF_TAILQ_ENTRY(struct HashElem) 	link;
} tHashElem;

GF_TAILQ_HEAD(HashHead, tHashElem);

typedef struct HashHeader
{
    int		type;
    int		size;
    int		nbElem;
    /* for table traversal */
    int		curIndex;
    tHashElem	*curElem;

    tHashHead	*hashHead;
} tHashHeader;



#define HASH_BYTE(x, y)  (((y) + ((x) << 4) + ((x) >> 4)) * 11)
#define DEFAULT_SIZE	32

static unsigned int
hash_str (tHashHeader *hash, const char *sstr)
{
	const unsigned char *str = (const unsigned char *)sstr;
	unsigned int val = 0;
	
	if (!str) {
		return 0;
	}

	/* Hash courtesy of the R5 hash in reiserfs modulo sign bits */
	while (*str)
	{
		val = (val + (*str >> 4) + (*str << 4)) * 11;
		str++;
	}
	
	return val % hash->size;
}

static unsigned int
hash_buf (tHashHeader *hash, char *sdata, int len)
{
  unsigned int	val = 0;
  unsigned char *data = (unsigned char *)sdata;
  int 		i;

  if (!data)
    return 0;

  /* Hash courtesy of the R5 hash in reiserfs modulo sign bits */
  for (i = 0; i < len; i++)
    val = (val + (data[i] >> 4) + (data[i] << 4)) * 11;

  return val % hash->size;
}

/** Create a new hash table
    @ingroup	hash
    @param	type	Type of key used (GF_HASH_TYPE_STR or GF_HASH_TYPE_BUF)
    @return	handle on new hash table
		<br>0 if Error
*/
void *
GfHashCreate(int type)
{
    tHashHeader	*curHeader;
    int		i;

    curHeader = (tHashHeader*)malloc(sizeof(tHashHeader));
    if (!curHeader) {
	return NULL;
    }
    curHeader->type = type;
    curHeader->size = DEFAULT_SIZE;
    curHeader->nbElem = 0;
    curHeader->curIndex = 0;
    curHeader->curElem = NULL;
    curHeader->hashHead = (tHashHead *)malloc(DEFAULT_SIZE * sizeof(tHashHead));
    for (i = 0; i < DEFAULT_SIZE; i++) {
	GF_TAILQ_INIT(&(curHeader->hashHead[i]));
    }
    return (void*)curHeader;
}

/* Double the size of the hash table */
static void
gfIncreaseHash(tHashHeader *curHeader)
{
    tHashHead	*oldHashHead;
    tHashElem	*curElem;
    int		oldSize;
    int		hindex;
    int		i;

    oldHashHead = curHeader->hashHead;
    oldSize = curHeader->size;

    curHeader->size *= 2;
    curHeader->hashHead = (tHashHead *)malloc(curHeader->size * sizeof(tHashHead));
    for (i = 0; i < curHeader->size; i++) {
	GF_TAILQ_INIT(&(curHeader->hashHead[i]));
    }
    
    /* copy the elements */
    for (i = 0; i < oldSize; i++) {
	while ((curElem = GF_TAILQ_FIRST(&(oldHashHead[i]))) != NULL) {
	    /* remove from old list */
	    GF_TAILQ_REMOVE(&(oldHashHead[i]), curElem, link);
	    /* insert in new list */
	    switch (curHeader->type) {
	    case GF_HASH_TYPE_STR:
		hindex = hash_str(curHeader, curElem->key);
		break;
	    case GF_HASH_TYPE_BUF:
		hindex = hash_buf(curHeader, curElem->key, curElem->size);
		break;
	    default:
		hindex = 0;	/* for the compiler... */
		break;
	    }
	    GF_TAILQ_INSERT_TAIL(&(curHeader->hashHead[hindex]), curElem, link);
	}
    }
    free(oldHashHead);
}

/** Add an element with a string key to a hash table.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key string to hash.
    @param	data	user data.
    @return	0 OK, 1 NOK.
*/
int
GfHashAddStr(void *hash, const char *key, void *data)
{
	tHashHeader		*curHeader = (tHashHeader *)hash;
	tHashElem		*newElem;
	unsigned int	index;
	
	if (curHeader->type != GF_HASH_TYPE_STR) {
		return 1;
	}
	
	if ((curHeader->nbElem + 1) > (2 * curHeader->size)) {
		gfIncreaseHash(curHeader);
	}
	
	index = hash_str(curHeader, key);
	newElem = (tHashElem*)malloc(sizeof(tHashElem));
	if (!newElem) {
		return 1;
	}

	newElem->key = strdup(key);
	newElem->size = strlen(key) + 1;
	newElem->data = data;
	GF_TAILQ_INSERT_TAIL(&(curHeader->hashHead[index]), newElem, link);
	curHeader->nbElem++;
	
	return 0;
}

/* Remove a table element */
static void *
gfRemElem(tHashHead *hashHead, tHashElem *elem)
{
    void	*data;

    data = elem->data;
    free(elem->key);
    GF_TAILQ_REMOVE(hashHead, elem, link);
    free(elem);
    return data;
}


/** Remove an element with a string key from a hash table.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key string to hash.
    @return	User data.
*/
void *
GfHashRemStr(void *hash, char *key)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;
    tHashElem		*curElem;
    unsigned int	index;

    index = hash_str(curHeader, key);
    curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[index]));
    while (curElem) {
	if (!strcmp(curElem->key, key)) {
	    curHeader->nbElem--;
	    return gfRemElem(&(curHeader->hashHead[index]), curElem);
	}
	curElem = GF_TAILQ_NEXT(curElem, link);
    }
    return NULL;
}

/** Get the user data associated with a string key.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key string to hash.
    @return	User data.
*/
void *
GfHashGetStr(void *hash, const char *key)
{
	tHashHeader		*curHeader = (tHashHeader *)hash;
	tHashElem		*curElem;
	unsigned int	index;
	
	index = hash_str(curHeader, key);
	curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[index]));
	while (curElem) {
		if (!strcmp(curElem->key, key)) {
			return curElem->data;
		}
		curElem = GF_TAILQ_NEXT(curElem, link);
	}
	return NULL;
}



/** Add an element with a memory buffer key to a hash table.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key buffer to hash.
    @param	sz	size of the buffer.
    @param	data	user data.
    @return	none
*/
void
GfHashAddBuf(void *hash, char *key, size_t sz, void *data)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;
    tHashElem		*newElem;
    unsigned int	index;
    
    if (curHeader->type != GF_HASH_TYPE_BUF) {
	return;
    }

    if ((curHeader->nbElem + 1) > (2 * curHeader->size)) {
	gfIncreaseHash(curHeader);
    }

    index = hash_buf(curHeader, key, sz);
    newElem = (tHashElem*)malloc(sizeof(tHashElem));
    newElem->key = (char *)malloc(sz);
    memcpy(newElem->key, key, sz);
    newElem->size = sz;
    newElem->data = data;
    GF_TAILQ_INSERT_TAIL(&(curHeader->hashHead[index]), newElem, link);
    curHeader->nbElem++;
}

/** Remove an element with a memory buffer key from a hash table.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key buffer to hash.
    @param	sz	size of the buffer.
    @return	User data.
*/
void *
GfHashRemBuf(void *hash, char *key, size_t sz)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;
    tHashElem		*curElem;
    unsigned int	index;

    index = hash_buf(curHeader, key, sz);
    curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[index]));
    while (curElem) {
	if (!memcmp(curElem->key, key, sz)) {
	    curHeader->nbElem--;
	    return gfRemElem(&(curHeader->hashHead[index]), curElem);
	}
	curElem = GF_TAILQ_NEXT(curElem, link);
    }
    return NULL;
}


/** Get the user data associated with a memory buffer key.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	key	key buffer to hash.
    @param	sz	size of the buffer.
    @return	User data.
*/
void *
GfHashGetBuf(void *hash, char *key, size_t sz)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;
    tHashElem		*curElem;
    unsigned int	index;

    index = hash_buf(curHeader, key, sz);
    curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[index]));
    while (curElem) {
	if (!memcmp(curElem->key, key, sz)) {
	    return curElem->data;
	}
	curElem = GF_TAILQ_NEXT(curElem, link);
    }
    return NULL;
}

/** Release a hash table.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @param	hashFree pointer on user function used to free the user data (NULL if not used).
    @return	none
*/
void
GfHashRelease(void *hash, tfHashFree hashFree)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;
    tHashElem		*curElem;
    void		*data;
    int			i;
    
    for (i = 0; i < curHeader->size; i++) {
	while ((curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[i]))) != NULL) {
	    data = gfRemElem(&(curHeader->hashHead[i]), curElem);
	    if (hashFree) {
		hashFree(data);
	    }
	}
    }
    free(curHeader->hashHead);
    free(curHeader);
}

/** Get the first user data of a hash table.
    This is used for table scan.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @return	User data.
    @see	GfHashGetNext
*/
void *
GfHashGetFirst(void *hash)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;

    curHeader->curIndex = -1;
    curHeader->curElem = NULL;
    
    return GfHashGetNext(hash);
}


/** Get the next user data of a hash table.
    This is used for table scan.
    @ingroup	hash
    @param	hash	Current hash table handle.
    @return	User data.
    @see	GfHashGetFirst
*/
void *
GfHashGetNext(void *hash)
{
    tHashHeader		*curHeader = (tHashHeader *)hash;

    if (curHeader->curElem) {
	curHeader->curElem = GF_TAILQ_NEXT(curHeader->curElem, link);
    }

    while (!curHeader->curElem) {
	curHeader->curIndex++;
	if (curHeader->curIndex == curHeader->size) {
	    return NULL;
	}
	curHeader->curElem = GF_TAILQ_FIRST(&(curHeader->hashHead[curHeader->curIndex]));
    }

    return curHeader->curElem->data;
}
