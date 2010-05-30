/*
 *   Copyright (c) 1999 David Parsons. All rights reserved.
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
 * id: what you'd expect
 */
#include "config.h"

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "basis/options.h"
#include <libgen.h>

/*
 * options to the program
 */
enum opt { REAL_ID=1, GROUPS, SUPP_GROUPS, NAMES, USER_ID, STD, HELP };

struct x_option options[] = {
    { REAL_ID,	'r', "real-id",		0,	"Print the real (not effective) user id" },
    { GROUPS,	'g', "group",		0,	"Print only the group" },
    { SUPP_GROUPS,'G', "supplemental", 	0,	"Print only supplemental groups" },
    { NAMES,	'n', "names", 		0,	"Print user/group names" },
    { USER_ID,	'u', "user-id", 	0,	"Print only the user-id" },
    { STD,      'S', "standard",	0,	"Use the standard id(name) format" },
    { HELP,	'?', "help",		0,	"Give this help message" },
};
#define NROPTIONS	(sizeof options/sizeof options[0])

static char *pgm = "id";

#define SHOW_UID	0x01
#define	SHOW_GROUP	0x02
#define SHOW_SUPP	0x04
#define SHOW_ALL	(SHOW_UID|SHOW_GROUP|SHOW_SUPP)


/*
 * mygetgroups() -- get the supplemental group list for
 *                  the given user id.
 */
int
mygetgroups(uid_t uid, gid_t grps[], int nr_grps)
{
    struct group *grp;
    struct passwd *pwd = getpwuid(uid);
    int x;
    int totgrp = 0;

    if (pwd == 0)
	return 0;

    setgrent();
    while ( (grp = getgrent()) != 0) {
#if 0
	if (grp->gr_gid == pwd->pw_gid)
	    continue;
#endif
	for (x=0; grp->gr_mem[x]; x++)
	    if (strcmp(grp->gr_mem[x], pwd->pw_name) == 0) {
		if (totgrp >= nr_grps)
		    return totgrp;
		grps[totgrp++] = grp->gr_gid;
		break;
	    }
    }
    return totgrp;
} /* mygetgroups */


void
usage(int rc)
{
    char iob[1024];

    setbuffer(stderr, iob, sizeof iob);

    fprintf(stderr, "usage: %s [options] [user]\n", pgm);
    showopts(stderr, NROPTIONS, options);
    setbuf(stderr, NULL);
    exit(rc);
}


/*
 * id, in mortal flesh
 */
main(int argc, char **argv)
{
    struct passwd *pwd;
    struct group *grp;
    enum {ID,WHOAMI} mode = ID;
    enum opt opt;
    int display_opt = 0;		/* what we wish to see */
    int std = 0;			/* show in standard format */
    int real_id = 0;			/* show real, not euid */
    int names = 0;			/* show names, not numbers */
    uid_t uid;				/* uid we want */
    gid_t gid;				/* gid we want */
#define NR_SGIDS	200
    gid_t sgids[NR_SGIDS];		/* supplemental groups */
    int nr_sgids;

    int x;
    char key;

    pgm = basename(argv[0]);

    if (strcasecmp(pgm, "whoami") == 0) {
	mode = WHOAMI;
	display_opt = SHOW_UID;
	names = 1;
    }
    else if (strcasecmp(pgm, "groups") == 0) {
	mode = WHOAMI;
	display_opt = SHOW_SUPP;
	names = 1;
    }
    else {
	x_opterr = 1;
	while ((opt = x_getopt(argc, argv, NROPTIONS, options)) != EOF) {
	    switch (opt) {
	    case REAL_ID:
			real_id = 1;
			break;
	    case NAMES:
			names = 1;
			break;
	    case GROUPS:
			display_opt |= SHOW_GROUP;
			break;
	    case SUPP_GROUPS:
			display_opt |= SHOW_SUPP;
			break;
	    case USER_ID:
			display_opt |= SHOW_UID;
			break;
	    case STD:
			std = 1;
			break;
	    default:
	    case HELP:
			usage((opt == HELP) ? 0 : 1);
	    }
	}
    }

    if (argc > x_optind) {
	if ( (pwd = getpwnam(argv[x_optind])) != 0) {
	    uid = pwd->pw_uid;
	    gid = pwd->pw_gid;
	}
	else {
	    fprintf(stderr, "%s: no user %s\n", pgm, argv[x_optind]);
	    exit(1);
	}
    }
    else {
	uid = real_id ? geteuid() : getuid();
	gid = real_id ? getegid() : getgid();
    }

    if (display_opt == 0) {
	display_opt = SHOW_ALL;
	std = 1;
    }

    key = 0;
    if (display_opt & SHOW_UID) {
	if (std || (display_opt & ~SHOW_UID))
	    fputs("uid=", stdout);
	
	if (std) {
	    printf("%d", uid);
	    if ( (pwd = getpwuid(uid)) != 0)
		printf("(%s)", pwd->pw_name);
	}
	else if (!names)
	    printf("%d", uid);
	else if ( (pwd = getpwuid(uid)) != 0)
	    fputs(pwd->pw_name, stdout);
	key = ' ';
    }
    if (display_opt & SHOW_GROUP) {
	if (key) putchar(key);

	if (std || (display_opt & ~SHOW_GROUP))
	    fputs("gid=", stdout);
	    
	if (std) {
	    printf("%d", gid);
	    if ( (grp = getgrgid(gid)) != 0)
		printf("(%s)", grp->gr_name);
	}
	else if (!names)
	    printf("%d", gid);
	else if ( (grp = getgrgid(gid)) != 0)
	    fputs(grp->gr_name, stdout);
	key = ' ';
    }
    if (display_opt & SHOW_SUPP) {
	nr_sgids = mygetgroups(uid, sgids, sizeof sgids);

	if (nr_sgids > 0) {
	    if (key) putchar(key);

	    if (std||(display_opt & ~SHOW_SUPP)) {
		printf("groups");
		key = '=';
	    }
	    else
		key = 0;

	    for (x=0; x < nr_sgids; x++,key=(std ? ',' : ' ') ) {
		if (key)
		    putchar(key);

		grp = getgrgid(sgids[x]);
		if (std) {
		    printf("%d", sgids[x]);
		    if (grp)
			printf("(%s)", grp->gr_name);
		}
		else if (names && grp)
		    fputs(grp->gr_name, stdout);
		else
		    printf("%d", sgids[x]);
	    }
	    key = ' ';
	}
    }
    if (key) putchar('\n');
    exit(0);
}
