#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <common.h>

typedef struct uhttpredir_options {
    int daemon;
    int verbose;
    int loglevel;
    char *config;
    char *pidfile;
#ifdef _WIN32
    int win32_install_service;
    int win32_remove_service;
    int win32_start_service;
    int win32_stop_service;
#endif
} * uhr_options_t;

extern uhr_options_t g_options;

int options_parse(int argc, char ** argv);
void options_free(void);

int config_load(void);
void config_free(void);
uint32_t config_get_nlisten(void);
uint32_t config_get_listen_addr(uint32_t idx);
uint16_t config_get_listen_port(uint32_t idx);
int config_is_listen_secure(uint32_t idx);
const char * config_get_listen_ssl_key(uint32_t idx);
const char * config_get_listen_ssl_cert(uint32_t idx);
const char * config_get_target(const char * host);

#endif /* __OPTIONS_H__ */
