#include <stdio.h>

#include "influx.h"

int main(const int argc, char *argv[])
{

    char *token = "trDoZZt96W7zsvTFLBNvryd5q0XkUaBp-mLYRSGA071VKmGN2UhOy2Ol6bq086oX_MRGG4Rzs--YpRzIXVcYmQ==";
    char *db = "";
    char *host = "iot.lan";
    char *bucket = "electricity";
    unsigned short port = 8086;
    int ret;

    influx_config_t config;
    influxInit(&config, host, port, db, bucket, token);
    ret = influxConnect(&config);

    ret = influxAuthenticateAndValidate(&config);
    if (ret)
        printf("Succesfully authenticated to Influx!\n");
    else
        printf("Couldn't authenticated! >:c\n");
    return 0;
}