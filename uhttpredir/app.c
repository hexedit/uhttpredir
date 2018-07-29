#include <app.h>
#include <options.h>
#include <utils.h>
#include <log.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <microhttpd.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef DEBUG
# define LOG_LEVEL LOG_DEBUG
#else
# define LOG_DEVEL LOG_INFO
#endif

static BOOL app_initialized = FALSE;
static uint32_t nlisten = 0;
static struct mhd_info {
    struct MHD_Daemon *mhd;
    char * ssl_cert;
    char * ssl_key;
} *mhd = 0;

static int request_handler(void *cls, struct MHD_Connection *connection,
                    const char *url, const char *method,
                    const char *version, const char *upload_data,
                    size_t *upload_data_size, void **con_cls)
{
    static const char *p404 = "404 Not Found";
    const char *host, *target;
    char *location;
    struct MHD_Response *response;
    int ret;

    host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "Host");
    target = config_get_target(host);

    if (0 == target)
    {
        response = MHD_create_response_from_buffer(
            strlen(p404),
            (void *) p404,
            MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    }
    else
    {
        response = MHD_create_response_from_buffer(0, 0, MHD_RESPMEM_PERSISTENT);
        location = malloc(strlen(target) + strlen(url) + 1);
        if (0 != location)
        {
            strcpy(location, target);
            strcat(location, url);
            MHD_add_response_header(response, "Location", location);
        }
        ret = MHD_queue_response(connection, MHD_HTTP_MOVED_PERMANENTLY, response);
        if (g_options->verbose)
            fprintf(stderr, ">> %s%s -> %s\n", host, url, location);
    }
    if (0 != response)
        MHD_destroy_response(response);

    return ret;
}

static int app_start(void)
{
    int i;
    struct sockaddr_in sin;
    const char * key, * cert;

    nlisten = config_get_nlisten();

    if (0 == nlisten)
    {
        fprintf(stderr, "No listeners defined\n");
        return 1;
    }

    mhd = calloc(nlisten, sizeof(struct mhd_info));
    for (i = 0; i < nlisten; i++)
    {
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = config_get_listen_addr(i);
        sin.sin_port = htons(config_get_listen_port(i));

        if (config_is_listen_secure(i))
        {
            cert = config_get_listen_ssl_cert(i);
            key = config_get_listen_ssl_key(i);

            mhd[i].ssl_cert = load_file(cert);
            if (0 == mhd[i].ssl_cert)
            {
                fprintf(stderr, "Failed to load certificate file: %s\n", cert);
                return 1;
            }

            mhd[i].ssl_key = load_file(key);
            if (0 == mhd[i].ssl_key)
            {
                fprintf(stderr, "Failed to load key file: %s\n", key);
                return 1;
            }

            mhd[i].mhd = MHD_start_daemon(
                MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL, 0,
                NULL, NULL, // accept callback
                &request_handler, (void *) (intptr_t) i,
                MHD_OPTION_SOCK_ADDR, &sin,
                MHD_OPTION_HTTPS_MEM_CERT, mhd[i].ssl_cert,
                MHD_OPTION_HTTPS_MEM_KEY, mhd[i].ssl_key,
                MHD_OPTION_END);
        }
        else
        {
            mhd[i].mhd = MHD_start_daemon(
                MHD_USE_SELECT_INTERNALLY, 0,
                NULL, NULL, // accept callback
                &request_handler, (void *) (intptr_t) i,
                MHD_OPTION_SOCK_ADDR, &sin,
                MHD_OPTION_END);
            mhd[i].ssl_cert = 0;
            mhd[i].ssl_key = 0;
        }

        if (0 != mhd[i].mhd)
            fprintf(stderr, "Running listener %d/%d on %s:%d\n",
                    i+1, nlisten,
                    inet_ntoa(sin.sin_addr),
                    ntohs(sin.sin_port));
        else
            fprintf(stderr, "Failed to start listener on %s:%d\n",
                    inet_ntoa(sin.sin_addr),
                    ntohs(sin.sin_port));
    }

    for(;;)
    {
        if ( !app_initialized ) break;
#ifdef _WIN32
        Sleep(10);
#else
        sleep(10);
#endif
    }

    return 0;
}

static void app_stop(void)
{
    int i;

    if (0 == mhd) return;
    for (i = 0; i < nlisten; i++)
    {
        if (0 != mhd[i].mhd)
            MHD_stop_daemon(mhd[i].mhd);
        if (0 != mhd[i].ssl_cert)
            free(mhd[i].ssl_cert);
        if (0 != mhd[i].ssl_key)
            free(mhd[i].ssl_key);
    }
    free(mhd);
}

int app_init(int argc, char **argv)
{
    int r;

    r = options_parse(argc, argv);
    if (0 != r)
        return r;

    r = config_load();
    if (0 != r)
    {
        fprintf(stderr, "Failed loading configuration\n");
        options_free();
        return r;
    }

    log_init(stderr, LOG_DEVEL);

    app_initialized = TRUE;
    return 0;
}

int app_run(void)
{
    int r;

    if (!app_initialized)
        return 1;

    r = app_start();
    app_stop();

    config_free();
    options_free();
    return r;
}

void app_shutdown(void)
{
    app_initialized = FALSE;
}
