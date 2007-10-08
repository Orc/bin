#include <string.h>

char *
strcat(char* dest, const char* src)
{
    /* skid to the end of the string, then strcpy the new string in */
    asm( "cld\n"
       "  repne\n"
       "  scasb\n"
       "  dec %%edi\n"
       "1:lodsb\n"
       "  stosb\n"
       "  orb %%al,%%al\n"
       "  jnz 1b"
       : /* this space intentionally left blank */
       : "D" (dest), "S" (src), "a" (0), "c" (-1L) );
    return dest;
}


#if TEST

#include <stdio.h>
#include <stdlib.h>

test(char *dest, char *src)
{
    char *res;
    char *buffer = alloca(strlen(dest) + strlen(src) + 10);
    strcpy(buffer, dest);
    fprintf(stderr, "strcat(\"%s\", \"%s\") = ",
		    dest ? dest : "null",
		    src ? src : "null");

    if ( res = strcat(buffer,src) ) {
	if (res != buffer)
	    fprintf(stderr,"-oops-");
	fprintf(stderr, "\"%s\"\n", res);
    }
    else
	fprintf(stderr,"null?\n");
}


main()
{
    test("hello,", " sailor");
}

#endif
