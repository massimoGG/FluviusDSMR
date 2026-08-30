#ifndef __INFLUX_HPP__
#define __INFLUX_HPP__

#include <string>

#include <lib/error_codes.h>
#include <curl/curl.h>
#include <string_view>


namespace influx {

class Influx
{
public:
    /* Delete default constructor */
    Influx() = delete;

    explicit Influx(const std::string &host, const std::string &org, const std::string &bucket, const std::string &token);
    explicit Influx(const std::string &host, const unsigned short port, const std::string &org, const std::string &bucket, const std::string &token);

    ~Influx();

    error_e post(const std::string &data);

private:
    error_e configCurl(const std::string &token);

    /** The HTTP buffer size */
    static const unsigned int s_bufsize;

    std::string m_url;
    CURL *m_curl;
    struct curl_slist *m_curl_headers;
};

error_e init(void);
error_e deinit(void);

};

#endif
