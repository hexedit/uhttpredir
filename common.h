#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif

#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifndef BOOL
# ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
# endif
# ifdef HAVE__BOOL
#  define BOOL _Bool
# else
#  define BOOL int
#  define TRUE 1
#  define FALSE 0
# endif
#endif

#ifndef _
# define _(x) x
#endif

#endif /* __COMMON_H__ */
