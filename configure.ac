AC_PREREQ([2.63])
AC_INIT([uhttpredir], [0.1.0])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([uhttpredir/main.c])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])
LT_INIT

AC_ARG_ENABLE(debug,
    [AC_HELP_STRING(
            [--enable-debug],
            [Enable build of debugging information and features]
    )]
)


if test "$sysconfdir" = '${prefix}/etc'; then
    sysconfdir='${prefix}/etc/'"${PACKAGE_NAME}"
fi


#########################
# Detect build platform #
#########################

AC_MSG_CHECKING([for host platform])

case $host_os in
    mingw*)   platform=win32; win32=yes ;;
    linux*)   platform=linux ;;
    freebsd*) platform=freebsd ;;
    openbsd*) platform=openbsd ;;
esac

if test x$platform = x; then
    AC_MSG_RESULT([unknown])
    AC_MSG_ERROR([Platform $host_os is not supported])
fi

AC_MSG_RESULT([$platform])


#########################
# Check for build tools #
#########################

if test x$enable_debug = xyes; then
    : ${CFLAGS="-O0 -g -Wall -DDEBUG"}
fi

AC_PROG_INSTALL
AC_PROG_MKDIR_P

#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION([0.17])

PKG_PROG_PKG_CONFIG

# Win32-specific tools
if test x$win32 = xyes; then
    if test x$enable_msi = xyes; then
        AC_CHECK_TOOL(CANDLE, candle, [], [msi=no])
        AC_CHECK_TOOL(LIGHT, light, [], [msi=no])
        AC_CHECK_TOOL(HEAT, heat, [], msi=no)
    fi
fi


##################################
# Check for standard C libraries #
##################################

AC_HEADER_STDC
AC_CHECK_HEADERS([getopt.h], [], [NOSTD=yes])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([pthread.h], [], [NOSTD=yes])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([arpa/inet.h])

# Platform-specific library checks
if test x$win32 = xyes; then
    AC_CHECK_HEADERS([windows.h], [], [NOSTD=yes])
    AC_CHECK_HEADERS([winsock2.h], [], [NOSTD=yes])
fi

# Required functions
AC_CHECK_FUNCS([getopt_long], [], [NOSTD=yes])

# Optional functions
AC_CHECK_FUNCS([sigaction])

# Required types

# Optional types

if test x$NOSTD = xyes; then
    AC_MSG_ERROR([Standard library has not met requirements])
fi


###############################
# Check required dependencies #
###############################

# Check for libmicrohttpd
PKG_CHECK_MODULES(
    [LIBMICROHTTPD],
    [libmicrohttpd >= 0.9.52],
    [],
    [
        AC_MSG_ERROR([libmicrohttpd >= 0.9.52 required but not found])
    ]
)

# Check for json-c
PKG_CHECK_MODULES(
    [JSON_C],
    [json-c >= 0.12],
    [],
    [
        AC_MSG_ERROR([json-c >= 0.12 required but not found])
    ]
)


#########################
# Automake conditionals #
#########################

AM_CONDITIONAL(WIN32, test x$win32 = xyes)
AM_CONDITIONAL(DEBUG, test x$enable_debug = xyes)
AM_CONDITIONAL(MSI,   test x$msi != xno)


###################
# Autoconf output #
###################

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    uhttpredir/Makefile
    win32-msi/Makefile
])
AC_OUTPUT
