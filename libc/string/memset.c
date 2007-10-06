#include <string.h>


void *
memset(void *dest, int c, size_t siz)
{
    if ( dest && siz )
	asm("cld\n"
	   " rep\n"
	   " stosb"
	   : /* this space intentionally left blank */
	   : "c" (siz), "a" (c), "D" (dest)
	   : "%ecx", "%edi" );

    return dest;
}


#if TEST
main()
{
    char dest[20];

    if (dest != memset(dest, '!', sizeof dest))
	printf("dest is != memset(dest)\n");

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);

    if (dest != memset(dest, '?', sizeof dest/2))
	printf("dest is != memset(dest)\n");

    printf("dest is now \"%.*s\"\n", sizeof dest, dest);
}
#endif
