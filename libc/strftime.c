#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <stdio.h>

struct context {
    char *bfr;
    int   bufsiz;
    const struct tm *t;
    int   err;
    int   ix;
};

static void fillftime(struct context *ctx, const char *format);

static void
fillc(struct context *ctx, char c)
{
    if (ctx->ix >= ctx->bufsiz || ctx->ix < 0)
	ctx->err = 1;
    else
	ctx->bfr[ctx->ix++] = c;
}

static void
fills(struct context *ctx, char *s)
{
    int size;


    if (s == 0)
	ctx->err = 1;
    else {
	size = strlen(s);

	if (ctx->ix < 0 || ctx->ix + size >= ctx->bufsiz)
	    ctx->err = 1;
	else {
	    memcpy(ctx->bfr + ctx->ix, s, size);
	    ctx->ix += size;
	}
    }
}

static void
fill(struct context *ctx, const char *fmt, ...)
{
    va_list ptr;
    int written;

    if (ctx->ix < 0)
	ctx->err = 1;
    else {
	va_start(ptr,fmt);

	written = vsnprintf(ctx->bfr+ctx->ix, ctx->bufsiz - ctx->ix, fmt, ptr);

	if (written < 0 || written + ctx->ix > ctx->bufsiz)
	    ctx->err = 1;
	else
	    ctx->ix += written;

	va_end(ptr);
    }
}


static char *
weekday_name(int wday)
{
    switch (wday) {
    case 0: return "Sunday";
    case 1: return "Monday";
    case 2: return "Tuesday";
    case 3: return "Wednesday";
    case 4: return "Thursday";
    case 5: return "Friday";
    case 6: return "Saturday";
    default:return 0;
    }
}

static char *
weekday_abbr(int wday)
{
    switch (wday) {
    case 0: return "Sun";
    case 1: return "Mon";
    case 2: return "Tue";
    case 3: return "Wed";
    case 4: return "Thu";
    case 5: return "Fri";
    case 6: return "Sat";
    default:return 0;
    }
}


static char *
month_name(int month)
{
    switch (month) {
    case 0: return "January";
    case 1: return "February";
    case 2: return "March";
    case 3: return "April";
    case 4: return "May";
    case 5: return "June";
    case 6: return "July";
    case 7: return "August";
    case 8: return "September";
    case 9: return "October";
    case 10:return "November";
    case 11:return "December";
    default: return 0;
    }
}


static char *
month_abbr(int month)
{
    switch (month) {
    case 0: return "Jan";
    case 1: return "Feb";
    case 2: return "Mar";
    case 3: return "Apr";
    case 4: return "May";
    case 5: return "Jun";
    case 6: return "Jul";
    case 7: return "Aug";
    case 8: return "Sep";
    case 9: return "Oct";
    case 10:return "Nov";
    case 11:return "Dec";
    default: return 0;
    }
}


static char *
ampm(int hour)
{
    return (hour < 12) ? "am" : "pm";
}


static int
weekofyear(const struct tm *time, int kind)
{
    int days, weeks;

    switch (kind) {
    default: days = time->tm_wday; break;
    case 1:  days = time->tm_wday-1; break;
    case 2:  days = (time->tm_wday < 3) ? (time->tm_wday-1)
					: (time->tm_wday + 6);
    }


    weeks = ( 6 + time->tm_yday - time->tm_wday )/ 7;

    if (weeks && (days > time->tm_yday % 7)) --weeks;

    return weeks ? weeks : 53/*we're the stub of the last year*/;
}


