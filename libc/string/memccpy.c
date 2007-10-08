#include <string.h>

/*
 * memccpy:  copy at most siz bytes from src to dest,
 *           stopping prematurely if we encounter ch.
 *           if we encountered ch, return a pointer to it (?),
 *           otherwise return NULL.
 */
void*
memccpy(void* dest,const void* src,int ch,size_t siz)
{
    register void *res;

    if ( /*dest && src &&*/ siz ) {
	asm("cld\n"
	   "1:lodsb\n"
	   " stosb\n"
	   " cmpb %%al, %%bl\n"
	   " je 2f\n"
	   " dec %%ecx\n"
	   " jne 1b\n"
	   "2:"
	    : "=D" (res), "=c" (siz)
	    : "S" (src), "D" (dest), "b" (ch), "c" (siz)
	    : "%eax" );
	if (siz) return res;
    }
    return NULL;
}

#if TEST
void
test(char *dest, char *src, int ch, size_t siz)
{
    char *res;

    bzero(dest,siz);

    res = memccpy(dest,src,ch,siz);

    printf("memccpy(dest,\"%s\",'%c',%d) = ", src?src:"null", ch, siz);
    if (res) printf("+%d", res-dest);
    else printf("null");
    printf(", dest = \"%s\"\n", dest);
}

main()
{
    char buffer[40];

    test(buffer, "I am a pirate", 'p', 14);
    test(buffer, "I am a pirate", 'q', 14);
    test(buffer, "I am a pirate", 'I', 14);
}
#endif
