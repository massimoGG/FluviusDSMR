#include <cstdio>
#include <cstdlib>

#include <lib/common.h>
#include <lib/error_codes.h>

#include <1_LL/tty/tty.hpp>

#include <2_/database/influx.hpp>
#include <2_/database/influxline.hpp>
#include <2_/DSMR/DSMR.hpp>

typedef struct {
    char *host;
    char *token;
    char *organisation;
    char *bucket;
} config_t;

/* Constants */
/** Length of DSMR line */
constexpr size_t c_DsmrLineLength = 512U;

/* Prototypes */
static error_e getConfig(config_t &config);
static int app_run(std::unique_ptr<Tty> tty, influx::Influx &ifx);

/* Public functions */
int main(const int, char *[])
{
    using namespace std;

    setupLogs();

    config_t config {};
    if (eError_ok != getConfig(config)) {
        exit(EXIT_FAILURE);
    }

    /* TTY Setup */
    std::unique_ptr<Tty> tty = findAndOpenTTYUSB();
    if (!tty)
    {
        printError(__func__, "Can't find suitable TTY");
        exit(EXIT_FAILURE);
    }

    /* At this point, we found a suitable TTYUSB* and opened it
     * Now setup termios attributes */
    if (setupTTY(tty.get()))
    {
        printError(__func__, "Can't setup TTY");
        exit(EXIT_FAILURE);
    }

    influx::Influx ifx(config.host, config.organisation, config.bucket, config.token);

    app_run(std::move(tty), ifx);

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
        printError(__func__, "Environment variables missing!");

        return eError_failed;    
    }

    return eError_ok;
}

static int app_run(std::unique_ptr<Tty> tty, influx::Influx &ifx)
{
    /* Temporary buffer for DSMR line */
    std::string totalBuffer{""};

    /* Total buffer to send to Influx */
    std::string influxBuffer{""};

    for (;;)
    {
        /* Current Line handling */
        {
            std::string tempData;
            tempData.resize(c_DsmrLineLength);
            
            const int readBytes = readTTY(tty.get(), {tempData.data(), tempData.size()});
            if (readBytes < 0)
            {
                printErrno(__func__, "readTTY returned a fatal response!");
                return -1;
            }

            /* Resize string to exact number of received bytes */
            tempData.resize(readBytes);

            /* Append data to the global buffer */
            totalBuffer += tempData;
        }

        /* If there's no new line in buffer, continue reading from TTY */
        if (!totalBuffer.contains('\n'))
        {
            continue;
        }

        /* At this point, we got a new line in our buffer! */
        /* Take the string and remove from dsmrBuffer */
        size_t newLineIndex = totalBuffer.find('\n');
        std::string dsmrLine = totalBuffer.substr(0, newLineIndex);

        decodeLine(influxBuffer, dsmrLine);

        /* Remove dsmrLine +1(for new Line) from totalBuffer */
        totalBuffer.erase(0, newLineIndex + 1);

        /* If line begins with '!' -> END of Frame -> Clear everything */
        if (dsmrLine.contains('!'))
        {
#if DEBUG
            printf("InfluxBuffer: %s\n\n", influxBuffer.c_str());
#endif
            /* Remove the last ',' because influx doesn't like that */
            influxBuffer.pop_back();

            /* Post everything to Influx */
            if (ifx.post(influxBuffer))
            {
                printError(__func__, "Writing data to InfluxDB failed '%s'", influxBuffer.c_str());
            }

            influxBuffer.clear();
        }
    }
}
