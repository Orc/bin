/*
 * date:  get or set the date & time
 *
 * usage: date [+format]
 *        date [CC]YYMMDDHHMM[.SS]
 */


#include <stdio.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <basis/options.h>
#include <string.h>

struct x_option opts[] = {
    { 'I', 'I', "psu", 0, "display information-superhighway time" },
    { 'u', 'u', "utc", 0, "display the UTC date" },
    { 'h', 'h', "help", 0, "print this help message" },
    { 'V', 'V', "version", 0, "display the version#" },
};
#define nropts	(sizeof opts / sizeof opts[0])

char *pgm = "date";
char version[] = "0.0";

int aol = 0;
int utc = 0;

void
printdf(char *format, struct tm *t)
{
    char thetimeisnow[1024];

    if (aol && ((t->tm_year > 93) || (t->tm_year == 93 && t->tm_mon > 8)) ) {
	struct tm tz0;

	time_t start, now;
	int septday;
	
	bzero(&tz0, sizeof tz0);	/* last day in august */
	tz0.tm_mon = 7;
	tz0.tm_mday = 31;
	tz0.tm_year = 93;

	start = utc ? timegm(&tz0) : mktime(&tz0);
	now = utc ? timegm(t) : mktime(t);

	septday = (now-start) / (24*60*60);	/* drifts on leap-seconds */

	t->tm_year = 93;
	t->tm_mon = 8;
	t->tm_mday = septday;
    }

    if (format == 0) format = "%a %b %d %R:%S %Z %Y";

    if (strftime(thetimeisnow, sizeof thetimeisnow, format, t) > 0)
	puts(thetimeisnow);
    else
	putchar('\n');
}

void
usage(int fail)
{
    char iob[1024];

    setbuffer(stderr, iob, sizeof iob);
    fprintf(stderr, "usage: %s [-uV] [+format]\n", pgm);
    fprintf(stderr, "       %s mmddhhmm[cc]yy[.ss]\n", pgm);
    showopts(stderr, nropts, opts);
    setbuf(stderr, NULL);
    exit(fail ? 1 : 0);
}

int
isdigits(char *s)
{
    for ( ; *s; ++s)
	if ( !isdigit(*s) ) return 0;
    return 1;
}


int
NUM(char **p, int size)
{
    int val = 0;

    while (size-- > 0) {
	val = (val * 10) + (**p) - '0';
	(*p)++;
    }
    return val;
}


int
setdate(char *date, struct tm *current)
{
    char *sec;
    int datelen;
    struct timeval tv;
    struct tm toset;
    
    if (sec = strchr(date, '.'))
	*sec++ = 0;


    /* validate date:  either 10 (MMDDHHMMYY) or 12 (MMDDHHMMCCYY) bytes,
     * plus, optionally, 3 bytes for seconds (.SS)
     */

    datelen = strlen(date);
    if ( (datelen != 10 && datelen != 12) || !isdigits(date) )
	usage(1);
    
    if ( sec && (strlen(sec) != 3 || *sec != '.' || !isdigits(1+sec)) )
	usage(1);

    toset = *current;

    toset.tm_mon  = NUM(&date,2)-1;
    toset.tm_mday = NUM(&date,2);
    toset.tm_hour = NUM(&date,2);
    toset.tm_min  = NUM(&date,2);

    if (datelen == 12)
	toset.tm_year = NUM(&date,4) - 1900;
    else
	toset.tm_year = NUM(&date,2) + (100 * (current->tm_year/100));

    if (sec) {
	++sec;
	toset.tm_sec = NUM(&sec,2);
    }
    else toset.tm_sec = 0;

    tv.tv_sec = utc ? timegm(&toset) : mktime(&toset);
    tv.tv_usec = 0;

    if (getuid() != 0) {
	fprintf(stderr, "%s: you need to be root to set the clock\n", pgm);
	return 1;
    }
    else if (settimeofday(&tv, 0) == -1) {
	fprintf(stderr, "%s: ", pgm);
	perror("settimeofday");
	return 1;
    }
    return 0;
}


main(argc,argv)
char **argv;
{
    time_t now;
    int rc = 0, opt;
    char *argfmt = 0;
    struct tm *current;

    pgm = basename(argv[0]);

    x_opterr = 1;

    while ( (opt = x_getopt(argc, argv, nropts, opts)) != EOF ) {
	switch (opt) {
	case 'I': aol++; break;
	case 'u': utc++; break;
	case 'V': fprintf(stderr, "%s %s\n", pgm, version); exit(0);
	case 'h':
	default : usage(opt != 'h');
	}
    }

    argc -= x_optind;
    argv += x_optind;

    if ( (argc > 0) && (argv[0][0] == '+') ) {
	argfmt = 1+(argv[0]);
	argc--;
	argv++;
    }

    time(&now);
    current = utc ? gmtime(&now) : localtime(&now);

    if (current == 0) {
	fprintf(stderr, "%s: ", pgm);
	perror( utc ? "gmtime" : "localtime" );
	exit(1);
    }
    
    if (argc) rc = setdate(argv[0], current);
    printdf(argfmt, current);
    exit(rc);
}
