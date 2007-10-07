#include <string.h>

size_t
strcspn(const char* target, const char* cset)
{
    register csiz;
    register count;
    register found;

    /* stash the cset size in a safe place */
    asm("cld\n"
       " repne\n"
       " scasb\n"
       " notl %0\n"
       " decl %0\n"
       : "=c" (csiz)
       : "D" (cset), "a" (0), "c" (-1L) );

    for (count=0; *target; ++target,++count) {
	/* scan each byte in target for a cset match */
	asm("cld\n"
	   " repne\n"
	   " scasb\n"
	   " je 1f\n"
	   " movl $1,%%eax\n"
	   "1:decl %%eax"
	   : "=a" (found)
	   : "a" (*target), "D" (cset), "c" (csiz)
	   : "%edi","%ecx");
	if (found) break;
    }
    return count;
}


#if TEST

void
test(char *target, char *cset)
{
    int count = strcspn(target, cset);

    printf("strcspn(\"%s\",\"%s\") = %d\n", target, cset, count);
}

main()
{
    test("abcdef", "ghi");
    test("abcdef", "def");
    test("abcdef", "aab");
}

#endif
