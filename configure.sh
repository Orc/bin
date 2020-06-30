#! /bin/sh

# local options:  ac_help is the help message that describes them
# and LOCAL_AC_OPTIONS is the script that interprets them.  LOCAL_AC_OPTIONS
# is a script that's processed with eval, so you need to be very careful to
# make certain that what you quote is what you want to quote.

# load in the configuration file
#
ac_help=''

LOCAL_AC_OPTIONS='
set=`locals $*`;
if [ "$set" ]; then
    eval $set
    shift 1
else
    ac_error=T;
fi'

locals() {
	:
}

TARGET=bin
. ./configure.inc

AC_INIT $TARGET
unset _MK_LIBRARIAN

AC_PROG_CC

# gcc (and clang) can bite me.
if [ "$IS_BROKEN_CC" ]; then
    case "$AC_CC $AC_CFLAGS" in
    *-Wall*)    ;;
    *)          AC_DEFINE 'while(x)' 'while( (x) != 0 )'
		AC_DEFINE 'if(x)' 'if( (x) != 0 )' ;;
    esac
fi

AC_PROG ar || AC_FAIL "$TARGET requires ar"
AC_PROG ranlib

AC_C_VOLATILE
AC_C_CONST
AC_SCALAR_TYPES sub

if AC_CHECK_BASENAME; then
    AC_SUB 'BASENAME' ''
    AC_CHECK_HEADERS libgen.h && AC_INCLUDE libgen.h
else
    AC_SUB 'BASENAME' 'libc/basename.o'
    AC_DEFINE 'basename' 'safe_basename'
    AC_EXTERN 'char *basename(const char *)'
fi

if AC_CHECK_FIELD utmp ut_user sys/types.h utmp.h; then
    AC_SUB 'UTMP' 'libc/utmp.o'
    AC_SUB 'WHO'  'who'
else
    AC_SUB 'UTMP' ''
    AC_SUB 'WHO'  ''
fi


AC_CHECK_HEADERS sys/types.h pwd.h && AC_CHECK_FUNCS getpwuid

if AC_CHECK_FUNCS 'bzero((char*)0,0)'; then
    : # Yay
elif AC_CHECK_FUNCS 'memset((char*)0,0,0)'; then
    AC_DEFINE 'bzero(p,s)' 'memset(p,s,0)'
else
    AC_FAIL "$TARGET requires bzero or memset"
fi

if AC_CHECK_FUNCS 'strcasecmp("a","b")'; then
    :
elif AC_CHECK_FUNCS stricmp; then
    AC_DEFINE strcasecmp stricmp
else
    AC_FAIL "$TARGET requires either strcasecmp() or stricmp()"
fi

if AC_CHECK_FUNCS 'strncasecmp("a","b", 1)'; then
    :
elif AC_CHECK_FUNCS strnicmp; then
    AC_DEFINE strncasecmp strnicmp
else
    AC_FAIL "$TARGET requires either strncasecmp() or strnicmp()"
fi

AC_CHECK_FUNCS 'mbstowcs'

if AC_CHECK_HEADERS sys/vfs.h; then
    AC_SUB 'DF' 'df'
else
    AC_SUB 'DF' ''
fi

AC_OUTPUT Makefile
