#ifndef HTTP_H
#define HTTP_H

#include <string>

struct http_config
{
    int sockfd;
    const char *remote_host;
    unsigned short remote_port;
};

struct http_response
{
    char *request_uri;
    char *body;
    char status_code;
    char *status_text;
};

struct http_config http_init(const char *host, unsigned short port);
int http_connect(struct http_config *config);
int http_get(struct http_config *config, const char *uri, const char *token);
int http_post(struct http_config *config, const std::string uri, const std::string query, const std::string token,
              const std::string postData);

#endif