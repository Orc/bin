#include <time.h>
#include <ctype.h>

main()
{
    time_t now;
    struct tm *t;
    char fmt[30];
    char bfr[200];
    int c;

    time(&now);
    if ( time(&now) == (time_t)-1 || (t = localtime(&now)) == 0 ) {
	perror("time &| localtime");
	exit(1);
    }

    for (c='A'; c <= 'Z'; c++) {
	snprintf(fmt, sizeof fmt, "%c%c", '%', c);
	if (strftime(bfr, sizeof bfr, fmt, t) == 0 || ((strlen(bfr) == 1 && bfr[0] == c)) )
	    printf("%26.26s", "");
	else
	    printf("%%%c := %-20.20s", c, bfr);

	snprintf(fmt, sizeof fmt, "%c%c", '%', tolower(c));
	if (strftime(bfr, sizeof bfr, fmt, t) == 0 || ((strlen(bfr) == 1 && bfr[0] == tolower(c))) )
	    putchar('\n');
	else
	    printf("%%%c := %s\n", tolower(c), bfr);
    }

    strftime(bfr, sizeof bfr, "%+", t);
    printf("%%+ := %s\n", bfr);
    strftime(bfr, sizeof bfr, "%%", t);
    printf("%%%% := %s\n", bfr);
}
