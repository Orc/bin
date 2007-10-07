#include <string.h>

char*
strstr(const char* haystack, const char* needle)
{
    register ok, hssiz, ndsiz;

    /* get length of haystack, needle */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (hssiz)
       : "D" (haystack), "a" (0), "c" (-1L)
       : "%edi");
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (ndsiz)
       : "D" (needle), "a" (0), "c" (-1L)
       : "%edi");


    if (hssiz) {
	for ( ;hssiz-- >= ndsiz; ++haystack ) {
	    asm("cld\n"
	       " repe\n"
	       " cmpsb\n"
	       " je 1f\n"
	       " sbbl %%eax,%%eax\n"
	       " or $1,%%al\n"
	       "1:"
		: "=a" (ok)
		: "a" (0), "S" (haystack), "D" (needle), "c" (ndsiz)
		: "%edi", "%esi", "%ecx" );
	    if (ok) return (char*)haystack;
	}
    }
    return 0;
}
