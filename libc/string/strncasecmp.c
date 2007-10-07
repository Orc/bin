#include <string.h>
#include <ctype.h>

int
strncasecmp(const char* s1, const char* s2, size_t siz)
{
    while (siz--) {
	if ( (*s1 == 0) || (toupper(*s1) != toupper(*s2)) )
	    return toupper(*s1)-toupper(*s2);
	++s1,++s2;
    }

    return 0;
}


#if TEST

void
test(char *a, char *b, int r1, int r2, int r3)
{
    int siz = strlen(a);
    int res;
    if (r1 != (res = strncasecmp(a,b,siz-1)))
	printf("strncasecmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz-1,res,r1);
    if (r2 != (res = strncasecmp(a,b,siz)))
	printf("strncasecmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz,res,r2);
    if (r3 != (res = strncasecmp(a,b,siz+1)))
	printf("strncasecmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz+1,res,r3);
}


main()
{
    test("a", "a", 0, 0, 0);
    test("A", "a", 0, 0, 0);
    test("a", "b", 0, -1, -1);
    test("A", "b", 0, -1, -1);
    test("a", "B", 0, -1, -1);
    test("b", "a", 0, 1, 1);
    test("aaa", "aaa", 0, 0, 0);
    test("aaa", "aab", 0, -1, -1);
    test("aab", "aaa", 0, 1, 1);
    test("aaa", "aba", -1,-1,-1);
    test("aba", "aaa", 1,1,1);
    test("aaaaa", "aaa",1,1,1);
    test("aaaaa", "aab",-1,-1,-1);
    test("aabaa", "aaa",1,1,1);
    test("aaa", "aaaaa",0,0,-1);
    test("aab", "aaaaa",0,1,1);
    test("aaa", "aabaa",0,-1,-1);
}

#endif
