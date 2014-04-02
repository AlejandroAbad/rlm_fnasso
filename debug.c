#include "debug.h"
#include <fnasso.h>
#include <stdarg.h>
#include <stdio.h>


char *str_lvl(int lvl)
{
	switch(lvl)
	{
		case FNASSO_DEBUG: return "DBG";
		case FNASSO_EVENT: return "EVT";
		case FNASSO_WARN: return "WRN";
		case FNASSO_ERROR: return "ERR";
		default: "???";
	}
}

void fnasso_printf(int lvl, const char* fooname, const char *fmt, ...)
{	
	va_list ap;
	va_start(ap, fmt);
	

	printf("[%s] %s: ", str_lvl(lvl), fooname);
	vprintf(fmt, ap);
	printf("\n");

	va_end(ap);
}


void fnasso_func_start(const char *fooname)
{
	fnasso_printf(FNASSO_DEBUG, fooname, "");
}

void fnasso_func_end(int level, const char *fooname, int status)
{
	fnasso_printf(level, fooname, "exit status (%d)", status);
}


