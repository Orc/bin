#include <string.h>

size_t
strlen(const char* s)
{
    register size_t siz;

    /*if ( !s ) return 0;*/

    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (siz)
       : "D" (s), "a" (0), "c" (-1L) );
    return siz;
}


#if TEST

test(char *s)
{
    printf("strlen(\"%s\") = %d\n", s?s:"null", strlen(s));
}


main()
{
    test("123456789");
    test("1");
}

#endif
