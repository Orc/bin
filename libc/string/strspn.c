#include <string.h>

size_t
strspn(const char* target, const char* sset)
{
    register ssiz;
    register count;
    register found;

    /* stash the sset size in a safe place */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (ssiz)
       : "D" (sset), "a" (0), "c" (-1L) );

    for (count=0; *target; ++target,++count) {
	/* scan each byte in target for a sset match */
	asm("cld\n"
	   " repne\n"
	   " scasb\n"
	   " je 1f\n"
	   " movl $1,%%eax\n"
	   "1:decl %%eax"
	   : "=a" (found)
	   : "a" (*target), "D" (sset), "c" (ssiz) );
	if (!found) break;
    }
    return count;
}


#if TEST

void
test(char *target, char *sset)
{
    int count = strspn(target, sset);

    printf("strspn(\"%s\",\"%s\") = %d\n", target, sset, count);
}

main()
{
    test("abcdef", "ghi");
    test("abcdef", "def");
    test("abcdef", "aab");
}

#endif
