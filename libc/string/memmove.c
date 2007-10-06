#include <string.h>

/* copy memory nondestructively;
 *      if dest overlaps tail of src; copy backwards,
 *      otherwise copy forwards
 */

void *
memmove(void *dest, const void *src, size_t siz)
{
    char *res = dest;

    if ( !(src && dest && siz) ) return dest;

    if ((src < dest) && (dest < src+siz)) {
	src += siz-1;
	dest += siz-1;
	asm("std\n"
	   " rep\n"
	   " movsb"
	    : /*this space intentionally left blank*/
	    : "S"(src), "D"(dest), "c"(siz)
	    : "%ecx", "%edi" );
    }
    else {
	asm("cld\n"
	   " rep\n"
	   " movsb"
	    : /*this space intentionally left blank*/
	    : "S"(src), "D"(dest), "c"(siz)
	    : "%ecx", "%edi" );
    }
    return res;
}


#if TEST
main()
{
    char dest[24];

    memmove(dest, "a twenty-char string....", sizeof dest);

    printf("1: dest    =\"%.*s\"\n", sizeof dest, dest);

    memmove(dest+4,dest,sizeof dest-4);

    printf("2: dest>>4 = \"%.4s[%.*s]\"\n", dest, sizeof dest-4, dest+4);

    memmove(dest, dest+4, sizeof dest-4);

    printf("3: dest<<4 = \"[%.*s]%.4s\"\n", sizeof dest-4, dest, dest+sizeof dest-4);
}
#endif
