/*
 * cat:  do what you'd expect
 *
 * This code is in the public domain.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


int
mmapcat(int fd)
{
    struct stat sb;
    char *map;

    /* don't mmap() the file if we can't stat it, it's not
     * a regular file, or if it's larger than 16mb
     */
    if ( fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode) || sb.st_size > (1L<<24) )
	return 0;

    if ( (map = mmap(0,sb.st_size,PROT_READ,MAP_SHARED,fd,0)) == (char*)-1 )
	return 0;

    write(1, map, sb.st_size);
    munmap(map, sb.st_size);
    return 1;
}


void
cat(char* file)
{
    int fd;
    char bfr[5120];
    int siz;

    if (strcmp(file, "-") == 0)
	fd = 0;
    else if ( (fd = open(file, O_RDONLY)) == -1 ) {
	perror(file);
	return;
    }

    if (!mmapcat(fd))
	while ((siz = read(fd, bfr, sizeof bfr)) > 0)
	    write(1,bfr,siz);
    close(fd);
}


void
main(int argc, char ** argv)
{

    int x;

    if (argc <= 1)
	cat("-");
    for (x=1; x < argc; x++)
	cat(argv[x]);
    exit(0);
}
