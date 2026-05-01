/**
 * http.c - Handles everything on HTTP layer
 *
 * Usage:
 * struct http_config config = http_init("ADDRESS", 8086);
 * int ret = http_connect(&config);
 *
 */
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // getaddrinfo()
#include <arpa/inet.h>
#include <unistd.h> // for write close and read

#include <sys/select.h>
#include <time.h> // select

#include "common.h"
#include "http.hpp"

/* Constants */
constexpr const size_t c_HttpTimeout = 5U;
constexpr const size_t c_maxHeaderSize = 512U;
constexpr const size_t c_maxBodySize = 2048U;

/* Prototypes */
int checkHTTPCode(std::string &s);
int sread(int fd, void *buf, size_t nbytes, int timeout);

struct http_config http_init(const char *host, unsigned short port)
{
    // size_t slen = strlen(host);
    // config->remote_host = malloc(slen + 1);
    // strncpy(config->remote_host, host, slen);
    // config->remote_host[slen] = 0;

    return (struct http_config){
        .sockfd = -1,
        .remote_host = host,
        .remote_port = port,
    };
}

/**
 * influxConnect connects with the first possible socket to InfluxDB
 * @returns the socket fd of the connection
 */
int http_connect(struct http_config *config)
{
    // Find suitable socket
    char service[6];
    sprintf(service, "%d", config->remote_port); // I hate this

    struct addrinfo hints = {
                        .ai_flags = 0,
                        .ai_family = AF_UNSPEC, // IPv4 or IPv6, whatever
                        .ai_socktype = SOCK_STREAM,
                        .ai_protocol = 0,
                        .ai_addrlen = 0,
                        .ai_addr = nullptr,
                        .ai_canonname = nullptr,
                        .ai_next = nullptr,
                    },
                    *servinfo, *sip;

    int ret;
    if ((ret = getaddrinfo(config->remote_host, service, &hints, &servinfo)) == -1)
    {
        printErrno(__func__, "getaddrinfo failed");
        return -1;
    }

    // Fetch first possible socket from servinfo
    int sockfd = -1;
    for (sip = servinfo; sip != NULL; sip = sip->ai_next)
    {
        if ((sockfd = socket(sip->ai_family, sip->ai_socktype, sip->ai_protocol)) == -1)
        {
            sockfd = -1;
            continue;
        }

        // Now try to connect
        if (connect(sockfd, sip->ai_addr, sip->ai_addrlen) == -1)
        {
            // Convert for debug
            char ip[INET6_ADDRSTRLEN];
            struct sockaddr_in *adr4 = (struct sockaddr_in *)sip->ai_addr;
            void *adr = &(adr4->sin_addr);
            inet_ntop(sip->ai_family, adr, ip, INET6_ADDRSTRLEN);

            printErrno(__func__, "Connection failed to %s:%d\n", ip, ntohs(adr4->sin_port));
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (sockfd == -1)
    {
        printErrno(__func__, "couldn't connect to server");
        return -1;
    }

    config->sockfd = sockfd;

    return sockfd;
}

/**
 * @returns int status code; 0 (false) on error
 */
int http_get(struct http_config *config, const char *uri, const char *token)
{
    std::string header{""};
    header.resize(c_maxHeaderSize);

    // Construct the headers
    size_t headerLength = sprintf(header.data(), "GET %s HTTP/1.1\r\n"
                                                 "Host: %s:%d\r\n"
                                                 "Connection: keep-alive\r\n"
                                                 "Authorization: Token %s\r\n\r\n",
                                  uri, config->remote_host, config->remote_port, token);
    header.resize(headerLength);

    /* Write */
    ssize_t nsent = write(config->sockfd, header.data(), header.size());
    if (nsent <= 0)
    {
        return 0; // error or closed connection
    }

    /* Read the HTTP response */
    std::string response;
    response.resize(c_maxBodySize);
    size_t read_len = 0;
    do
    {
        read_len += sread(config->sockfd, response.data() + read_len, c_maxBodySize, c_HttpTimeout);

        /* If we reached max size */
        if (read_len % c_maxBodySize == 0)
        {
            /* Increase size */
            response.resize(response.capacity() + c_maxBodySize);
        }

    } while (read_len > c_maxBodySize);

    /* If an error occured */
    if (read_len <= 0)
    {
        return 0;
    }

    int status_code = checkHTTPCode(response);

    return status_code;
}

/** @returns int status code; 0 (false) on error */
int http_post(
    struct http_config *config, const std::string uri, const std::string query, const std::string token,
    const std::string postData)
{
    {
        std::string totalBuffer = "POST " + uri + "?" + query + " HTTP/1.1\r\n";
        totalBuffer += "Host: " + std::string{config->remote_host} + ":" + std::to_string(config->remote_port) + "\r\n";
        totalBuffer += "Connection: keep-alive\r\n";
        totalBuffer += "Content-Length: " + std::to_string(postData.size()) + "\r\n";
        totalBuffer += "Authorization: Token " + token + "\r\n\r\n";

        /* copy post_data at offset due to header */
        totalBuffer += postData;

        // Write
        int nsent = write(config->sockfd, totalBuffer.data(), totalBuffer.size());
        if (nsent <= 0)
        {
            return 0; // error or closed connection
        }
    }

    /* read response */
    {
        std::string reply;
        reply.resize(c_maxBodySize);

        int retRead = 0;
        do
        {
            reply.resize(reply.size() + c_maxBodySize);

            char *dst = reply.data() + reply.size() - c_maxBodySize;
            int retRead = sread(config->sockfd, dst, c_maxBodySize, c_HttpTimeout);
            if (retRead <= 0)
            {
                return 0;
            }

            /* Shrink */
            reply.resize(reply.size() - c_maxBodySize + retRead);

        } while (retRead % c_maxBodySize != 0);

        int status_code = checkHTTPCode(reply);

        return status_code;
    }
}

/**
 * checkHTTPCode parses the given HTTP response string and extracts the HTTP code
 * @returns HTTP code
 */
int checkHTTPCode(std::string &str)
{
    // HTTP/1.1 401 Unauthorized or HTTP/1.1 200 OK
    size_t firstSpace = str.find(" ");
    size_t secondSpace = str.find(" ", firstSpace + 1);

    std::string code = str.substr(firstSpace, secondSpace - firstSpace);

    return atoi(code.c_str());
}

/// @brief Block reads from fd for timeout seconds
/// @param fd
/// @param buf
/// @param nbytes
/// @param timeout
/// @return bytes read
int sread(int fd, void *buf, size_t nbytes, int timeout)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    struct timeval tv = {
        .tv_sec = timeout,
        .tv_usec = 0,
    };

    int ret = select(fd + 1, &set, NULL, NULL, &tv);
    if (ret == -1)
    {
        printErrno(__func__, "select error");
        return -1;
    }
    if (ret == 0)
    {
        printError(__func__, "Timeout!");
        return 0;
    }

    // Read the reply
    int nread = read(fd, buf, nbytes);
    return nread;
}
