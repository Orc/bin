#ifndef __OPTIONS_D
#define __OPTIONS_D

struct x_option {
    int  optval;	/* option value */
    char flag;		/* single character flag or null */
    char *name;		/* option name or null */
    char *has_argument;	/* does it need a following argument (and what that
			 * argument is, for help messages) 
			 */
    char *description;	/* what this options is for (for help messages) */
};

extern int x_optind;
extern int x_opterr;
extern char *x_optarg;

int
x_getopt(int argc, char **argv, int optcount, struct x_option *opts);

#endif/*__OPTIONS_D*/
