CFLAGS = -Ilibc -I.


PROGS=date who cat id


all: $(PROGS)

date:  date.o libc/strftime.o basis/options.o

who:   who.o libc/utmp.o libc/dbz.o

cat:   cat.o

id:    id.o basis/options.o


clean: 
	rm -f $(PROGS) *.o
