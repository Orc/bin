#include <string.h>
#include <stdlib.h>

char*
strdup(const char* src)
{
    char *res;
    int   siz;

    if ( !src ) return 0;

    /* get strlen(src)+1 (for trailing null) */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       : "=c" (siz)
       : "D" (src), "a" (0), "c" (-1L) );

#if TEST
    printf("in strdup: siz+1 = %d\n", siz);
#endif

    if ( res = malloc(siz) ) 
	/* allocate and copy all src + trailing null */
	asm("cld\n"
	   " rep\n"
	   " movsb"
	    : /*this space intentionally left blank*/
	    : "S"(src), "D"(res), "c"(siz) );
    return res;
}


#if TEST

void
test(char *src)
{
    char *res = strdup(src);

    printf("strdup(\"%s\") = \"%s\"\n", src ? src : "null", res ? res : "null");
}

main()
{
    test("hello, sailor");
    test(NULL);
    test("hello");
    test("boys");
}
#endif
