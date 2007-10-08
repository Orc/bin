#include <string.h>

char*
strcpy(char* dest, const char* src)
{

    /*if ( src && dest )*/
	asm("cld\n"
	   "1:lodsb\n"
	   "  stosb\n"
	   "  orb %%al,%%al\n"
	   "  jnz 1b"
	    : /* this space intentionally left blank */
	    : "S"(src), "D"(dest) );
    return dest;
}


#if TEST

void
test(char *dest, char *src, size_t siz)
{
    char *res;


    if (dest) bzero(dest, siz);

    printf("strcpy(%x,\"%s\") = ", dest, src ? src : "null");

    res = strcpy(dest,src);

    printf("\"%s\"\n", res);

}

main()
{
    char dest[20];
    char *res;


    test(dest, "a twenty-char string", sizeof dest);
}
#endif
