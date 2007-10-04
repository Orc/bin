CFLAGS = -Ilibc -I.


all: date who

date:  date.o libc/strftime.o basis/options.o

who:   who.o libc/utmp.o libc/dbz.o
