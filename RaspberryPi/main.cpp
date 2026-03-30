#include <cstdio>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <exception>
#include <string>
#include <vector>

#include "http.hpp"
#include "influx.hpp"

extern "C"
{
#include "common.h"
#include "tty.h"
#include "DSMR.h"
}

/* Constants */
/** line-protocol buffer */
constexpr size_t c_LineBufferSize = 2048U;

/* Prototypes */
int run(int ttyfd, struct influx_config *iconfig);
void ErrorHandler(void);

/* Public functions */
int main(const int , char *[])
{
    using namespace std;

    // std::set_terminate(ErrorHandler);

    setupLogs();

    /* Get env variables */
    const char *host = getenv("INFLUX_HOST");
    const char *token = getenv("INFLUX_TOKEN");
    const char *organisation = getenv("INFLUX_ORG");
    const char *bucket = getenv("INFLUX_BUCKET");

    if ((host == nullptr)|| (token == nullptr) || (organisation == nullptr) || (bucket == nullptr))
    {
        printError(__func__, "Environment variables missing!");

        exit(EXIT_FAILURE);
    }


    /* TTY Setup */
    printLog(__func__, "Finding available TTY");
    int ttyfd = findAndOpenTTYUSB();
    if (ttyfd == -1)
    {
        printError(__func__, "Can't find suitable TTY");
        exit(EXIT_FAILURE);
    }

    /* At this point, we found a suitable TTYUSB* and opened it
     * Now setup termios attributes */
    printLog(__func__, "Setting up TTY");
    ttyfd = setupTTY(ttyfd);
    if (ttyfd == -1)
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
        goto cleanup;
    }

    if (!influx_authenticate(&iconfig))
    {
        printError(__func__, "Couldn't authenticate Influx connection");
        goto cleanup;
    }

    run(ttyfd, &iconfig);

cleanup:
    // Cleanup
    closeTTY(ttyfd);

    return EXIT_FAILURE;
}

int run(int ttyfd, struct influx_config *iconfig)
{
    int ret = 0;

    /* Line protocol handling */
    constexpr size_t bufferLength = 128U;
    std::string lineBuffer;
    lineBuffer.resize(bufferLength);

    int readBytes;

    std::vector<char> influxBuffer;
    influxBuffer.resize(c_LineBufferSize);

    int totalOffset = 0, offset = 0;

    for (;;)
    {
        readBytes = readTTY(ttyfd, lineBuffer.data(), bufferLength);
        if (readBytes < 0)
        {
            printErrno(__func__, "readTTY returned a fatal response!");
            // Fatal
            return -1;
        }
        // TODO:  Maybe a function that resets the DSMR if detecting '/FLU5'
        // If it's not the !CRC, decode line
        offset = decodeLine(influxBuffer.data() + totalOffset, lineBuffer.data(), readBytes);
        totalOffset += offset;

        if (lineBuffer[0] == '!')
        {
            // If line contains the !CRC -> send to Influx
            // Remove last comma
            influxBuffer[totalOffset - 1] = 0;
            influxBuffer.resize(totalOffset);

            printLog(__func__, "Encoded DSMR: '%*.s'", influxBuffer.data(),influxBuffer.size());

            ret = influx_write_DSMR(iconfig, influxBuffer.data(), totalOffset);
            if (!ret)
                printError(__func__, "Writing data to InfluxDB failed: (%dbytes) '%s'", totalOffset, influxBuffer);
            // TODO: After x amount of failures, exit with failure?

            // clear influxBuffer
            influxBuffer.clear();
            totalOffset = 0;
        }
    }
}

void ErrorHandler(void)
{
    printError(__func__, "An exception occured!");
    exit(EXIT_FAILURE);
}