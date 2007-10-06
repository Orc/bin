#include <string.h>

void*
memchr(const void* s, int c, size_t siz)
{
    void *ret;


    if ( s && siz ) {
	asm("cld\n"
	   " repne\n"
	   " scasb\n"
	   " je 1f\n"
	   " movl $1,%0\n"
	   "1:decl %0"
	   : "=D" (ret)
	   : "a" (c), "D" (s), "c" (siz));
	return ret;
    }
    return 0;
}


#if TEST
void
test(char *a, char c, int siz)
{
    char *res = memchr(a,c,siz);

    printf("memchr(\"%s\",'%c',%d) = \"%s\"\n", a ? a : "null", c, siz,
						res ? res : "null" );
}

main()
{
    test("abcd---ea", 'a', 4);
    test("abcd---eb", 'b', 4);
    test("abcd---ec", 'c', 4);
    test("abcd---ed", 'd', 4);

    test("abcd---ed", 'd', 3);
    test("abcd---ed", 'f', 4);
    test(NULL, 'a', 4);
    test("abcd---ed", 'f', 0);
}
#endif
