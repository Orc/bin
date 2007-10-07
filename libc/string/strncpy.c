#include <string.h>

char*
strncpy(char* dest, const char* src, size_t siz)
{
    if ( dest && src && siz )
	asm("cld\n"
	   "1:lodsb\n"
	   "  stosb\n"
	   "  dec %%ecx\n"
	   "  je 2f\n"
	   "  orb %%al, %%al\n"
	   "  jne 1b\n"
	   "  repne\n"
	   "  stosb\n"
	   "2:"
	    : /* this space intentionally left blank */
	    : "S" (src), "D" (dest), "c" (siz)
	    : "%ecx", "%edx", "%esi" );
    return dest;
}

#if TEST
#include <stdio.h>

void
test(char *dest, char *src, size_t siz)
{
    unsigned char *res;

    memset(dest,'?',siz+1);

    res = strncpy(dest,src,siz);

    printf("strncpy(dest, \"%s\", %d) = ", src ? src : "null", siz);
    if (res) {
	putchar('"');
	for (; siz-- > 0; ++res) {
	    if (*res >= ' ' && *res <= '~') putchar(*res);
	    else if (*res > '~')
		printf("<%02x>", *res);
	    else if (*res == 0)
		printf("\\0");
	    else printf("^%c", *res + 32);
	}
	putchar('"');
	if (*res == '?')
	    puts("ok!");
	else
	    puts("overflow");

    }
    else puts("null");
}

main()
{
    char buffer[40];

    test(buffer, "I am a pirate",  4);
    test(buffer, "I am a pirate", 14);
    test(buffer, "I am a pirate", 24);
}
#endif
