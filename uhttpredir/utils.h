#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.h>

char * load_file(const char *file);

#ifdef _WIN32

char *win32_strerror(int err);

#endif /* _WIN32 */

#endif /* __UTILS_H__ */
