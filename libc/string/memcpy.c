#include <string.h>

void*
memcpy(void* dest, const void* src, size_t siz)
{

    if ( /*dest && src &&*/ siz ) 
	asm("cld\n"
	   " rep\n"
	   " movsb"
	    : /*this space intentionally left blank*/
	    : "S"(src), "D"(dest), "c"(siz) );

    return dest;
}


#if TEST
main()
{
    char dest[24];
    char *res;

    /* good result */
    memcpy(dest, "a twenty-char string....", sizeof dest);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    /* overlapping copy */

    memcpy(dest+4, dest, sizeof dest-4);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    /* overlapping copy 2 */

    memcpy(dest, "a twenty-char string....", sizeof dest);

    memcpy(dest, dest+4, sizeof dest-4);

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

}
#endif
