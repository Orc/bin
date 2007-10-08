#include <string.h>

int
strncmp(const char* s1, const char* s2, size_t siz)
{
    int cmp;

    if ( !siz ) return 0;

    asm( "cld\n"
       "1:lodsb\n"
       "  scasb\n"
       "  jne 2f\n"
       "  orb %%al,%%al\n"
       "  je  3f\n"
       "  dec %%ecx\n"
       "  jne 1b\n"
       "  jmp 3f\n"
       "2:sbbl %%eax,%%eax\n"
       "  orb $1, %%al\n"
       "  jmp 4f\n"
       "3:xorl %%eax,%%eax\n"
       "4:"
       : "=a" (cmp)
       : "S" (s1), "D" (s2), "c" (siz) );
    return cmp;
}


#if TEST

void
test(char *a, char *b, int r1, int r2, int r3)
{
    int siz = strlen(a);
    int res;
    if (r1 != (res = strncmp(a,b,siz-1)))
	printf("strncmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz-1,res,r1);
    if (r2 != (res = strncmp(a,b,siz)))
	printf("strncmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz,res,r2);
    if (r3 != (res = strncmp(a,b,siz+1)))
	printf("strncmp(\"%s\",\"%s\",%d) = %d, not %d\n", a,b,siz+1,res,r3);
}


main()
{
    test("a", "a", 0, 0, 0);
    test("a", "b", 0, -1, -1);
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
