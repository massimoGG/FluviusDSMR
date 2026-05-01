#ifndef INFLUX_H
#define INFLUX_H

#include "http.hpp"
#include <string>

typedef struct influx_config
{
    struct http_config httpConfig;

    const char *bucket;
    const char *organization;
    const char *token;

} influx_config_t;

struct influx_config influx_init(
    struct http_config *hconfig,
    const char *organization, const char *bucket, const char *token);

int influx_connect(struct influx_config *config);
int influx_authenticate(struct influx_config *config);
int influx_write_DSMR(influx_config_t *config, std::string line);

time_t convertTimestamp(const char *line);

#endif