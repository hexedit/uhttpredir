#include <app.h>
#include <options.h>
#include <win32svc.h>
#include <log.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
# define HAVE_SIGNAL
#endif /* HAVE_SIGNAL_H */

#ifdef _WIN32

static char l_datadir[MAX_PATH];
static char l_localedir[MAX_PATH];
static char l_sysconfdir[MAX_PATH];
static char l_localstatedir[MAX_PATH];

const char * g_datadir = l_datadir;
const char * g_localedir = l_localedir;
const char * g_sysconfdir = l_sysconfdir;
const char * g_localstatedir = l_localstatedir;

static void win32init(void);

#endif /* _WIN32 */

static int daemon_run(void);
static void signal_handler(int);

int main(int argc, char ** argv)
{
    int r;

#ifdef _WIN32
    win32init();
#endif

    r = app_init(argc, argv);
    if (0 != r)
        return r;

#ifdef HAVE_SIGNAL

#ifndef _WIN32
	signal( SIGUSR1, signal_handler );
	signal( SIGHUP, signal_handler );
#endif /* ! _WIN32 */

	signal( SIGINT, signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGSEGV, signal_handler );
	signal( SIGABRT, signal_handler );

#endif /* HAVE_SIGNAL */
    
#ifdef _WIN32
    if (g_options->win32_start_service)
        return service_start();
    if (g_options->win32_stop_service)
        return service_stop();
    if (g_options->win32_install_service)
        return service_install();
    if (g_options->win32_remove_service)
        return service_remove();
#endif

    if (!g_options->daemon)
        return app_run();
    else
        return daemon_run();
}

#ifdef _WIN32
static void win32init(void)
{
    static char _binpath[MAX_PATH], _apppath[MAX_PATH], _modname[MAX_PATH];

    GetModuleFileName(NULL, _binpath, MAX_PATH);
    _splitpath(_binpath, NULL, NULL, _modname, NULL);
    strcat(_binpath, "\\..\\..");
    GetFullPathName(_binpath, MAX_PATH, _apppath, NULL);

    sprintf(l_datadir, "%s\\share", _apppath);
    sprintf(l_localedir, "%s\\locale", g_datadir);
    sprintf(l_sysconfdir, "%s\\conf", _apppath);
    sprintf(l_localstatedir, "%s\\var", _apppath);
}
#endif /* _WIN32 */

static int daemon_run(void)
{
#ifdef _WIN32

    return service_init_start();

#else /* _WIN32 */

    int fr;

    fr = fork();
    switch ( fr )
    {
        case 0:
            return app_run();
        case -1:
            return errno;
        default: {
            FILE *pid;
            pid = fopen( g_options->pidfile, "w+" );
            if ( 0 != pid )
            {
                fprintf( pid, "%d\n", fr );
                fclose( pid );
            }
        }
    }

    return 0;

#endif /* _WIN32 */
}

static void signal_handler( int sig )
{
#ifdef HAVE_SIGNAL
	switch ( sig )
	{

#ifndef _WIN32
		case SIGUSR1:
			break;
		case SIGHUP:
			// TODO reload configuration
			break;
#endif /* ! _WIN32 */

		case SIGINT:
			logprintf( LOG_INFO, "Interrupt. Terminating..." );
            app_shutdown();
			break;
		case SIGTERM:
			logprintf( LOG_INFO, "Terminating..." );
            app_shutdown();
			break;
		case SIGSEGV:
            // TODO debug output
			exit( EXIT_FAILURE );
		case SIGABRT:
            // TODO debug output
			exit( EXIT_FAILURE );
	}
#endif /* HAVE_SIGNAL */
}
