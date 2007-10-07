#include <string.h>

char*
stpcpy(char* dest, const char* src)
{

    asm("cld\n"
       "1:lodsb\n"
       "  stosb\n"
       "  orb %%al,%%al\n"
       "  jnz 1b\n"
       "  dec %%edi"
	: "=D" (dest)
	: "S"(src), "D"(dest)
	: "%eax", "%esi" );
    return dest;
}


#if TEST

void
test(char *dest, char *src)
{
    char *res;

    printf("stpcpy(%x,\"%s\") = ", dest, src ? src : "null");
    printf("(predict %x)", dest + strlen(src));

    res = stpcpy(dest,src);
    printf("%x (%d)", res, *res);
    if (strcmp(src,dest) == 0)
	puts("ok");
    else
	puts(":-(");
}

main()
{
    char dest[200];
    char *res;


    test(dest, "a twenty-char string");
}
#endif
