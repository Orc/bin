#include <string.h>
#include <ctype.h>

int
strcasecmp(const char*s1, const char*s2)
{
    while (*s1 && (toupper(*s1) == toupper(*s2)) )
	++s1, ++s2;

    return toupper(*s1)-toupper(*s2);
}


#if TEST

void
test(char *a, char *b)
{
    printf("strcasecmp(\"%s\",\"%s\") = %d\n", a, b, strcasecmp(a,b));
}


main()
{
    test("a", "a");
    test("a", "b");
    test("b", "a");
    test("aaa", "aaa");
    test("aAa", "aaa");
    test("aaa", "aab");
    test("aab", "aaa");
    test("aaa", "aba");
    test("aba", "aaa");
    test("aaaaa", "aaa");
    test("aAaAa", "aaa");
    test("aaaaa", "aab");
    test("aabaa", "aaa");
    test("aaa", "aaaaa");
    test("aaa", "aabaa");
    test("aab", "aaaaa");
    test("aa\277", "aa\276");
}

#endif
