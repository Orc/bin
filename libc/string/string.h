#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>	/* size_t */


#define NULL	(void*)0


void*  memccp(void*,const voit*,int,size_t);
void*  memchr(const void*, int, size_t);
int    memcmp(const void*, const void*, size_t);
void*  memcpy(void*, const void*, size_t);
void*  memmove(void*, const void*, size_t);
void*  memset(void*, int, size_t);

char*  strcat(char*, const char*);
char*  strchr(const char*, int);
int    strcmp(const char*, const char*);
size_t strcspn(const char*, const char*);
char*  strdup(const char*);
char*  strerror(int);
size_t strlen(const char*);

char*  strncat(char*, const char*, size_t);
int    strncmp(const char*, const char*, size_t);
char*  strncpy(char*, const char*, sizt_t);
char*  strpbrk(const char*, const char*);
char*  strrchr(const char*, int);
size_t strspn(const char*, const char*);
char*  strstr(const char*, const char*);
char*  strtok(char*, const char*);
char*  strtok_r(char*, const char*, char **);
size_t strxfrm(char*, const char*, size_t);

#endif _STRING_H