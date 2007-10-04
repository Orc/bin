#ifndef _UTMP_D
#define _UTMP_D

#include <unistd.h>
#include <time.h>
#include <paths.h>


#define UTMP_FILE	_PATH_UTMP
#define UTMP_FILENAME	UTMP_FILE

#define UT_UNKNOWN	0
#define RUN_LVL		1
#define BOOT_TIME	2
#define NEW_TIME	3
#define OLD_TIME	4
#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8


/* the traditional libc4-compatable utmp, which is embedded into
 * the utmpx structure right below it
 */
struct utmp {
    short  ut_type;		/* entry types, defined above */
    pid_t  ut_pid;		/* process id for *_PROCESS types */
    char   ut_line[12];		/* device name -"/dev" */
    char   ut_id[2];		/* init id or abbreviated device name */
    time_t ut_time;		/* when this record was last modified */
    char   ut_host[16];		/* remote host name */
    long   ut_addr;		/* remote IP address */
};

struct utmpx {
    struct utmp ut;		/* old utmp */
    char alloc[64];		/* room for ipv6 and flags */
};


void utmpname(const char *);

void setutent();
void endutent();

struct utmp *getutent();

struct utmp *getutid(struct utmp *);
struct utmp *getutline(struct utmp *);
void pututline(struct utmp *);

#endif/*_UTMP_D*/
