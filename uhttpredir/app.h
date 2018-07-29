#ifndef __APP_H__
#define __APP_H__

#include <common.h>

#ifdef _WIN32
# include <win32inc.h>
#endif

int app_init(int argc, char ** argv);
int app_run(void);
void app_shutdown(void);

#endif /* __APP_H__ */
