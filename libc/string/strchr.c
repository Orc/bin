#include <string.h>

char*
strchr(const char* target, int ch)
{
    register void *res;

    asm( "cld\n"
       "1:lodsb\n"
       "  orb %%al, %%al\n"
       "  je 2f\n"
       "  cmpb %%al, %%bl\n"
       "  jne 1b\n"
       "2:"
	: "=a" (ch), "=S" (res)
	: "S" (target), "b" (ch) );
    return ch ? (res-1) : NULL;
}

#if TEST
void
test(char *target, int ch)
{
    char *res;

    res = strchr(target,ch);

    printf("strchr(\"%s\",'%c') = ", target, ch);
    if (res) printf("\"%s\"\n", res);
    else printf("null\n");
}

main()
{
    test("I am a pirate", 'p');
    test("I am a pirate", 'q');
    test("I am a pirate", 'I');
}
#endif
