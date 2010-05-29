/*
 *   Copyright (c) 1998 David Parsons. All rights reserved.
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
 * Uname: get system information
 */
#include <stdio.h>
#include <basis/options.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <stdlib.h>

#define SHOW_OS		0x01
#define SHOW_NAME	0x02
#define	SHOW_RELEASE	0x04
#define SHOW_VERSION	0x08
#define SHOW_HARDWARE	0x10

#define SHOW_EVERYTHING	0xff
#define SHOW_DEFAULT	SHOW_NAME

/*
 * options that can be passed in to uname
 */
enum cmds { oALL=1, oOS, oMACH, oNODE, oREL, oVER, oUVER, oHELP, oPROC };

struct x_option options[] = {
    { oOS,  's', "operating-system", 0,	"Show operating system" },
    { oMACH,'m', "hardware", 0,		"Show hardware type" },
    { oNODE,'n', "nodename", 0,		"Show machine name" },
    { oVER, 'v', 0,          0,		"Show OS version" },
    { oREL, 'r', "release", 0,		"Show OS release" },
    { oALL, 'a', "all", 0,		"Show all the above information" },
    { oUVER, 0,  "version", 0,		"Show uname's version" },
    { oHELP,'?', "help", 0,		"Show this help message" },

    /* GNU compatability hack */
    { oPROC,'p', "processor", 0,	"Show processor type" },
};
#define NROPTIONS	(sizeof options/sizeof options[0])

char *pgm = "uname";


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


spit(char *info)
{
    static int spacing = 0;

    if (spacing)
	putchar(' ');
    else
	spacing++;
    fputs(info, stdout);
}

main(int argc, char ** argv)
{
    struct utsname info;
    int showme;
    int opt;
    
    x_opterr = 1;
    showme = 0;
    pgm = basename(argv[0]);

    while ((opt = x_getopt(argc, argv, NROPTIONS, options)) != EOF) {
	switch (opt) {
	case oOS:	showme |= SHOW_OS;		break;
	case oNODE:	showme |= SHOW_NAME;		break;
	case oREL:	showme |= SHOW_RELEASE;		break;
	case oVER:	showme |= SHOW_VERSION;		break;
	case oMACH:	showme |= SHOW_HARDWARE;	break;
	case oPROC:	showme |= SHOW_HARDWARE;	break;
	case oALL:	showme = SHOW_EVERYTHING;	break;
	case oUVER:	puts("uname $Revision$");	exit(0);
	case oHELP:
	default:
			usage(opt != oHELP);
	}
    }
    if (showme == 0)
	showme = SHOW_OS;

    if (uname(&info) < 0) {
	perror("uname");
	exit(1);
    }

    if (strcmp(info.machine, "i?86") == 0)
	strcpy(info.machine, "i386");

    if (showme & SHOW_OS)
	spit(info.sysname);
    if (showme & SHOW_NAME)
	spit(info.nodename);
    if (showme & SHOW_RELEASE)
	spit(info.release);
    if (showme & SHOW_VERSION)
	spit(info.version);
    if (showme & SHOW_HARDWARE)
	spit(info.machine);
    if ((showme & SHOW_EVERYTHING) == SHOW_EVERYTHING)
	putchar(' ');	/* compatability with FrameMaker.  Sigh. */
    if (showme)
	putchar('\n');
    exit(0);
} /* uname */
