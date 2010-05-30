/*
 *   Copyright (c) 1999,2002 David Parsons. All rights reserved.
 *   
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. All advertising materials mentioning features or use of this
 *     software must display the following acknowledgement:
 *     
 *   This product includes software developed by David Parsons
 *   (orc@pell.chi.il.us)
 *
 *  4. My name may not be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *     
 *  THIS SOFTWARE IS PROVIDED BY DAVID PARSONS ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVID
 *  PARSONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * a little handmade df, for Mastodon
 */
#include "config.h"

#include <stdio.h>
#include <basis/options.h>
#include <memory.h>
#include <stdlib.h>
#include <mntent.h>
#include <unistd.h>

#include <sys/vfs.h> /* Linux-specific: for statfs */

#define PROCMNT	"/proc/mounts"

char **filesys;
int nrfilesys = 0;
char **exclusions;
int nrexclusions = 0;
char **inclusions;
int nrinclusions = 0;

enum { BYTES, POSIX, KILOBYTES, MEGABYTES, HUMAN } display = POSIX;

int show_zero_length = 0;
int show_inodes = 0;
int show_fs_type = 0;
int local_fs_only = 0;

char *pgm;

#define KILO	(long)(1024)
#define MEGA	(long)(KILO*KILO)
#define GIGA	(long)(KILO*KILO*KILO)


/*
 * ullfmt() is a local that formats a long long number in
 * decimal form.  It's here because linux libc 4.x doesn't
 * support long long formats in printf
 */
static void
ullfmt(unsigned long long arg, char **bufp)
{
    if (arg < 0) {
	*(*bufp)++ = '-';
	ullfmt(-arg, bufp);
    }
    else {
	if (arg > 9)
	    ullfmt(arg / 10, bufp);
	*(*bufp)++ = '0' + (char)(arg % 10);
    }
} /* ullfmt */
 

/*
 * print() prints out a filesystem total in our desired print format. 
 * we are passed in a count of blocks and a blocksize, which is most
 * likely larger than 2>>31. We use 64 (or 128, if this is a machine
 * with 64-bit integers) bit arithmatic to compute the number, and
 * then we massage it into the desired format, with is either raw
 * bytes, POSIX-approved 512-byte chunks, 1k chunks, megabytes, or
 * free-format (kilobytes, megabytes or gigabytes; when filesystems
 * start getting into the terabyte range this will start to produce
 * pretty junky looking output.
 */
char *
print(long count, long size)
{
    static char buf[80];
    char *bufp = buf;
    unsigned long long total = (long long)count;
    unsigned long long fraction;
    
    total *= (long long)size;
    
    bzero(buf, sizeof buf);

    switch (display)  {
    case BYTES:
	ullfmt(total, &bufp);
	break;

    case POSIX:
	total /= (KILO/2);
	ullfmt(total, &bufp);
	break;

    case KILOBYTES:
	total /= KILO;
	ullfmt(total, &bufp);
	break;

    case MEGABYTES:
	total /= MEGA;
	ullfmt(total, &bufp);
	break;

    default:
	if (total >= GIGA) {
	    fraction = ((total % GIGA) / (100 * MEGA));
#if 0
	    if (fraction % 10 >= 5)
		if (fraction >= 95) {
		    total += GIGA;
		    fraction = 0;
		}
		else
		    fraction += 5;
#endif
	    if (fraction >= 10) {
		fraction = 0;
		total += GIGA;
	    }
		
		
	    ullfmt((long long)(total / GIGA), &bufp);
	    *bufp++ = '.';
#if 0
	    ullfmt(fraction/10, &bufp);
#else
	    ullfmt(fraction, &bufp);
#endif
	    *bufp++ = 'G';
	}
	else if (total >= MEGA) {
	    ullfmt(total/MEGA, &bufp);
	    *bufp++ = 'M';
	}
	else if (total >= KILO) {
	    ullfmt(total/KILO, &bufp);
	    *bufp++ = 'K';
	}
	else
	    ullfmt(total, &bufp);
    }
    *bufp = 0;
    return buf;
} /* print */


