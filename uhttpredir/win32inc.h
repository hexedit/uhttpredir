#if !defined(__WIN32INC_H__) && defined(_WIN32)
#define __WIN32INC_H__

#ifdef DATADIR
# undef DATADIR
#endif
#define DATADIR g_datadir
extern const char * g_datadir;

#ifdef LOCALEDIR
# undef LOCALEDIR
#endif
#define LOCALEDIR g_localedir
extern const char * g_localedir;

#ifdef SYSCONFDIR
# undef SYSCONFDIR
#endif
#define SYSCONFDIR g_sysconfdir
extern const char * g_sysconfdir;

#ifdef LOCALSTATEDIR
# undef LOCALSTATEDIR
#endif
#define LOCALSTATEDIR g_localstatedir
extern const char * g_localstatedir;

void win32init(void);

#endif /* __WIN32INC_H__ */
