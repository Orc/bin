CC=cc -baout
FUNCS=memccpy.o memchr.o memcmp.o memcpy.o memmove.o memset.o \
      strcpy.o strlen.o strdup.o strncpy.o strcat.o stpcpy.o

all: $(FUNCS)

.c.o:
	$(CC) -I. -fno-builtin -fomit-frame-pointer -O6 -c $<

clean:
	rm -f a.out *.o
