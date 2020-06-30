#include "config.h"

#include <stdio.h>
#include <utmp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "basis/options.h"

int headings = 0;
int mememe = 0;
int quick = 0;
int terse = 0;
int mesg = 0;
int activity = 0;
time_t now;

char *pgm = "who";

#define SAME(a,b)	(strcasecmp(a,b) == 0)


struct x_option opts[] = {
    { 'H', 'H', 0, 0, "display headers" },
    { 'm', 'm', 0, 0, "who am i?" },
    { 'q', 'q', 0, 0, "only display user names and # of users" },
    { 's', 's', 0, 0, "don't display # of users with -q" },
    { 'T', 'T', 0, 0, "display mesg settings" },
    { 'u', 'u', 0, 0, "display idle times" },
    { '?', '?', "help", 0, "show this help message" },
} ;

#define NROPTS (sizeof opts / sizeof opts[0])



void
usage(int fail)
{
    char iob[1024];

    setbuffer(stderr, iob, sizeof iob);
    fprintf(stderr, "usage: %s [-HmqsTu?] [am I] [file]\n", pgm);
    showopts(stderr, NROPTS, opts);
    setbuf(stderr, NULL);
    exit(fail ? 1 : 0);
}

void
header(struct utmp *ut)
{
    if (quick)
	return;

    printf("%-*s ", sizeof ut->ut_user, "NAME");
    if (mesg)
	printf("S ");
    printf("%-*s ", 8, "LINE");
    printf("%-12s ", "TIME");
    if (activity)
	printf("%-6s", "IDLE");
    printf("FROM\n");
}

void
printutmp(struct utmp *ut)
{
    char datestring[80];
    struct stat sb;
    struct tm *tm;
    int gotstats = 0;

    if (quick) {
	printf("%.*s ", sizeof ut->ut_user, ut->ut_user);
	return;
    }
    printf("%-*.*s ", sizeof ut->ut_user, sizeof ut->ut_user, ut->ut_user);

    if (mesg||activity) {
	char fulldevpath[5+sizeof ut->ut_line + 1];

	strcpy(fulldevpath, "/dev/");
	strncat(fulldevpath, ut->ut_line, sizeof ut->ut_line);
	fulldevpath[5+sizeof ut->ut_line] = 0;

	gotstats =  (stat(fulldevpath, &sb) == 0);
    }

    if (mesg)
	printf("%c ", gotstats ? ( (sb.st_mode & S_IWOTH) ? '+' : '-' ) : '?' );

    printf("%-*.*s ", 8, 8, ut->ut_line);

    tm = localtime( &ut->ut_time );
    if (ut->ut_time > now-(60*60*24*182))
	strftime(datestring, sizeof datestring, "%b %e %H:%M", tm);
    else
	strftime(datestring, sizeof datestring, "%b %e, %Y", tm);
    printf("%12s ", datestring);

    if (activity) {
	if ( (sb.st_atime > now - 60) || !gotstats )
	    printf("      ");
	else {
	    int hours, minutes;
	    minutes = (now-sb.st_atime) / 60;
	    hours = minutes/60;
	    if (hours < 100)
		printf("%02d:%02d ", hours, minutes%60);
	    else
		printf(" old  ");
	}
    }
    if (ut->ut_host[0])
	printf("(%.*s)", sizeof ut->ut_host, ut->ut_host);

    putchar('\n');
}


void
whoami()
{
    struct utmp key, *ut;
    char *tty = ttyname(fileno(stdin));
    int rc = 0;

    if ( tty == 0 || strncmp(tty, "/dev/", 5) != 0 )
	exit(1);

    tty += 5;
    bzero(&key, sizeof key);

    strncpy(key.ut_line, tty, sizeof key.ut_line);

    setutent();
    if ( (ut = getutline(&key)) == 0 )
	exit(1);
    if (headings) header(ut);
    printutmp(ut);
    if (quick) putchar('\n');
    endutent();
}

int
alive(pid_t pid)
{
    return (kill(pid,0) == 0 || errno == EPERM);
}


void
who()
{
    struct utmp *ut;
    int count=0;

    if (headings) header(ut);
    setutent();
    while (ut = getutent())
	if ( (ut->ut_type == USER_PROCESS) && alive(ut->ut_pid) ) {
	    printutmp(ut);
	    count++;
	}
    if (quick) {
	if (!terse) printf("\n# users = %d", count);
	putchar('\n');
    }
    endutent();
    exit(0);
}


main(argc,argv)
int argc;
char **argv;
{
    int opt;

    pgm = basename(argv[0]);


    if ( SAME(pgm, "whoami") ) {
	mememe=quick=1;
	whoami();
	exit(0);
    }
    else if ( SAME(pgm, "users") ) {
	terse=quick=1;
	who();
	exit(1);
    }

    x_opterr = 1;
    while ( (opt=x_getopt(argc,argv, NROPTS, opts)) != EOF ) {
	switch (opt) {
	case 'H' : headings = 1; break;
	case 'm' : mememe = 1; break;
	case 'q' : quick = 1; break;
	case 's' : terse = 1; break;
	case 'T' : mesg = 1; break;
	case 'u' : activity = 1; break;
	case '?' :
	default  : usage(opt != '?');
	}
    }

    argc -= x_optind;
    argv += x_optind;

    if ( (argc >= 2) && SAME(argv[0], "am") && SAME(argv[1], "I") ) {
	mememe = 1;
	argc -= 2;
	argv += 2;
    }

    if ( argc == 1 )
	utmpname(argv[0]);
    else if ( argc > 1 )
	usage(1);

    time(&now);
    if (mememe)
	whoami();
    else
	who();
    exit(0);
}
