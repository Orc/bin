#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

typedef struct {
    char *i_name;
    short i_valid;
    short i_len;
    int   i_blocks;
    struct stat i;
#define i_ino	i.st_ino
#define i_mode	i.st_mode
#define i_links	i.st_nlink
#define i_uid	i.st_uid
#define i_gid	i.st_gid
#define i_size	i.st_size
#define i_atime	i.st_atime
#define i_ctime	i.st_ctime
#define i_mtime	i.st_mtime
} info;

typedef struct {
    info **index;
    info *files;
    int nrf;
    int szf;
} pack;


char *pgm = "ls";
int columns;		/* display in columns */
int all = 0;		/* show all files except . and .. */
int follow = 0;		/* follow links */
enum {NOT,BYNAME,BYTIME,BYSIZE,BYLEN} sortorder = BYNAME;
enum {CTIME,MTIME}       whichtime = CTIME;
int inodes = 0;		/* print inodes */
int links = 0;		/* print # links */
int directories = 1;	/* print contents of directories */
int dates = 0;		/* print dates */
int sizes = 0;		/* print sizes */
int blocks = 0;		/* print blocks */
int fancysizes = 0;	/* print k/m/g sizes */
int fancy = 0;		/* ornament the listing */
int owner = 0;		/* print owner/group */
int permissions = 0;	/* print permissions */
int pretty = 0;		/* print ? for nonprinting characters */
int showtarget = 0;	/* show softlink targets */
int reverse = 0;	/* reverse sort */
int needaheader;	/* need to print headers on directory listings? */
int psuvm = 0;		/* use internet time */
int dirblocks = 0;	/* print ``total %d'' line before directory listins  */
int needstat = 0;	/* depends on options */


unsigned *pp_stack = 0;
unsigned pp_top=0, pp_size = 0;

int
pushdir(char *dir)
{
    if (pp_top >= pp_size) {
	if (pp_size == 0) {
	    pp_size = pp_top + 200;
	    pp_stack = malloc(pp_size * sizeof pp_stack[0]);
	}
	else {
	    pp_size = pp_top * 10;
	    pp_stack = realloc(pp_stack, pp_size * sizeof pp_stack[0]);
	}
    }
    pp_stack[pp_top] = open(".", O_RDONLY);

    if ( chdir(dir) == -1 ) {
	close(pp_stack[pp_top]);
	return -1;
    }
    else pp_top++;
}


void
popdir()
{
    if (pp_top) {
	fchdir(pp_stack[--pp_top]);
	close(pp_stack[pp_top]);
    }
}


typedef struct {
    int key;
    char value[8];
} list;

typedef struct {
    list *data;
    int nrd;
    int szd;
} map;


map pass = { 0, 0, 0 };
map group= { 0, 0, 0 };

int
listcmp(void *c1, void*c2)
{
    list *a = (list*)c1;
    list *b = (list*)c2;

    return a->key - b->key;
}


char *
about(map *p, int key)
{
    list k, *ret;

    k.key = key;
    if ( ret = bsearch(&k, p->data, p->nrd, sizeof p->data[0], listcmp))
	return ret->value;
    return 0;
}


char*
remember(map *p, int key, char *name)
{
    int i;
    char *ret;

    if ( ret = about(p,key) )
	return ret; /* already remembered */

    if (p->nrd >= p->szd) {
	if (p->data) {
	    p->szd = (p->nrd * 10);
	    p->data = realloc(p->data, p->szd * sizeof p->data[0]);
	}
	else {
	    p->nrd = 0;
	    p->szd = 200;
	    p->data = realloc(p->data, p->szd * sizeof p->data[0]);
	}
    }
    p->data[p->nrd].key = key;
    strncpy(p->data[p->nrd].value, name, sizeof(p->data[0].value));

    /* always keep sorted for speedy lookups */
    qsort(p->data, p->nrd, sizeof p->data[0], listcmp);

    return name;
}


static int
skip(char *s)
{
    return !( all || (*s != '.') );
}

void
newer(pack *p)
{
    p->nrf = 0;
    p->szf = 200;
    p->files = malloc(p->szf * sizeof p->files[0]);
}


info
toinfo(struct dirent *de, info *p)
{
    p->i_name = strdup(de->d_name);
    p->i_len = strlen(de->d_name);
#if defined(DTTOIF)
    p->i_mode = DTTOIF(de->d_type);
#endif
    if (needstat) {
	p->i_valid = ((follow ? stat : lstat)(p->i_name, &p->i)) == 0;

	if (p->i_valid)
	    p->i_blocks = (p->i_size+511) / 512;
    }
    else p->i_valid = 0;
}


void
expand(pack *p)
{
    if (p->nrf >= p->szf) {
	p->szf = p->nrf * 10;
	p->files = realloc(p->files, p->szf * sizeof p->files[0]);
    }
}