/*
 * percent() returns the % available as a string suitable for printing
 */
char *
percent(unsigned long total, unsigned long avail)
{
    static char bfr[80];


    while (total > 100000) {
	total /= 100;
	avail /= 100;
    }

    if (total == 0)
	strcpy(bfr, " 0%");
    else
	sprintf(bfr, "%2u%%", (100*avail)/total);
    return bfr;
} /* percent */


/*
 * addToList() adds a string to a list
 */
void
addToList(char *item, char ***list, int *nrlist)
{
    (*list) = realloc( (*list), (1+(*nrlist)) * sizeof (**list) );
    if ( (*list) == 0 ) {
	perror(pgm);
	exit(1);
    }
    (*list)[(*nrlist)] = item;
    (*nrlist)++;
} /* addToList */


/*
 * header() spits out the df header.  It may be called repeatedly, but it
 *          will only print the header once.
 */
void
header()
{
    static int didheader = 0;

    if (didheader)
	return;
    didheader = 1;

    printf("%-20s", "Filesystem");
    if (show_fs_type)
	printf(" %-7s", "Type");

    if (show_inodes) {
	printf(" %-7s %-7s %-7s", "Inodes", "IUsed", "IFree");
	printf(" %-6s", "%Ifree");
    }
    else {
	char *format;
	char *size;
	switch (display) {
	case BYTES:
	    format = "    Bytes       Used      Available";
	    break;
	case POSIX:
	    format = " 512-blocks  Used   Available";
	    break;
	case KILOBYTES:
	    format = "Kilobytes    Used   Available";
	    break;
	case MEGABYTES:
	    format = "Megabytes  Used   Avail";
	    break;
	default:
	    format = "  Size   Used  Avail";
	    break;
	}
	fputs(format, stdout);
	printf(" Capacity");
    }
    printf(" Mounted on\n");
} /* header */


/*
 * printmount() prints the disk or inode used information for a given
 *              mount.
 */
void
printmount(struct mntent *mnt, char *mountdir)
{
    int i;
    struct statfs fs;

    if (statfs(mnt->mnt_dir, &fs) != 0) {
	perror(mountdir);
	return;
    }

    if (local_fs_only && mnt->mnt_fsname[0] != '/')
	return;

    for (i = 0; i < nrexclusions; i++)
	if (strcmp(mnt->mnt_type, exclusions[i]) == 0)
	    return;

    if (nrinclusions > 0) {
	for (i = 0; i < nrinclusions; i++)
	    if (strcmp(mnt->mnt_type, inclusions[i]) == 0)
		break;
	if (i >= nrinclusions)
	    return;
    }

    if (fs.f_blocks <= 0 && !show_zero_length)
	return;

    header();
    if (display == HUMAN && strlen(mnt->mnt_fsname) > 19)
	printf("%s\n%-19s", mnt->mnt_fsname, "");
    else
	printf("%-19s", mnt->mnt_fsname);

    if (show_fs_type)
	printf(" %-7s", mnt->mnt_type);

    if (show_inodes) {
	if (fs.f_files >= 0 && fs.f_ffree >= 0)
	    printf(" %7d %7d %7d %6s",
		    fs.f_files, fs.f_ffree, fs.f_files-fs.f_ffree,
			    percent(fs.f_files, fs.f_ffree));
	else
	    printf(" %7s %7s %7s %6s", "?", "?", "?", "");
    }
    else {
	char *fmt;

	switch (display) {
	case BYTES:	fmt = " %11s";	break;
	case POSIX:
	case KILOBYTES:	fmt = " %9s";	break;
	case MEGABYTES:	fmt = " %7s";	break;
	default:	fmt = " %6s";	break;
	}

	if (fs.f_blocks < 0 || fs.f_bsize < 0) {
	    printf(fmt, "?");
	    printf(fmt, "?");
	    printf(fmt, "?");

	    printf(" %6s  ", "");
	}
	else {
	    printf(fmt, print(fs.f_blocks, fs.f_bsize));
	    printf(fmt, print(fs.f_blocks-fs.f_bfree, fs.f_bsize));
	    printf(fmt, print(fs.f_bavail, fs.f_bsize));

	    printf(" %6s  ", percent(fs.f_blocks, fs.f_blocks-fs.f_bfree));
	}
    }

    printf(" %s\n", mnt->mnt_dir);
} /* printmount */


