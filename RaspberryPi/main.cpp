#include <cstdlib>

#include <debug.h>
#include <error_codes.h>

#include <1_LL/tty/tty.hpp>

#include <influx.hpp>
#include <influxline.hpp>
#include <2_/DSMR/DSMR.hpp>
#include <string>

typedef struct {
    char *host;
    char *token;
    char *organisation;
    char *bucket;
} config_t;

/* Constants */

/* Prototypes */
static error_e getConfig(config_t &config);
static error app_run(Tty &tt, influx::Influx &ifx);

/* Public functions */
int main(const int, char *[])
{
    using namespace std;

    config_t config {};
    if (eError_ok != getConfig(config)) {
        exit(EXIT_FAILURE);
    }

    /* TTY Setup */
    std::unique_ptr<Tty> tty = findAndOpenTTYUSB();
    if (!tty)
    {
        DBG_ERR( "Can't find suitable TTY");
        exit(EXIT_FAILURE);
    }

    /* At this point, we found a suitable TTYUSB* and opened it
     * Now setup termios attributes */
    if (setupTTY(tty.get()))
    {
        DBG_ERR("Can't setup TTY");
        exit(EXIT_FAILURE);
    }

    if (eError_ok != influx::init()) {
        DBG_ERR("Influx Init");
        exit(EXIT_FAILURE);
    }
    
    influx::Influx ifx(config.host, config.organisation, config.bucket, config.token);

    app_run(*tty, ifx);

    influx::deinit();

    return EXIT_FAILURE;
}

static error_e getConfig(config_t &config)
{    
    /* Get env variables */
    config.host = getenv("INFLUX_HOST");
    config.token = getenv("INFLUX_TOKEN");
    config.organisation = getenv("INFLUX_ORG");
    config.bucket = getenv("INFLUX_BUCKET");

    if ((config.host == nullptr) || (config.token == nullptr) || (config.organisation == nullptr) || (config.bucket == nullptr))
    {
        DBG_ERR("Environment variables missing!");

        return eError_failed;    
    }

    return eError_ok;
}

static error app_run(Tty &tty, influx::Influx &ifx)
{

    /* Current fetched values */
    influx::InfluxLine currentInfluxLine("meter");
    
    LineReader reader(tty); 

    for (;;)
    {
        const std::string line = reader.read();
        if (line.empty())
        {
            continue;
        }

        decodeLine(currentInfluxLine, line);

        /* If line begins with '!' -> END of Frame */
        if (line.contains('!'))
        {
            const std::string &postLine = currentInfluxLine.getLine();

            /* Post everything to Influx */
            const error_e idxRet = ifx.post(postLine);
            if (eError_ok != idxRet)
            {
                DBG_ERR( "Writing data to InfluxDB failed : %02X -> '%s'", idxRet, postLine.c_str());
            }

            currentInfluxLine.clear();
        }
    }
}
