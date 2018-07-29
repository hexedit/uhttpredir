#include <options.h>
#include <utils.h>

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <json.h>

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef _WIN32
# include <win32inc.h>
#endif

static struct uhttpredir_options l_options = { 1, 0, 0 };
static char * l_config_path;
static struct json_object * l_config;

uhr_options_t g_options = &l_options;

static void options_init(void)
{
    l_options.daemon = 0;
    l_options.verbose = 0;
    l_options.loglevel = 3;
    l_options.config = 0;

    l_options.pidfile = malloc(FILENAME_MAX);
    strncpy(l_options.pidfile, LOCALSTATEDIR, FILENAME_MAX);
    strncat(l_options.pidfile, "/run/uhttpredir.pid", FILENAME_MAX);
}

static void set_config_path(const char * path)
{
    if (0 != l_config_path)
    {
        config_free();
        free(l_config_path);
    }
    if (0 == path)
    {
        l_config_path = 0;
        return;
    }
    l_config_path = malloc(strlen(path) + 1);
    strcpy(l_config_path, path);
}

static void set_pidfile(const char * path)
{
    if (0 != l_config_path)
    {
        config_free();
        free(l_config_path);
    }
    if (0 == path)
    {
        l_config_path = 0;
        return;
    }
    l_config_path = malloc(strlen(path) + 1);
    strcpy(l_config_path, path);
}

static void show_help()
{
    // TODO usage help
}

int options_parse(int argc, char ** argv)
{
    int c;

    options_init();

    for (;;)
    {
        static struct option long_options[] = {
            { "help",      no_argument,       0, '?' },
            { "verbose",   no_argument,       0, 'v' },
            { "daemon",    no_argument,       0, 'D' },
            { "config",    required_argument, 0, 'c' },
            { "pidfile",   required_argument, 0, 'P' },
#ifdef _WIN32
            {"install-service", no_argument, &l_options.win32_install_service, 1},
            {"remove-service", no_argument, &l_options.win32_remove_service, 1},
            {"start-service", no_argument, &l_options.win32_start_service, 1},
            {"stop-service", no_argument, &l_options.win32_stop_service, 1},
#endif
            { 0, 0, 0, 0 }
        };
        int index = 0;

        c = getopt_long(argc, argv, "?vDc:P:", long_options, &index );
        if (-1 == c)
            break;
        
        switch (c)
        {
            case '?':
                show_help();
                return 1;
            case 0:
                break;
            case 'D':
                l_options.daemon = 1;
                break;
            case 'v':
                l_options.verbose = 1;
                break;
            case 'c':
                set_config_path(optarg);
                break;
            case 'P':
                set_pidfile(optarg);
                break;
            default:
                fprintf(stderr, "Invalid option: %c\n", c);
                return 1;
        }
    }

    return 0;
}

void options_free(void)
{
    set_config_path(0);
}

int config_load(void)
{
    char * config;

    if (0 != l_config)
        config_free();
    if (0 == l_config_path )
    {
        l_config_path = malloc(FILENAME_MAX);
        snprintf(l_config_path, FILENAME_MAX, "%s/config.json", SYSCONFDIR);
    }

    config = load_file(l_config_path);
    if (0 != config)
    {
        l_config = json_tokener_parse(config);
        free(config);
    }

    return l_config == 0;
}

void config_free(void)
{
    json_object_put(l_config);
    l_config = 0;
}

uint32_t config_get_nlisten(void)
{
    struct json_object * listen;
    listen = json_object_object_get(l_config, "listen");
    if (0 == listen) return 0;
    if (!json_object_is_type(listen, json_type_array)) return 0;
    return json_object_array_length(listen);
}

static struct json_object * config_get_listen(uint32_t idx)
{
    struct json_object * listen,
                       * item;
    listen = json_object_object_get(l_config, "listen");
    if (0 == listen) return 0;
    if (!json_object_is_type(listen, json_type_array)) return 0;
    item = json_object_array_get_idx(listen, idx);
    if (0 == item) return 0;
    if (!json_object_is_type(item, json_type_object)) return 0;
    return item;
}

uint32_t config_get_listen_addr(uint32_t idx)
{
    static const char * def = "0.0.0.0";
    struct json_object * listen,
                       * address;
    const char * addr = def;
    listen = config_get_listen(idx);
    if (0 != listen)
    {
        address = json_object_object_get(listen, "address");
        if (0 != address)
            addr = json_object_get_string(address);
    }
    return inet_addr(addr);
}

uint16_t config_get_listen_port(uint32_t idx)
{
    static uint16_t def = 80;
    struct json_object * listen,
                       * port;
    listen = config_get_listen(idx);
    if (0 == listen) return def;
    port = json_object_object_get(listen, "port");
    if (0 == port) return def;
    return json_object_get_int(port) & 0xffff;
}

int config_is_listen_secure(uint32_t idx)
{
    struct json_object * listen,
                       * secure;
    listen = config_get_listen(idx);
    if (0 == listen) return 0;
    secure = json_object_object_get(listen, "secure");
    if (0 == secure) return 0;
    return json_object_get_boolean(secure);
}

const char * config_get_listen_ssl_key(uint32_t idx)
{
    struct json_object * listen,
                       * key;
    listen = config_get_listen(idx);
    if (0 == listen) return 0;
    key = json_object_object_get(listen, "key");
    if (0 == key) return 0;
    if (!json_object_is_type(key, json_type_string)) return 0;
    return json_object_get_string(key);
}

const char * config_get_listen_ssl_cert(uint32_t idx)
{
    struct json_object * listen,
                       * cert;
    listen = config_get_listen(idx);
    if (0 == listen) return 0;
    cert = json_object_object_get(listen, "cert");
    if (0 == cert) return 0;
    if (!json_object_is_type(cert, json_type_string)) return 0;
    return json_object_get_string(cert);
}

const char * config_get_target(const char * host)
{
    struct json_object * targets,
                       * target;
    targets = json_object_object_get(l_config, "targets");
    if (0 == targets) return 0;
    if (!json_object_is_type(targets, json_type_object)) return 0;
    target = json_object_object_get(targets, host);
    if (0 == target) return 0;
    if (!json_object_is_type(target, json_type_string)) return 0;
    return json_object_get_string(target);
}
