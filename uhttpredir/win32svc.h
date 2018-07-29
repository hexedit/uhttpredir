#ifndef __WIN32SVC_H__
#define __WIN32SVC_H__

#include <win32inc.h>

#define SERVICE_NAME "uhttpredir"
#define SERVICE_DISPLAY_NAME "Tiny HTTP redirector"

int service_init_start();
int service_init();
int service_install();
int service_remove();
int service_start();
int service_stop();

#ifdef _WIN32

void ServiceMain(int argc, char *argv[]);
void ServiceControlHandler(DWORD request);

#endif /* _WIN32 */

#endif /* __WIN32SVC_H__ */
