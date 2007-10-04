/*

dbz.c  V1.9

Copyright 1988 Jon Zeeff (zeeff@b-tech.ann-arbor.mi.us)
You can use this code in any manner, as long as you leave my name on it
and don't hold me responsible for any problems with it.

Hacked on by gdb@ninja.UUCP (David Butler); Sun Jun  5 00:27:08 CDT 1988

Various improvments + INCORE by moraes@ai.toronto.edu (Mark Moraes)

These routines replace dbm as used by the usenet news software
(it's not a full dbm replacement by any means).  It's fast and
simple.  It contains no AT&T code.

BSD sites will notice some savings in disk space.  Sys V sites without 
dbm will notice much faster operation.  Note that I find that dbz is 
2-18 times faster than dbm.  

Note: if you change INDEX_SIZE, you have to run mkdbm to udate the index
file.

This code relies on the fact that news stores a pointer to the history 
file as the dbm data.  It doesn't store another copy of the key like 
dbm does so it saves disk space.  Make sure that the data file exists 
with the proper name.  All you can do is fetch() and store() data.  

Just make news with the DBM option and link with dbz.o.

*/

/*
 *  Set this to the something several times larger than the maximum # of
 *  lines in a history file.  It should be a prime number. 
 */
#define INDEX_SIZE  120011 /* Use 300007L unless you need to save space */

#define SYSV			/* just to keep lint happy */
/* #define BNEWS		/* B news needs lowercase */
/* #define CNEWS		/* C News needs rfc822izing */

#define BLKSIZE	1023

/* 
 Define INCORE if you want to hold the index file entirely in 
 memory.  It should speed things up considerably if you are doing many 
 db operations per invocation (eg, news expire - yes, unbatching - no) 
 and shouldn't cost much for VM machines.  Note that this will work 
 even if there isn't enough memory to hold the index file - it'll just 
 revert to the slow old scheme, so it doesn't hurt to define it.  Do 
 not use this on 16 bit int machines.   Make sure that dbmclose()
 gets called.
*/ 

/* #define INCORE   /* do it on the command line, not here */

/* 3rd argument to read() is unsigned in SYSV */
#ifdef SYSV
# define io_int	unsigned
#else
# define io_int int
#endif

#ifndef NULL
#define NULL 0
#endif

#define DEBUG if (0) (void) printf	          /* for no debugging */
/* #define DEBUG if (1) (void) printf		  /* for debugging */
/* #define DEBUG if (debug) (void) printf        /* for "rn" type debugging */

/* Note: let the optimizer remove any if(0) or if(1) code above */

#include <fcntl.h>
#include <string.h>
#include <ctype.h>

extern long lseek();
extern char *malloc();
extern void free();

static long get_ptr();
static void lcase();
static int put_ptr();
static long hash();
static void crcinit();

#include "dbz.h"

static int Data_fd;		/* points to /usr/lib/news/[n]history */
static int Index_fd = -1;	/* points to /usr/lib/news/[n]history.pag */
#ifdef INCORE
static long *core_index = (long *) 0;
#endif

int 
dbzinit(name)
char *name;
{
	char *index_file;	/* index file name */
#ifdef INCORE
        io_int nbytes;
#endif

	if (Index_fd >= 0) {
		DEBUG("dbzinit: dbzinit aready called once\n");
		return(-1);    /* init already called once */
	}

#define PAGEXT ".pag"
	if ((index_file =
	 malloc((unsigned) (1 + strlen(name) + sizeof(PAGEXT)))) == (char *)0)
		return -1;
	
	(void) strcpy(index_file, name);
	(void) strcat(index_file, PAGEXT);

	/* if we don't have permission to open the pag file for read/write */
	/* then we may never attempt a store().  If we do, it will fail */

	if ((Index_fd = open(index_file, O_RDWR)) < 0 &&
	    (Index_fd = open(index_file, O_RDONLY)) < 0)
	{
		DEBUG("dbzinit: Index_file open failed\n");
		return(-1);
	}

#ifdef INCORE
	nbytes = (INDEX_SIZE * sizeof(long));
	core_index = (long *) malloc((unsigned) nbytes);
	if (core_index != (long *) NULL) {
		if( read(Index_fd, (char *) core_index, nbytes)
		 == -1) {
			DEBUG("dbzinit: read failed\n");
			return(-1);
		}
	}
#endif /* INCORE */

	if ((Data_fd = open(name, O_RDONLY)) < 0) {

		/* The only time the data file does not exist is when */
		/* expire is running (as "news") and it is ok to create it */
		/* If we don't have "permission" the create will fail, too */

		if (close(creat(name, 0644)) < 0 ||
		    (Data_fd = open(name, O_RDONLY)) < 0) {
			DEBUG("dbzinit: Data_file open failed\n");
			(void) close(Index_fd);
			free(index_file);
			Index_fd = -1;
			return(-1);
		}
	}

	crcinit();	/* initialize the crc table */
	free(index_file);
	DEBUG("dbzinit: succeeded\n");
	return Data_fd;
}

