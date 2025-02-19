#include "stdarg.h"
#include "unistd.h"

static char printbuf[1024];
extern int vsprintf(char *buf, const char *fmt, va_list args);

int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}