void
another(struct dirent *de, pack *p)
{
    expand(p);
    toinfo(de, &(p->files[p->nrf++]) );
}

void
more(char *f, struct stat *fi, pack *p)
{
    expand(p);
    p->files[p->nrf].i_name = strdup(f);
    p->files[p->nrf].i_len = strlen(f);
    p->files[p->nrf].i_valid = 1;
    p->files[p->nrf].i_blocks = (fi->st_size+511)/512;
    p->files[p->nrf].i = *fi;
    p->nrf++;
}


time_t
date(info *p)
{
    switch (whichtime) {
    case CTIME: return p->i_ctime;
    default:    return p->i_mtime;
    }
}


int
fsort(void *c1, void *c2)
{
    int ret = 0;
    info *a = *((info**)c1);
    info *b = *((info**)c2);

    switch (sortorder) {
    case BYNAME:ret = strcasecmp(a->i_name, b->i_name); break;
    case BYTIME:if (date(a) < date(b))
		    ret = 1;
		else if (date(a) > date(b))
		    ret = -1;
		break;
    case BYSIZE:if (a->i_len < b->i_len)
		    ret = 0;
		else if (a->i_len > b->i_len)
		    ret = -1;
		break;
    case BYLEN: ret = a->i_len - b->i_len;
		break;
    }
    return reverse ? -ret : ret;
}


void
mkindex(pack *p)
{
    int i;

    p->index = malloc(p->nrf * sizeof p->index[0]);
    for (i=0; i<p->nrf; i++)
	p->index[i] = &(p->files[i]);
    if (sortorder != NOT)
	qsort( p->index, p->nrf, sizeof(info*), fsort);
}


int
package(char *path, pack *p)
{
    DIR *d;
    struct dirent *de;

    if (pushdir(path) == -1)
	return -1;

    if (d = opendir(".")) {
	newer(p);
	while (de = readdir(d)) {
	    if ( skip(de->d_name) )
		continue;

	    another(de, p);
	}
	closedir(d);
	mkindex(p);
	popdir();
	return p->nrf;
    }
    else return -1;
}


void
discard(pack *p)
{
    int i;
    if (p->index) free(p->index);
    if (p->files) {
	for (i=0; i<p->nrf; i++)
	    free(p->files[i].i_name);
	free(p->files);
    }
    p->nrf = p->szf = 0;
    p->files = 0;
    p->index = 0;
}

void
printperm(info *p)
{
    if (S_ISBLK(p->i_mode)) putchar('b');
    else if (S_ISCHR(p->i_mode)) putchar('c');
    else if (S_ISDIR(p->i_mode)) putchar('d');
    else if (S_ISLNK(p->i_mode)) putchar('l');
    else if (S_ISSOCK(p->i_mode)) putchar('s');
    else if (S_ISFIFO(p->i_mode)) putchar('p');
    else putchar('-');

    putchar(p->i_mode & S_IRUSR ? 'r' : '-');
    putchar(p->i_mode & S_IWUSR ? 'w' : '-');
    if (p->i_mode & S_ISUID) putchar(p->i_mode & S_IXUSR ? 's' : 'S');
    else putchar(p->i_mode & S_IXUSR ? 'x' : '-');

    putchar(p->i_mode & S_IRGRP ? 'r' : '-');
    putchar(p->i_mode & S_IWGRP ? 'w' : '-');
    if (p->i_mode & S_ISGID) putchar(p->i_mode & S_IXGRP ? 's' : 'S');
    else putchar(p->i_mode & S_IXGRP ? 'x' : '-');

    putchar(p->i_mode & S_IROTH ? 'r': '-');
    putchar(p->i_mode & S_IWOTH ? 'w': '-');
    if (p->i_mode & S_ISVTX) putchar(p->i_mode & S_IXOTH ? 't' : 'T');
    else putchar(p->i_mode & S_IXOTH ? 'x' : '-');
    putchar(' ');
}


void
printuidgid(info *p)
{
    char *s;
    char bfr[8];

    if ( (s = about(&pass, p->i_uid)) == 0 ) {
	struct passwd *pwd = getpwuid(p->i_uid);
	if (pwd)
	    s = remember(&pass, p->i_uid, pwd->pw_name);
	else {
	    snprintf(bfr, sizeof bfr, "%d", p->i_uid);
	    s = remember(&pass, p->i_uid, bfr);
	}
    }
    printf("%-8.8s", s);

    if ( (s = about(&group, p->i_gid)) == 0) {
	struct group *grp = getgrgid(p->i_gid);
	if (grp)
	    s = remember(&group, p->i_gid, grp->gr_name);
	else {
	    snprintf(bfr, sizeof bfr, "%d", p->i_gid);
	    s = remember(&group, p->i_gid, bfr);
	}
    }
    printf("%-8.8s", s);
}


