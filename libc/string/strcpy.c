#include <string.h>

void*
strcpy(char* dest, const char* src, size_t siz)
{

    if (!dest) return 0;

    if (src && (siz > 0)) {
	asm("cld\n"
	   " rep\n"
	   " movsb"
	    : /*this space intentionally left blank*/
	    : "S"(src), "D"(dest), "c"(siz));
    }
    return dest;
}


#if TEST
main()
{
    char dest[20];
    char *res;

    char *dest2 = dest + 4;

    /* good result */
    strcpy(dest, "a twenty-char string", sizeof dest);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    /* overlapping copy */

    strcpy(dest2, dest, 4);

    printf("dest is now \"%.*s[%.*s]%.*s\"\n",
	    4, dest,
	    4, dest+4,
	    sizeof dest - 8, dest+8);

    /* overlapping copy 2 */

    strcpy(dest, "a twenty-char string", sizeof dest);

    strcpy(dest, dest2, 4);

    printf("dest is now \"%.*s[%.*s]%.*s\"\n",
	    4, dest,
	    4, dest+4,
	    sizeof dest - 8, dest+8);

}
#endif
