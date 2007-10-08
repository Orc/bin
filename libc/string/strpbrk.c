#include <string.h>

char *
strpbrk(const char* target, const char* sset)
{
    register ssiz;
    int bingo;

    /* stash the sset size in a safe place */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (ssiz)
       : "D" (sset), "a" (0), "c" (-1L) );

    for (; *target; ++target) {
	/* scan each byte in target for a sset match */
	asm( "cld\n"
	   "  repne\n"
	   "  scasb\n"
	   "  je 1f\n"
	   "  xorl %%eax,%%eax\n"
	   "1:"
	   : "=a" (bingo)
	   : "a" (*target), "D" (sset), "c" (ssiz) );
	if (bingo) return (char*)target;
    }
    return 0;
}


#if TEST

void
test(char *target, char *sset)
{
    char *res = strpbrk(target, sset);

    printf("strpbrk(\"%s\",\"%s\") = %s\n", target, sset, res);
}

main()
{
    test("abcdef", "ghi");
    test("abcdef", "def");
    test("abcdef", "aab");
}

#endif