static void
expand(struct context *ctx, char control)
{
    switch (control) {
    case 0:     ctx->err = 1; break;
    case 'A':   fills(ctx,weekday_name(ctx->t->tm_wday)); break;
    case 'a':   fills(ctx,weekday_abbr(ctx->t->tm_wday)); break;
    case 'B':   fills(ctx,month_name(ctx->t->tm_mon)); break;
    case 'b':   fills(ctx,month_abbr(ctx->t->tm_mon)); break;
    case 'C':   fill(ctx,"%02d", (1900+ctx->t->tm_year) / 100); break;
    case 'c':   expand(ctx, '+'); break;
    case 'D':   fillftime(ctx, "%m/%d/%y"); break;
    case 'd':   fill(ctx,"%02d", ctx->t->tm_mday); break;
    case 'e':   fill(ctx,"%2d", ctx->t->tm_mday); break;
    case 'F':	fillftime(ctx, "%Y-%m-%d"); break;
    case 'g':
    case 'G':   {   int year;

		    year = ctx->t->tm_year + 1900;
		    if (ctx->t->tm_yday <= ctx->t->tm_wday)
			year--;
		    if (control == 'g')
			fill(ctx, "%02d", year%100);
		    else
			fill(ctx, "%d", year);
		}
		break;
    case 'h':   expand(ctx, 'b'); break;
    case 'H':   fill(ctx,"%02d", ctx->t->tm_hour); break;
    case 'I':   fill(ctx,"%02d", ctx->t->tm_hour%12); break;
    case 'j':   fill(ctx,"%03d", ctx->t->tm_yday); break;
    case 'k':   fill(ctx,"%2d", ctx->t->tm_hour); break;
    case 'l':   fill(ctx,"%2d", ctx->t->tm_hour % 12); break;
    case 'M':   fill(ctx,"%02d", ctx->t->tm_min); break;
    case 'm':   fill(ctx,"%02d", ctx->t->tm_mon + 1); break;
    case 'n':   fillc(ctx,'\n'); break;
    case 'p':   fills(ctx,ampm(ctx->t->tm_hour)); break;
    case 'R':   fillftime(ctx, "%H:%M"); break; break;
    case 'r':	fillftime(ctx, "%I:%M:%S %p"); break;
    case 'S':   fill(ctx,"%02d", ctx->t->tm_sec); break;
    case 's':   fill(ctx,"%d", mktime((struct tm*)ctx->t)); break;
    case 'T':   fillftime(ctx, "%H:%M:%S"); break;
    case 't':   fillc(ctx,'\t'); break;
    case 'U':   fill(ctx, "%02d", weekofyear(ctx->t, 0)); break;
    case 'u':   fill(ctx,"%d", ctx->t->tm_wday == 0 ? 7 : ctx->t->tm_wday);
			    break;
    case 'V':	fill(ctx, "%02d", weekofyear(ctx->t,2)); break;
    case 'v':   fillftime(ctx, "%e-%b-%Y"); break;
    case 'W':   fill(ctx, "%02d", weekofyear(ctx->t,1)); break;
    case 'w':   fill(ctx,"%d", ctx->t->tm_wday); break;
    case 'X':   expand(ctx, 'T'); break;
    case 'x':   expand(ctx, 'D'); break;
    case 'Y':   fill(ctx,"%d", ctx->t->tm_year + 1900); break;
    case 'y':   fill(ctx,"%02d", ctx->t->tm_year % 100); break;
    case 'Z':   fills(ctx,tzname[ctx->t->tm_isdst ? 1 : 0]); break;
    case 'z':   {   int plus, tz;
    #if _LINUX_C_LIB_VERSION_MAJOR == 4
		    tz = timezone / 60;
    #else
		    tz = ctx->t->tm_gmtoff / 60;
    #endif
		    plus = ( tz>=0 ) ? '+' : '-';
		    tz %= (12*60);
		    fill(ctx,"%c%02d%02d", plus, abs(tz/60), abs(tz%60));
		}
		break;
    case '+':   fillftime(ctx, "%a %b %d %R:%S %Z %Y"); break;
    default:
    case '%':   fillc(ctx, control); break;
    }
}


static void
fillftime(struct context *ctx, const char *format)
{
    const char *p;

    for (p = format; *p && !ctx->err; ++p) {
	if (*p == '%')
	    expand(ctx, *++p);
	else 
	    fillc(ctx, *p);
    }
}


size_t
strftime(char *res, size_t size, const char *format, const struct tm *time)
{
    struct context c = { res, size, time };

    if ( res == 0 || size <= 0 || time == 0 || format == 0 )
	return 0;

    fillftime(&c, format);
    fillc(&c, 0);
    return c.err ? 0 : (c.ix-1);
}
