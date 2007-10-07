#include <string.h>
#include <errno.h>


char*
strtok_r(char*carcass, const char* cset, char **context)
{
    char *ret;

    if (carcass == 0) carcass = *context;

    if (carcass == 0) { errno = EINVAL; return 0; }

    carcass += strspn(carcass, cset);

    if (*carcass == 0) { *context = 0; return 0; }

    ret = carcass;

    if (carcass = strpbrk(ret, cset)) {
	*carcass++ = 0;
	*context = carcass;
    }
    else
	*context = 0;
    return ret;
}

char*
strtok(char* carcass, const char* cset)
{
    static char *context = 0;

    return strtok_r(carcass, cset, &context);
}

#if TEST

void
test(char *carcass, char *cset)
{
    char *scratch = strdup(carcass);
    char *p;
    int count=0;
    char *context;

    printf("chop /%s/ by /%s/...\n", scratch, cset);
    for (p=strtok_r(scratch,cset,&context); p; p = strtok_r(0,cset,&context))
	printf("%d: %s\n", ++count,p);
}


main()
{
    char bfr[80];
    char *p;

    test("this is   \r\n      a test", " \r\n");

    strcpy(bfr, "/this/was/no/test/");

    p = strtok(bfr, "/");

    printf("regular strtok: p=%s\n", p);

    test("this is a beer", " ");

    printf("regular (next) strtok: p=%s\n", p=strtok(0, "/"));
}

#endif
