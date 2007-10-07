#include <string.h>

char*
strrchr(const char* str, int ch)
{
    char *res;

    /* skid to the end of the string, then search backwards */
    asm( "cld\n"
       "  repne\n"
       "  scasb\n"
       "  notl %%ecx\n"
       "  incl %%ecx\n"
       "  mov %%bl, %%al\n"
       "  std\n"
       "  repne\n"
       "  scasb\n"
       "  je 1f\n"
       "  movl $-1,%%edi\n"
       "1:incl %%edi"
       : "=D" (res)
       : "D" (str), "a" (0), "b" (ch), "c" (-1L)
       : "%eax", "%ecx" );
    return res;
}


#if TEST

void
test(char *s, int ch)
{
    char *res;

    printf("strrchr(\"%s\", ", s);
    if (ch >= ' ' && ch <= '~')
	printf("'%c'", ch);
    else if (ch == 0)
	printf("\0");
    else
	printf("<%02x>", ch);
    printf(") = %s\n", strrchr(s,ch));
}

main()
{
    char bfr[80];

    test("12/23/34/",  0 );
    test("12/23/34/", '/');
    test("12/23/34/", '3');
    test("12/23/34/", 'j');
    test("12/23/34/", '1');
    strcpy(bfr,"012/23/34/");
    test(bfr+1, '0');
}
#endif