int
dbzclose()
{
	int err = 0;
	
	if (Index_fd >= 0) {
#ifdef INCORE
		if (core_index != (long *) NULL) {
			int count;
			io_int nbytes = INDEX_SIZE * sizeof(long);
			
			count = write(Index_fd, (char *) core_index, nbytes);
			if (count != nbytes)
				err++;
			(void) free((char *) core_index);
			core_index = (long *) NULL;
		}
#endif
		err += close(Index_fd);
		Index_fd = -1;
		err += close(Data_fd);
	}
	DEBUG("dbzclose: %s\n", err == 0 ? "succeeded" : "failed");
	return (err == 0 ? 0 : -1);
}

/* get an entry from the database */

dbzdatum
dbzfetch(key)
dbzdatum key;
{
	register long index_ptr;
	long index_size = INDEX_SIZE;
        char buffer[BLKSIZE + 1];
	static long data_ptr;
	dbzdatum output;
	int keysize;

	DEBUG("fetch: (%s)\n", key.dptr);
	output.dptr = (char *)0;
	output.dsize = 0;

	for (index_ptr = hash(key.dptr, key.dsize);
	    --index_size && (data_ptr = get_ptr(index_ptr)) >= 0L;
	    index_ptr = ++index_ptr % INDEX_SIZE) {

		if (lseek(Data_fd, data_ptr, 0) == -1L) {
			DEBUG("fetch: seek failed\n");
			return output;
		}
		/* Key had better be less than BLKSIZE */
		keysize = key.dsize;
		if (keysize >= BLKSIZE) {
			keysize = BLKSIZE;
			DEBUG("keysize is %d - truncated to %d\n", key.dsize,
			 BLKSIZE);
		}
		if (read(Data_fd, buffer, (io_int) keysize) != keysize) {
			DEBUG("fetch: read failed\n");
			return output;
		}
		
		buffer[keysize] = '\0';
#ifdef BNEWS
		/* key should be lcase version of article id and no tab */
		lcase(buffer, key.dsize);
		if (buffer[key.dsize - 1] == '\t') 
			buffer[key.dsize - 1] = '\0';
#endif
#ifdef CNEWS
		if (buffer[keysize - 1] == '\t') 
		   buffer[keysize - 1] = 0;      /* get rid of the tab */
		rfc822ize(buffer);
#endif

		DEBUG("fetch: buffer (%s) looking for (%s) size = %d\n", 
                       buffer,key.dptr,keysize);

		if (strncmp(key.dptr, buffer, keysize - 1) == 0) {
			/* we found it */
			output.dptr = (char *)&data_ptr;
			output.dsize = sizeof(long);
			DEBUG("fetch: successful\n");
			return(output);
		}
	}

	/* we didn't find it */

	DEBUG("fetch: failed\n");
	return(output);
}

/* add an entry to the database */
dbzstore(key, data)
dbzdatum key;
dbzdatum data;
{
	/* lint complains about a possible pointer alignment problem here */
	/* it is not a problem because dptr is the first element of a */
	/* structure and should be aligned for anything */

	DEBUG("store: (%s, %ld)\n", key.dptr, *((long *)data.dptr));
	return(put_ptr(hash(key.dptr, key.dsize), *((long *)data.dptr)));
}

/* get a data file pointer from the specified location in the index file */

