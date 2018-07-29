#include <log.h>
#include <stdarg.h>

static const char *strlevel[LOG_LEVELS] = {
	"Debug",
	"Info",
	"Warning",
	"Error",
	"Critical",
	"Fatal"
};
static FILE * log_file;
static int log_level;

void log_init( FILE * log, int log_level )
{
    log_file = log;
}

void logprintf( int level, const char *format, ... )
{
	va_list args;
	static char buf[1024];

	if ( 0 == log_file ) return;
	if ( log_level > level ) return;
	va_start( args, format );
	
	sprintf( buf, "%s: %s\n", strlevel[level], format );
	vfprintf( log_file, buf, args );
	fflush( log_file );

	va_end( args );
}