int
internet_time(struct tm *t)
{
    struct tm tz0;

    time_t start, now;
    int septday;
    
    if ( (t->tm_year < 93) || (t->tm_year == 93 && t->tm_mon <= 8) )
	return 0;

    bzero(&tz0, sizeof tz0);	/* last day in august */
    tz0.tm_mon = 7;
    tz0.tm_mday = 31;
    tz0.tm_year = 93;

    start = mktime(&tz0);
    now = mktime(t);

    septday = (now-start) / (24*60*60);	/* drifts on leap-seconds */

    t->tm_year = 93;
    t->tm_mon = 8;
    t->tm_mday = septday;
    return 1;
}



void
printdate(time_t clock)
{
    char buf[20];
#define HALFYEAR (60*60*24*182)
    static time_t now = 0;
    struct tm *tm = localtime(&clock);
    int ancient;

    if (now == 0) {
	time(&now);
    }

    if (psuvm) {
	int newtime = internet_time(tm);
	strftime(buf, sizeof buf, newtime ? "%b %e"
	                                  : "%b %e  %Y", tm);
    }
    else {
	ancient = (clock < now-HALFYEAR) || (clock > now+HALFYEAR);
	strftime(buf, sizeof buf, ancient ? "%b %e  %Y" : "%b %e %H:%M", tm);
    }
    printf("%12.12s ", buf);
}


void
printsizes(off_t size)
{
    if (fancysizes) {
	if (size < 10000)
	    printf("%6ld ", size);
	else {
	    static char ext[] = "KMGTPEZY";

	    char bfr[8];
	    int ui, hsize, frac;

	    for (ui=0; (size >= 1000*1024) && ext[ui]; ui++)
		size /= 1024;

	    hsize = size/1024;
	    frac = (size%1024)/102;
	    if (ext[ui] == 'K' && frac >= 5) {
		hsize++;
		frac=0;
	    }
	    if ( (hsize < 100) && (frac != 0) )
		snprintf(bfr, sizeof bfr, "%d.%d", hsize, frac);
	    else
		snprintf(bfr, sizeof bfr, "%4d", hsize);
	    printf(" %4s%c ", bfr, ext[ui]);
	}
    }
    else
	printf("%8ld ", size);
}


void
printname(char *p, int len)
{
    int i;
    int ch;

    if (pretty) {
	for (i = 0; i < len; i++) {
	    ch = p[i];
	    if ( isprint(ch) && (ch >= ' ') )
		putchar(ch);
	    else
		putchar('?');
	}
    }
    else
	fwrite(p, len, 1, stdout);
}



void
printit(info *p)
{
    int sz;
    char link[80];

    if (permissions)
	printperm(p);
    if (links)
	printf("%3d ", p->i_links);
    if (inodes)
	printf("%7d ", p->i_ino);
    if (blocks)
	printf("%5d ", p->i_blocks);
    if (owner)
	printuidgid(p);
    if (sizes)
	printsizes(p->i_size);
    if (dates)
	printdate(date(p));

    printname(p->i_name, p->i_len);

    if (S_ISLNK(p->i_mode) && showtarget
			   && (sz=readlink(p->i_name, link, sizeof link)) > 0) {
	printf(" -> ");
	printname(link, sz);
    }

}

void
prefix(info *p)
{
    if (fancy)
	putchar(S_ISDIR(p->i_mode) ? '[' : ' ');
}


