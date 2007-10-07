#include <string.h>

int
strcmp(const char*s1, const char*s2)
{
    int cmp;

    asm( "cld\n"
       "1:lodsb\n"
       "  scasb\n"
       "  jne 2f\n"
       "  orb %%al,%%al\n"
       "  jne 1b\n"
       "  jmp 3f\n"
       "2:sbbl %%eax,%%eax\n"
       "  orb $1, %%al\n"
       "3:"
       : "=a" (cmp)
       : "S" (s1), "D" (s2), "a" (0)
       : "%esi", "%edi" );
    return cmp;
}


#if TEST

void
test(char *a, char *b)
{
    printf("strcmp(\"%s\",\"%s\") = %d\n", a, b, strcmp(a,b));
}


main()
{
    test("a", "a");
    test("a", "b");
    test("b", "a");
    test("aaa", "aaa");
    test("aaa", "aab");
    test("aab", "aaa");
    test("aaa", "aba");
    test("aba", "aaa");
    test("aaaaa", "aaa");
    test("aaaaa", "aab");
    test("aabaa", "aaa");
    test("aaa", "aaaaa");
    test("aaa", "aabaa");
    test("aab", "aaaaa");
}

#endif
