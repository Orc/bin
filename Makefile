CFLAGS = -Ilibc -I.


PROGS=date who cat id df uname ls


all: $(PROGS)

date:  date.o libc/strftime.o basis/options.o

who:   who.o libc/utmp.o libc/dbz.o basis/options.o

cat:   cat.o

id:    id.o basis/options.o

df:    df.o basis/options.o

uname: uname.o basis/options.o


clean: 
	rm -f $(PROGS) *.o