void
suffix(info *p)
{
    if (fancy) {
	if (S_ISDIR(p->i_mode))
	    putchar(']');
	else if (S_ISFIFO(p->i_mode))
	    putchar('|');
	else if (S_ISSOCK(p->i_mode))
	    putchar('=');
	else if (S_ISLNK(p->i_mode))
	    putchar('@');
	else if (S_ISREG(p->i_mode) && (p->i_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
	    putchar('*');
	else
	    putchar(' ');
    }
}


void
ls(pack *p)
{
    int i, j, idx, sz, max=0, maxuf;
    int cols, depth, runt;

    if (p->nrf == 0) return;

    if (columns) {
	/* calculate how many rows, then correct for runty last row
	 */
	for (i=0; i<p->nrf; i++)
	    if ( p->index[i]->i_len > max )
		max = p->index[i]->i_len;

	if (!fancy)
	    max += 2;
	maxuf = max;
	if (inodes)
	    max += 8;
	if (blocks)
	    max += 6;
	if (fancy)
	    max += 2;

	cols = (1+80)/max;	/* XXXX assuming fixed width screen! */

	if (cols == 1)
	    depth = p->nrf;
	else if (cols == p->nrf)
	    depth = 1;
	else {
	    depth = (p->nrf+cols-1) / cols;
	    if ( (runt = p->nrf % depth) == 0)
		runt = depth;

	    if ( (depth-runt)*cols >= p->nrf)
		depth -= (depth-runt)/cols;
	}


	for (j=0; j < depth; j++) {
	    for (i=0; i < cols; i++) {
		idx = (i*depth) + j;

		if (idx < p->nrf) {
		    prefix(p->index[idx]);
		    printit(p->index[idx]);
		    suffix(p->index[idx]);
		    if ( i < cols-1 && idx+depth < p->nrf) {
			printf("%*s", maxuf-(p->index[idx]->i_len), "");
			continue;
		    }
		}
	    }
	    putchar('\n');
	}
    }
    else for (i=0; i < p->nrf; i++) {
	printit(p->index[i]);
	putchar('\n');
    }
}


lsdir(char *path)
{
    pack pk;
    long i, bsize;

    if ( package(path, &pk) >= 0) {
	if (needaheader)
	    printf("\n%s:\n", path);
	if (dirblocks) {
	    for (i=0,bsize=0; i < pk.nrf; i++)
		bsize += pk.files[i].i_blocks;
	    printf("total %d:\n", bsize);

	}
	ls(&pk);
	discard(&pk);
    }
}


void
usage()
{
    fprintf(stderr,"usage: %s [-acdfghilrstuBCFI1] [-Sorder] [file]...\n", pgm);
    exit(1);
}


double
main(int argc, char **argv)
{
    int opt;
    int i;
    pack files, dirs;
    struct stat st;

    opterr = 0;

    columns = isatty(0);
    pgm = basename(argv[0]);

    columns = isatty(0);

    if (strcasecmp(pgm, "lf") == 0)
	fancy = pretty = 1;
    else if ( (strcasecmp(pgm, "ll") == 0) || (strcasecmp(pgm, "dir") == 0) ) {
	columns=0;
	dirblocks=showtarget=sizes=dates=links=owner=permissions=1;
    }

    while ((opt = getopt(argc,argv,"acdfghilrstuBCFIS:1-:")) != EOF) {
	switch (opt) {
	case 'l':   columns=0;
		    dirblocks=showtarget=sizes=dates=links=owner=permissions=1;
		    break;
	case 'a':   all = 1;
		    break;
	case 'h':   fancysizes = 1;
		    break;
	case 't':   sortorder = BYTIME;
		    break;
	case 's':   blocks = 1;
		    break;
	case 'i':   inodes = 1;
		    break;
	case 'f':   sortorder = NOT;
		    break;
	case 'c':   whichtime = MTIME;
		    break;
	case 'd':   directories = 0;
		    break;
	case 'u':   whichtime = CTIME;
		    break;
	case 'g':   break;
	case 'B':   pretty = 1;
		    break;
	case 'C':   columns = 1;
		    break;
	case '1':   columns = 0;
		    break;
	case 'r':   reverse = 1;
		    break;
	case 'F':   fancy=pretty = 1;
		    break;
	case 'I':   psuvm = 1;
		    break;
	case 'S':   /* silly sort order */
		    if (strcasecmp(optarg, "time") == 0)
			sortorder = BYTIME;
		    else if (strcasecmp(optarg, "name") == 0)
			sortorder = BYNAME;
		    else if (strcasecmp(optarg, "size") == 0)
			sortorder = BYSIZE;
		    else if (strcasecmp(optarg, "length") == 0)
			sortorder = BYLEN;
		    else {
			fprintf(stderr, "%s: -S takes TIME,NAME,SIZE, or LENGTH\n", pgm, optarg);
			usage();
		    }
		    break;
	case '-':   /*silently kill gnu-style long options */
		    break;
	default:    usage(1);
	}
    }

    argc -= optind;
    argv += optind;

    needaheader = (argc > 1);
    needstat = follow || inodes || links || dates || sizes
		      || blocks || owner || permissions || dirblocks;
#if !defined(DTTOIF)
    needstat = needstat || fancy;
#endif


    if (argc == 0)
	lsdir(".");
    else {
	newer(&files);
	newer(&dirs);

	/* list files first */
	for (i=0; i < argc; i++)
	    if ( ((follow ? stat : lstat)(argv[i],&st)) == -1 ) {
		fprintf(stderr, "%s: ", pgm); perror(argv[i]);
		continue;
	    }
	    else  if (directories && S_ISDIR(st.st_mode))
		more(argv[i], &st, &dirs);
	    else
		more(argv[i], &st, &files);

	mkindex(&files);
	mkindex(&dirs);

	if (files.nrf) ls(&files);

	for (i=0; i < dirs.nrf; i++)
	    lsdir(dirs.index[i]->i_name);
    }
    exit(0);
}
