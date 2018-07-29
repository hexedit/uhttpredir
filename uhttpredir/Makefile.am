AM_CFLAGS = -I$(top_srcdir) -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALSTATEDIR=\"$(localstatedir)\"

if WIN32
bin_PROGRAMS = uhttpredir
else
sbin_PROGRAMS = uhttpredir
endif

noinst_HEADERS = app.h options.h utils.h log.h

uhttpredir_SOURCES = main.c app.c options.c utils.c log.c
uhttpredir_CFLAGS = $(AM_CFLAGS) $(LIBMICROHTTPD_CFLAGS) $(JSON_C_CFLAGS)
uhttpredir_LDFLAGS = $(AM_LDFLAGS) $(LIBMICROHTTPD_LIBS) $(JSON_C_LIBS)
uhttpredir_LDADD =

if WIN32
uhttpredir_SOURCES += win32svc.c
uhttpredir_LDFLAGS += -lws2_32
endif

EXTRA_DIST = win32inc.h win32svc.c win32svc.h
