#include <string.h>

char *
strncat(char* dest, const char* src, size_t siz)
{
    /* skid to the end of the string, then strncpy the new string in */
    if (siz) 
	asm( "cld\n"
	   "  repne\n"
	   "  scasb\n"
	   "  dec %%edi\n"
	   "1:lodsb\n"
	   "  stosb\n"
	   "  dec %%ebx\n"
	   "  jz 2f\n"
	   "  orb %%al,%%al\n"
	   "  jnz 1b\n"
	   "2:"
	   : /* this space intentionally left blank */
	   : "D" (dest), "S" (src), "a" (0), "b" (siz), "c" (-1L) );
    return dest;
}


#if TEST

#include <stdio.h>
#include <stdlib.h>

test(char *dest, char *src, size_t siz)
{
    char *res;
    int count;
    char *buffer = alloca(strlen(dest) + strlen(src) + 10 + siz);

    memset(buffer, '?', strlen(dest) + strlen(src) + 10 + siz);
    strcpy(buffer, dest);
    printf("strncat(\"%s\", \"%s\", %d) = ",
		    dest ? dest : "null",
		    src ? src : "null", siz);

    if ( res = strncat(buffer, src, siz) ) {
	if (res != buffer)
	    printf("-oops-");
	putchar('"');
	for (count=strlen(dest)+siz+1; *res && count; --count,++res)
	    if (*res >= ' ' && *res <= '~')
		putchar(*res);
	    else if (*res == 0)
		printf("\\0");
	    else
		printf("<%02x>", *res);
	putchar('"');
	putchar('\n');
    }
    else
	puts("null?");
}


main()
{
    test("hello,", " sailor", 80);
    test("hello,", " sailor", 4);
}

#endif