static long
get_ptr(index_ptr)
long index_ptr;
{
	long data_ptr;

	DEBUG("get_ptr: (%ld)\n", index_ptr);

#ifdef INCORE
	/* If we have the index file in memory, use it */
	if (core_index != (long *) NULL) {
		DEBUG("get_ptr: (incore) succeeded\n");
		return (core_index[index_ptr] - 1);
	}
#endif
		
	/* seek to where it should be */
	if (lseek(Index_fd, (long)(index_ptr * sizeof(long)) ,0) == -1L) {
		DEBUG("get_ptr: seek failed\n");
		return -1L;
	}

	/* read it */
	if (read(Index_fd, (char *)&data_ptr, sizeof(long)) != sizeof(long) ||
	    data_ptr == 0L) {
		DEBUG("get_ptr: failed\n");
		return(-1L);
	}

	DEBUG("get_ptr: succeeded\n");
	return(--data_ptr);	/* remove the offset we added in put_ptr */
}

/* put a data file pointer into the specified location in the index file */
/* move down further if slots are full (linear probing) */

static
put_ptr(index_ptr, data_ptr)
register long index_ptr;
long data_ptr;
{
	long index_size = INDEX_SIZE;

	/* find an empty slot */
	while (--index_size && get_ptr(index_ptr) >= 0L) {
		index_ptr = ++index_ptr % INDEX_SIZE;
	}

	if (index_size == 0L) {
		DEBUG("put_ptr: hash table overflow - failed\n");
		return(-1);
	}

#ifdef INCORE
	/* If we have the index file in memory, use it */
	if (core_index != (long *) NULL) {
		core_index[index_ptr] = data_ptr + 1;
		DEBUG("put_ptr: (incore) succeeded\n");
		return 0;
	}
#endif
		
	/* seek to spot */
	if (lseek(Index_fd, (long)(index_ptr * sizeof(long)), 0) == -1L) {
		DEBUG("put_ptr: seek failed\n");
		return -1;
	}

	++data_ptr;   /* add one so that we can use 0 as no pointer */

	/* write in data */
	if (write(Index_fd, (char *)&data_ptr, sizeof(long)) != sizeof(long)) {
		DEBUG("put_ptr: write failed\n");
		return(-1);
	}

	DEBUG("put_ptr: succeeded\n");
	return(0);
}

#ifdef BNEWS

static void
lcase(s, n)
register char *s;
register int n;
{
	for (; n > 0; --n, ++s) {
		if (isupper(*s)) *s = tolower(*s);
	}
}

#endif

/* This is a simplified version of the pathalias hashing function.
 * Thanks to Steve Belovin and Peter Honeyman
 *
 * hash a string into a long int.  31 bit crc (from andrew appel).
 * the crc table is computed at run time by crcinit() -- we could
 * precompute, but it takes 1 clock tick on a 750.
 *
 * This fast table calculation works only if POLY is a prime polynomial
 * in the field of integers modulo 2.  Since the coefficients of a
 * 32-bit polynomial won't fit in a 32-bit word, the high-order bit is
 * implicit.  IT MUST ALSO BE THE CASE that the coefficients of orders
 * 31 down to 25 are zero.  Happily, we have candidates, from
 * E. J.  Watson, "Primitive Polynomials (Mod 2)", Math. Comp. 16 (1962):
 *	x^32 + x^7 + x^5 + x^3 + x^2 + x^1 + x^0
 *	x^31 + x^3 + x^0
 *
 * We reverse the bits to get:
 *	111101010000000000000000000000001 but drop the last 1
 *         f   5   0   0   0   0   0   0
 *	010010000000000000000000000000001 ditto, for 31-bit crc
 *	   4   8   0   0   0   0   0   0
 */

#define POLY 0x48000000L	/* 31-bit polynomial (avoids sign problems) */

static long CrcTable[128];

static void
crcinit()
{	register int i, j;
	register long sum;

	for (i = 0; i < 128; ++i) {
		sum = 0L;
		for (j = 7 - 1; j >= 0; --j)
			if (i & (1 << j))
				sum ^= POLY >> j;
		CrcTable[i] = sum;
	}
	DEBUG("crcinit: done\n");
}

static long
hash(name, size)
register char *name;
register int size;
{
	register long sum = 0L;

	while (size--) {
		sum = (sum >> 7) ^ CrcTable[(sum ^ (*name++)) & 0x7f];
	}
	DEBUG("hash: returns (%ld)\n", sum % INDEX_SIZE);
	return(sum % INDEX_SIZE);
}
