#include <string.h>

/* copy memory nondestructively;
 *      if dest overlaps tail of src; copy backwards,
 *      otherwise copy forwards
 */

void *
memmove(void *dest, const void *src, size_t siz)
{
    if ( /*src && dest &&*/ siz ) {
	if ((src < dest) && (dest < src+siz)) {
	    asm("std\n"
	       " rep\n"
	       " movsb"
		: /*this space intentionally left blank*/
		: "S"(src+siz-1), "D"(dest+siz-1), "c"(siz)
		: "%ecx", "%esi", "%edi" );
	}
	else {
	    asm("cld\n"
	       " rep\n"
	       " movsb"
		: /*this space intentionally left blank*/
		: "S"(src), "D"(dest), "c"(siz)
		: "%ecx", "%esi", "%edi" );
	}
    }
    return dest;
}


#if TEST
main()
{
    char dest[24];

    memmove(dest, "a twenty-char string....", sizeof dest);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    memmove(dest+4,dest,sizeof dest-4);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    memmove(dest, "a twenty-char string....", sizeof dest);

    memmove(dest, dest+4, sizeof dest-4);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);
}
#endif
