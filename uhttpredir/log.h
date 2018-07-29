#ifndef __LOG_H__
#define __LOG_H__

#include <common.h>
#include <stdio.h>

enum {
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL,
	LOG_FATAL,
	LOG_LEVELS
};

void log_init( FILE * log, int log_level );
void logprintf( int level, const char *format, ... );

#endif /* __LOG_H__ */