/*
 * utility comparison function for alpha-sorting the mounted filesystems
 * list.
 */
mntcmp(struct mntent* a, struct mntent* b)
{
    return strcmp(a->mnt_dir, b->mnt_dir);
} /* mntcmp */


/*
 * doit() loads up the mounted filesystems list and either prints out
 *        information on selected filesystems or all the filesystems
 *        that are currently mounted
 */
doit()
{
    FILE* f;
    struct statfs fsinfo;
    struct mntent* mnt;
    struct mntent *mntpoints;
    int nrmnt = 0;
    int x, y, lastmatch;
    
    mntpoints = malloc(1);

    /* first load up the list of mounted filesystems
     * We prefer MOUNTED to PROCMNT because PROCMNT calls the root
     * filesystem /dev/boot, which doesn't actually exist on the
     * Mastodon systems that I've been developing this program on.
     */
    if ((f = setmntent(MOUNTED, "r")) == 0)
	if ((f = setmntent(PROCMNT, "r")) == 0) {
	    fprintf(stderr, "%s: can't open " MOUNTED " or " PROCMNT "\n", pgm);
	    exit(1);
	}
    while (mnt = getmntent(f)) {
	mntpoints = realloc(mntpoints, (2+nrmnt) * sizeof mntpoints[0]);
	mntpoints[nrmnt].mnt_fsname = strdup(mnt->mnt_fsname);
	mntpoints[nrmnt].mnt_dir    = strdup(mnt->mnt_dir);
	mntpoints[nrmnt].mnt_type   = strdup(mnt->mnt_type);
	nrmnt++;
    }
    endmntent(f);

    if (nrfilesys == 0) {
	/* if we didn't specify any filesystems to display info on,
	 * we'll just walk through the mnt list and print out information
	 * on as much stuff as we can
	 */
	for (x = 0; x < nrmnt; x++)
	    printmount(&mntpoints[x], mntpoints[x].mnt_fsname);
    }
    else {
	/* since we've specified filesystems, we need to do some
	 * work to prepare for it.  We may be passed a directory
	 * name, and so we'll have to find the best fit of that
	 * directory name to the mounted filesystems.
	 *
	 * So we sort the list of mountpoints so that the filesystems
	 * are sorted in directory order, like so:
	 *               /
	 *               /usr
	 *               /usr/bin
	 *               /home
	 *               /cdrom/0
	 *               /cdrom/1
	 *
	 * and then, for each specified directory, we walk up this
	 * list comparing the mountpoint to the beginning of the
	 * directory;  we keep a record of the last match made, because
	 * in most cases we don't want to use the first match (/, for
	 * example, matches everything, but this isn't helpful if you're
	 * trying to look at /home.)
	 *
	 * As a little something extra, specifying the device where
	 * the directory is mounted also needs to work, so the fs
	 * devices need to be checked as well.  This is done before
	 * the paths are checked.
	 */
	 
	qsort(mntpoints, nrmnt, sizeof mntpoints[0], mntcmp);
	
	for (x = 0; x < nrfilesys; x++) {
	    if (strncmp(filesys[x], "/dev/", 5) == 0) {
		/* device node, so the user is probably expecting
		 * the df info for the filesystem mounted on that
		 * device
		 */

		for (y=0; y < nrmnt; y++)
		    if (strcmp(filesys[x], mntpoints[y].mnt_fsname) == 0) {
			printmount(&mntpoints[y], filesys[x]);
			goto more;
		    }
	    }
	    else {
		int lastmatch = -1;
		int len;
		char *current = getcwd(0,0);
		char *dir = 0;

		if (current) {
		    /* the path specified may be relative;  chdir() to
		     * it, then pick up the working directory using the
		     * GNU-hacked `getcwd' call. On non-glibc systems,
		     * getcwd() needs to have the path passed in.
		     */
		    if (chdir(filesys[x]) != 0) {
			free(current);
			goto fail;
		    }
		    dir = getcwd(0,0);
		    chdir(current);
		    free(current);
		}
		if (dir == 0)
		    dir = strdup(filesys[x]);

		for (y = 0; y < nrmnt; ++y) {
		    len = strlen(mntpoints[y].mnt_dir);
		    if (len > strlen(dir))
			continue;
		    if (strncmp(mntpoints[y].mnt_dir, dir, len) == 0)
			lastmatch = y;
		}

		if (lastmatch >= 0) {
		    printmount(&mntpoints[lastmatch], dir);
		    free(dir);
		    goto more;
		}
		free(dir);
	    }
    fail:
	    fprintf(stderr, "%s: unknown path <%s>\n",
			pgm, filesys[x]);
    more:   ;
	}
    }
} /* doit */


