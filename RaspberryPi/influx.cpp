/**
 * influx.c - Handles TCP/IP connection to Influx DB
 * and writes the given data using the line protocol.
 *
 * Consists of connecting to Influx
 *
 * Note: Basic HTTP implementation
 * Note: This wouldn't be implemented on microcontrollers.
 */
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <span>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // getaddrinfo()
#include <arpa/inet.h>
#include <unistd.h> // for write close and read

#include <sys/select.h>
#include <time.h> // select

#include "common.h"
#include "influx.hpp"
#include "http.hpp"

struct influx_config influx_init(
    struct http_config *hconfig,
    const char *organization, const char *bucket, const char *token)
{
    return (struct influx_config){
        .httpConfig = *hconfig,
        .bucket = bucket,
        .organization = organization,
        .token = token,
    };
}

/**
 * Connects to InfluxDB
 * @returns 1 (true) in case we successfulyl connected or else 0
 */
int influx_connect(struct influx_config *config)
{
    if (http_connect(&(config->httpConfig)) == -1)
        return 0;
    return 1;
}

/**
 * Performs HTTP GET Query with token and check if we get a 200 response
 * @returns boolean, 0 in case an error occured, HTTP statuscode in case the connection was authenticated
 */
int influx_authenticate(struct influx_config *config)
{
    return http_get(&(config->httpConfig), "/api/v2/buckets", config->token);
}

/**
 * Performs HTTP POST Query with token and Line protocol data
 * @returns 0 if unsuccessfull HTTP response or malloc error; or the HTTP statuscode
 */
int influx_write_DSMR(influx_config_t *config, std::string line)
{
    /* Query */
    const std::string query = "bucket=" + std::string{config->bucket} + "&org=" + std::string{config->organization} + "&precision=s";

    const std::string measurement = "meter";

    time_t time = convertTimestamp(line.c_str());

    /* Body */
    std::string body = measurement + " " + line.substr(23) + " " + std::to_string(time);

    int ret = http_post(&(config->httpConfig), "/api/v2/write", query, config->token,
                        body);

    return ret;
}

/**
 * Converts meter timestamp=YYMMDDhhmmssX to Unix timestamp
 * Assuming the first element is timestamp
 * //250914143330S
 * //25Y 09M 14d 14h 33m 30s
 */
time_t convertTimestamp(const char *line)
{
    struct tm t;

    char year[3], month[3], day[3], hour[3], minute[3], second[3];

    // Extract components from the timestamp
    const char *ts = line + 10;
    strncpy(year, ts, 2);
    year[2] = '\0';
    strncpy(month, ts + 2, 2);
    month[2] = '\0';
    strncpy(day, ts + 4, 2);
    day[2] = '\0';
    strncpy(hour, ts + 6, 2);
    hour[2] = '\0';
    strncpy(minute, ts + 8, 2);
    minute[2] = '\0';
    strncpy(second, ts + 10, 2);
    second[2] = '\0';

    // Convert year to 2000s
    int yearInt = atoi(year) + 2000 - 1900;

    // Fill timestruct
    t.tm_year = yearInt;
    t.tm_mon = atoi(month) - 1;
    t.tm_mday = atoi(day);
    t.tm_hour = atoi(hour);
    t.tm_min = atoi(minute);
    t.tm_sec = atoi(second);
    t.tm_isdst = 1; // If timestamps from meter are in DST (S) => 1
    // Wintertime should be 0........

#if DEBUG
    printLog(__func__, "%s", line);
    printLog(__func__, "Year %d\tMonth %d\tDay %d\n", t.tm_year, t.tm_mon, t.tm_mday);
    printLog(__func__, "Hour %d\tMinute %d\tSecond %d\n", t.tm_hour, t.tm_min, t.tm_sec);

    printLog(__func__, "time(NULL)=%ld\n", time(NULL));
    printLog(__func__, "mktime(&t)=%ld\n", mktime(&t));
#endif
    return mktime(&t);
}
