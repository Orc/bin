#include <string.h>

char*
strstr(const char* haystack, const char* needle)
{
    register ok, hssiz, ndsiz;

    /* get length of haystack, needle */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (hssiz)
       : "D" (haystack), "a" (0), "c" (-1L) );
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (ndsiz)
       : "D" (needle), "a" (0), "c" (-1L) );


    if (hssiz) {
	for ( ;hssiz-- >= ndsiz; ++haystack ) {
	    asm( "cld\n"
	       "  repe\n"
	       "  cmpsb\n"
	       "  je 1f\n"
	       "xorl %%eax,%%eax\n"
	       "1:"
		: "=a" (ok)
		: "a" (1), "S" (haystack), "D" (needle), "c" (ndsiz) );
	    if (ok) return (char*)haystack;
	}
    }
    return 0;
}

#if TEST

void
test(char *hay, char *needle)
{
    char *res = strstr(hay,needle);

    printf("strstr(\"%s\",\"%s\") = \"%s\"\n", hay, needle, res);
}

main()
{
    test("aabbcc", "bb");
    test("aabbccaabbcc", "bb");
    test("aabbcc", "dd");
    test("aabbcc", "aabbcc");
    test("aabbcc", "aabbccdd");
    test("aabbcc", "qqrrsstt");
}

#endif
