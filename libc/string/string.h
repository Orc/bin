#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>	/* size_t && NULL */

void*  memccpy(void*,const void*,int,size_t);
void*  memchr(const void*, int, size_t);
int    memcmp(const void*, const void*, size_t);
void*  memcpy(void*, const void*, size_t);
void*  memmove(void*, const void*, size_t);
void*  memset(void*, int, size_t);

size_t strlen(const char*);
char*  strchr(const char*, int);

char*  strcpy(char*, const char*);
char*  strcat(char*, const char*);
int    strcmp(const char*, const char*);

char*  strncpy(char*, const char*, size_t);
char*  strncat(char*, const char*, size_t);
int    strncmp(const char*, const char*, size_t);

int    strcasecmp(const char*,const char*);
int    strncasecmp(const char*,const char*,size_t);

char*  strdup(const char*);
char*  strerror(int);	/* EXTERN: from errno */

char*  strpbrk(const char*, const char*);
size_t strcspn(const char*, const char*);
char*  strrchr(const char*, int);
size_t strspn(const char*, const char*);
char*  strstr(const char*, const char*);
char*  strtok(char*, const char*);
char*  strtok_r(char*, const char*, char **);
#if LOCALE
size_t strxfrm(char*, const char*, size_t);
#else
#define strxfrm(d,s,c) strncpy(d,s,c)
#endif

#endif _STRING_H