/*
 * options that can be passed in to df
 */
struct x_option options[] = {
    { 'a', 'a', "all", 0,		"show filesystems that have 0 blocks" },
    { 'b', 'b', "bytes", 0,		"print sizes in bytes" },
    { 'P', 'P', "posix", 0,		"print sizes in 512 byte blocks (default)" },
    { 'k', 'k', "kilobytes", 0,		"print sizes in kilobytes" },
    { 'm', 'm', "megabytes", 0, 	"print sizes in megabytes" },
    { 'h', 'h', "human-readable", 0,	"print sizes in a more human readable form\n"
					"(eg: 12K, 34M, 5.6G)" },
    { 'h', 'H', 0, 0, 0 },
    { 'i', 'i', "inodes", 0,		"print inode usage, not space usage" },
    { 'l', 'l', "local", 0,		"show local filesystems only" },
    { 'T', 'T', "print-type", 0,	"print the type of each filesystem" },
    { 't', 't', "type", "TYPE",		"show filesystems of type TYPE" },
    { 'x', 'x', "exclude", "TYPE",	"exclude filesystems of type TYPE" },
    { '?', '?', "help", 0, 		"show this help message" },
};
#define NROPTIONS	(sizeof options/sizeof options[0])


/*
 * spit out a usage message, then quit
 */
static void
usage(int is_an_error)
{
    char iob[1024];

    setbuffer(stderr, iob, sizeof iob);
    fprintf(stderr, "usage: %s [options]\n", pgm);
    showopts(stderr, NROPTIONS, options);
    setbuf(stderr, NULL);

    exit (is_an_error ? 1 : 0);
} /* usage */



/* 
 * df, in the flesh
 */
int
main(int argc, char **argv)
{
    int opt;

    pgm = basename(argv[0]);
    
    exclusions = malloc(1);
    inclusions = malloc(1);


    x_opterr = 1;
    while ((opt = x_getopt(argc, argv, NROPTIONS, options)) != EOF)
	switch (opt) {
	case 'b': display = BYTES;					break;
	case 'P': display = POSIX;					break;
	case 'k': display = KILOBYTES;					break;
	case 'm': display = MEGABYTES;					break;
	case 'H':
	case 'h': display = HUMAN;					break;
	case 'i': show_inodes = 1;					break;
	case 'l': local_fs_only = 1;					break;
	case 'a': show_zero_length = 1;					break;
	case 't': addToList(x_optarg, &inclusions, &nrinclusions);	break;
	case 'v': break;
	case 'x': addToList(x_optarg, &exclusions, &nrexclusions);	break;
	case 'T': show_fs_type = 1;					break;
	
	default:  usage(opt == '?');	/* exits */
	}

    argc -= x_optind;
    argv += x_optind;

    if (argc > 0) {
	if (nrinclusions || nrexclusions)
	    fprintf(stderr, "%s: warning: `x' and `t' options conflict with"
			    " filesystem arguments\n", pgm);
    }

    filesys = argv;
    nrfilesys = argc;

    doit();
    exit(0);
} /* df */
