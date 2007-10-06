#include <string.h>

int
memcmp(const void *s1, const void *s2, size_t size)
{
    register res;

    if ( !size ) return 0;

    if ( !s1 ) return s2 ? -1 : 0;
    if ( !s2 ) return 1;

    asm("cld\n"
       " repe\n"
       " cmpsb\n"
       " je 1f\n"
       " sbbl %%eax,%%eax\n"
       " or $1,%%al\n"
       "1:"
	: "=a" (res)
	: "a" (0), "S" (s1), "D" (s2), "c" (size)
	: "%eax" );

    return res;
}


#if TEST
test(char *a, char *b, int size)
{
    int res = memcmp(a, b, size);

    printf("memcmp(\"%s\", \"%s\", %d) = %d\n", a ? a : "null",
						b ? b : "null", size, res);
}
main()
{

    test("aaaa","aaaa",4);
    test("aaaa","aaab", 4);
    test("aaac","aaab", 4);
    test("abac", "aaab", 4);
    test("a", "a", 1);
    test("b", "a", 1);
    test(NULL,"a", 1);
    test("b",NULL, 1);
    test(NULL,NULL, 1);
    test("a","b", 0);
}
#endif
