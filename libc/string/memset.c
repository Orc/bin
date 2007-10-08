#include <string.h>


void *
memset(void *dest, int c, size_t siz)
{
    if ( /*dest &&*/ siz )
	asm("cld\n"
	   " rep\n"
	   " stosb"
	   : /* this space intentionally left blank */
	   : "c" (siz), "a" (c), "D" (dest) );

    return dest;
}


#if TEST
main()
{
    char dest[20];

    if (dest != memset(dest, '!', sizeof dest))
	printf("dest is != memset(dest)\n");

    printf("memset(dest,'!',%d) = \"%.*s\"\n", sizeof dest, sizeof dest, dest);

    if (dest != memset(dest, '?', sizeof dest/2))
	printf("dest is != memset(dest)\n");

    printf("memset(dest,'?',%d) = \"%.*s\"\n", sizeof dest/2, sizeof dest, dest);
}
#endif
