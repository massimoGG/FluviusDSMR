#include <cstdio>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <exception>
#include <string>
#include <vector>
#include <span>

#include "http.hpp"
#include "influx.hpp"
#include "tty.hpp"

#include "DSMR.hpp"

#include "common.h"

/* Constants */
/** Length of DSMR line */
constexpr size_t c_DsmrLineLength = 512U;

/** line-protocol buffer */
constexpr size_t c_LineBufferSize = 2048U;

/* Prototypes */
int run(std::unique_ptr<Tty> tty, struct influx_config *iconfig);
void ErrorHandler(void);

/* Public functions */
int main(const int, char *[])
{
    using namespace std;

    // std::set_terminate(ErrorHandler);

    setupLogs();

    /* Get env variables */
    const char *host = getenv("INFLUX_HOST");
    const char *token = getenv("INFLUX_TOKEN");
    const char *organisation = getenv("INFLUX_ORG");
    const char *bucket = getenv("INFLUX_BUCKET");

    if ((host == nullptr) || (token == nullptr) || (organisation == nullptr) || (bucket == nullptr))
    {
        printError(__func__, "Environment variables missing!");

        exit(EXIT_FAILURE);
    }

    /* TTY Setup */
    printLog(__func__, "Finding available TTY");
    std::unique_ptr<Tty> tty = findAndOpenTTYUSB();
    if (!tty)
    {
        printError(__func__, "Can't find suitable TTY");
        exit(EXIT_FAILURE);
    }

    /* At this point, we found a suitable TTYUSB* and opened it
     * Now setup termios attributes */
    printLog(__func__, "Setting up TTY");
    if (setupTTY(tty.get()))
    {
        printError(__func__, "Can't setup TTY");
        exit(EXIT_FAILURE);
    }

    /* InfluxDB connection setup */
    printLog(__func__, "Setting up Influx HTTP connection");
    struct http_config hconfig = http_init(host, 8086);
    int ret = http_connect(&hconfig);
    if (ret == -1)
    {
        printError(__func__, "HTTP connection to Influx failed");
        exit(EXIT_FAILURE);
    }

    printLog(__func__, "Connection established");

    // At this point we've got an established HTTP connection

    struct influx_config iconfig = influx_init(&hconfig, organisation, bucket, token);
    // Now validate connection
    if (!influx_connect(&iconfig))
    {
        printError(__func__, "Couldn't connect to server");
        exit(EXIT_FAILURE);
    }

    if (!influx_authenticate(&iconfig))
    {
        printError(__func__, "Couldn't authenticate Influx connection");
        exit(EXIT_FAILURE);
    }

    run(std::move(tty), &iconfig);

    return EXIT_FAILURE;
}

int run(std::unique_ptr<Tty> tty, struct influx_config *iconfig)
{
    /* Temporary buffer for DSMR line */
    std::string totalBuffer{""};

    /* Total buffer to send to Influx */
    std::string influxBuffer{""};

    for (;;)
    {
        /* Current Line handling */
        std::string tempData;
        {
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

            if (!influx_write_DSMR(iconfig, influxBuffer))
            {
                printError(__func__, "Writing data to InfluxDB failed '%s'", influxBuffer.c_str());
            }

            influxBuffer.clear();
        }
    }
}

void ErrorHandler(void)
{
    printError(__func__, "An exception occured!");
    exit(EXIT_FAILURE);
}