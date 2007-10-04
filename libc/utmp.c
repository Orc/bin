/*
 * utmp with dbz
 */

#include <unistd.h>
#include <string.h>
#include <sys/file.h>

#include "utmp.h"
#include "dbz.h"


static char *ut_file = 0;
static int   ut_fd   = -1;
static int   ut_dbz  = 0;
static off_t ut_pos  = 0;


/* choose our own utmp file */

void
utmpname(const char *file)
{
    if (ut_file) free(ut_file);
    if (strcmp(file, UTMP_FILE) == 0)
	ut_file = 0;
    else
	ut_file = strdup(file);
}


/* (open the utmp file and) position to start of file
 */
void
setutent()
{
    char *file = ut_file ? ut_file : UTMP_FILE;

    if (ut_fd == -1) {
	ut_dbz = (ut_fd = dbminit(file)) != -1;

	if (!ut_dbz)
	    if ( (ut_fd = open(file, O_RDWR, 0644)) == -1)
		ut_fd = open(file, O_RDONLY, 0644);
    }
    ut_pos = 0;
}


/* close the utmp file
 */
void
endutent()
{
    if (ut_fd == -1) return;

    if (ut_dbz) {
	ut_dbz = 0;
	dbmclose();
    }
    else
	close(ut_fd);

    ut_fd = -1;
}


/* a static utmp entry for getutent(), getutid(), getutline()
 */
static struct utmp cache;


/* get the next utmp entry
 */
struct utmp *
getutent()
{
    if (ut_fd == -1 || lseek(ut_fd, ut_pos, SEEK_SET) != ut_pos) return 0;

    if (read(ut_fd, &cache, sizeof cache) != sizeof cache) return 0;
    ut_pos += sizeof cache;

    return &cache;
}


/* get a utmp entry by type or id
 */
struct utmp *
getutid(struct utmp *id)
{
    if (ut_fd == -1 || lseek(ut_fd, ut_pos, SEEK_SET) != ut_pos) return 0;

    if (id->ut_type > 0 && id->ut_type <= OLD_TIME) {
	/* table scan no matter what */
	while (read(ut_fd, &cache, sizeof cache) == sizeof cache) {
	    if (cache.ut_type == id->ut_type)
		return &cache;
	    ut_pos += sizeof cache;
	}
    }
    else if (id->ut_type >= INIT_PROCESS && id->ut_type <= DEAD_PROCESS) {
	/* table scan no matter what */
	while (read(ut_fd, &cache, sizeof cache) == sizeof cache) {
	    if (memcmp(id->ut_id, cache.ut_id, sizeof cache.ut_id) == 0)
		return &cache;
	    ut_pos += sizeof cache;
	}
    }
    else return 0;
}


/* get a utmp entry by line
 */
struct utmp *
getutline(struct utmp *line)
{
    if (ut_fd == -1 || lseek(ut_fd, ut_pos, SEEK_SET) != ut_pos) return 0;

    if (ut_dbz) {
	datum key, data;
	long data_pos;

	key.dsize = strlen(line->ut_line);
	key.dptr  = line->ut_line;

	data = fetch(key);

	if ( data.dsize == sizeof data_pos ) {
	    memcpy(&data_pos, data.dptr, data.dsize);
	    if ( lseek(ut_fd, data_pos, SEEK_SET) == data_pos
	       && read(ut_fd, &cache, sizeof cache) == sizeof cache )
		return &cache;
	}
    }
    else {
	while ( read(ut_fd, &cache, sizeof cache) == sizeof cache ) {
	    ut_pos += sizeof cache;
	    if ( strcmp(line->ut_line, cache.ut_line) == 0 )
		return &cache;
	}
    }
    return 0;
}


/* save a utmp entry by line
 */
void
setutline(struct utmp *line)
{
    if (ut_fd == -1 || lseek(ut_fd, ut_pos, SEEK_SET) != ut_pos) return;

    if (ut_dbz) {
	datum key, data;
	long data_pos;

	key.dsize = strlen(line->ut_line);
	key.dptr = line->ut_line;

	data = fetch(key);

	if ( data.dsize == sizeof data_pos ) /* update existing entry */ {
	    memcpy(&data_pos, data.dptr, data.dsize);
	    if ( lseek(ut_fd, data_pos, SEEK_SET) == data_pos )
		write(ut_fd, line, sizeof *line);
	}
	else /* new entry! store it away */ {
	    data.dsize = sizeof *line;
	    data.dptr = (char*) line;

	    /* keep writers from stepping on each other */
	    if (flock(ut_fd, LOCK_EX) == 0) {
		store(key,data);
		flock(ut_fd, LOCK_UN);
	    }
	}
    }
    else {
	struct utmp *data;

	if ( (data = getutline(line))
		  && lseek(ut_fd, ut_pos, SEEK_SET) == ut_pos ) {
	    write(ut_fd, line, sizeof *line);
	    return;
	}
    }
}
