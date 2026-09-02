#include <array>
#include <cstring>
#include <ctime>
#include <utility>

#include <influxline.hpp>
#include "DSMR.hpp"
#include <debug.h>
#include <error_codes.h>

static time_t convertTimestamp(const char *line);

class OID_itf
{
public:
    /** @brief Parses the given \p line */
    virtual void parse(influx::InfluxLine &dest, std::string &line) = 0;

    virtual const std::string &getTitle(void) = 0;
};

/**
 * @brief Date-time stamp of P1 message (0-0:1.0.0*255)
 *
 */
class Timestamp : public OID_itf
{
public:
    Timestamp(std::string title) : m_title{title} {}

    void parse(influx::InfluxLine &dest, std::string &line)
    {
        /* Parse from 'S' or 'W' */
        const std::string timestamp = line.substr(1, line.find_first_of("SW") - 1);
        
        dest.setTimestamp(convertTimestamp(timestamp.c_str()));
    }

    const std::string &getTitle(void) {
        return m_title;
    }

private:
    std::string m_title{};
};

/**
 * @brief 1-0:1.6.0*255
 *(TST)(F5(3,3)) Unit kW
 */
class TimestampedFloat : public OID_itf
{
public:
    TimestampedFloat(std::string title) : m_title{title} {}

    void parse(influx::InfluxLine &dest, std::string &line)
    {
        /*(260330214500S)(03.655*kW) */
        /* Timestamp then value */

        /* Timestamp */
        const std::size_t l = line.find(')');
        const std::string &strTimestmap = line.substr(1, l - 2).c_str();

        dest.addField(
            m_title + std::string("_timestamp"), 
            convertTimestamp(strTimestmap.c_str())
        );
        
        const std::size_t r = line.find('*', l + 1);
        dest.addField(
            m_title + std::string("_value"),
            std::stof(line.substr(l + 2, r - l - 2))
        );
    }

    const std::string &getTitle(void) {
        return m_title;
    }

private:
    std::string m_title{};
};

/**
 * @todo 
 * @brief 0-0:98.1.0*255
 *0-0:98.1.0(7)(1-0:1.6.0)(1-0:1.6.0)(231101000000W)(632525252525W)(00.000*kW)(251001000000S)(250913201500S)(02.508*kW)(251101000000W)(251025123000S)(02.567*kW)(251201000000W)(251106180000W)(02.918*kW)(260101000000W)(251226153000W)(03.743*kW)(260201000000W)(260111151500W)(04.563*kW)(260301000000W)(260211100000W)(03.271*kW)
 */
class MaximumDemandOfLast13Months : public OID_itf
{
public:
    MaximumDemandOfLast13Months(influx::InfluxLine &, std::string title) : m_title{title} {}

    const std::string &getTitle(void) {
        return m_title;
    }

private:
    std::string m_title{};
};

/**
 * @todo convert this to the protocol -> Floating decimal number with fixed number of decimals; see section 6.4
 * @brief 1-0:1.8.1*255 1-0:1.8.2*255
 * 1-0:1.8.1(001209.869*kWh)
 * 1-0:1.8.2(001126.192*kWh)
 * 1-0:2.8.1(001040.710*kWh)
 * 1-0:2.8.2(000468.914*kWh)
 * 
 * @brief 1-0:2.8.1*255 1-0:2.8.2*255
 * 
 *  F9(3,3), tag 6
 * F4(1,1), tag 18
 */
class FloatingPoint : public OID_itf
{
public:
    FloatingPoint(std::string title) : m_title{title} {}

    void parse(influx::InfluxLine &dest, std::string &line)
    {
        const std::size_t l = line.find('*');
        const std::string &value = line.substr(1, l - 1);

        dest.addField(m_title, std::stof(value));
    }

    const std::string &getTitle(void) {
        return m_title;
    }

private:
    std::string m_title{};
};

using oidElem_t = std::pair<std::string, OID_itf *>;

static const std::array<oidElem_t, 15>
    c_OIDMap{
        std::make_pair("0-0:1.0.0", new Timestamp("timestamp")),
        std::make_pair("1-0:1.6.0", new TimestampedFloat("maximum_demand_running_month")),
        // std::make_pair("0-0:98.1.0", std::make_unique<MaximumDemandOfLast13Months>("maximum_demand_last13_months")),
        std::make_pair("1-0:1.8.1", new FloatingPoint("meter_electricity_delivered_to_client_tariff_1")),
        std::make_pair("1-0:1.8.2", new FloatingPoint("meter_electricity_delivered_to_client_tariff_2")),
        std::make_pair("1-0:2.8.1", new FloatingPoint("meter_electricity_delivered_by_client_tariff_1")),
        std::make_pair("1-0:2.8.2", new FloatingPoint("meter_electricity_delivered_by_client_tariff_2")),
        std::make_pair("1-0:21.7.0", new FloatingPoint("instantaneous_active_positive_power_L1")),
        std::make_pair("1-0:41.7.0", new FloatingPoint("instantaneous_active_positive_power_L2")),
        std::make_pair("1-0:61.7.0", new FloatingPoint("instantaneous_active_positive_power_L3")),
        std::make_pair("1-0:22.7.0", new FloatingPoint("instantaneous_active_negative_power_L1")),
        std::make_pair("1-0:42.7.0", new FloatingPoint("instantaneous_active_negative_power_L2")),
        std::make_pair("1-0:62.7.0", new FloatingPoint("instantaneous_active_negative_power_L3")),
        std::make_pair("1-0:1.7.0", new FloatingPoint("actual_electricity_power_delivered")),
        std::make_pair("1-0:2.7.0", new FloatingPoint("actual_electricity_power_received")),
        std::make_pair("0-1:24.2.3", new TimestampedFloat("gas_volume")),
    };

/**
 * processLine parses a given line from DSMR Serial TTY and fills the
 * given DSMR_T
 *
 * Logical names of COSEM objects uses OBIS (object identification system)
 *
 * Raw packet encoding:
 *      / xxx 5 Identification = /FLU 5 253769484_A
 *
 *      Data
 *      !CRC
 * The CRC = CRC16 calculated over the preceding characters
 *  in the data message from / to ! using polynomial,
 *  computed with least significant bit first,
 *  result is a 4 hexadecimal character (MSB first)
 * @returns offset of dstLineBuffer
 */
error_e decodeLine(influx::InfluxLine &dest, const std::string &line)
{
    /* First decode (key) */
    /* Find first '(' */
    std::size_t pos = line.find('(');
    if (pos == std::string::npos)
    {
        return eError_invalid;
    }

    /* Extract OID key substrict */
    std::string oidKey = line.substr(0, pos);

    /* Extract rest of line */
    std::string oidValue = line.substr(pos);
        
    /* Match with map */
    for (auto &p : c_OIDMap)
    {
        if (p.first == oidKey)
        {
            /* Parse the line from the meter */
            p.second->parse(dest, oidValue);

            return eError_ok;
        }
    }

    return eError_invalid;
}

/**
 * Converts meter timestamp=YYMMDDhhmmssX to Unix timestamp
 * Assuming the first element is timestamp
 * //250914143330S
 * //25Y 09M 14d 14h 33m 30s
 */
static time_t convertTimestamp(const char *ts)
{
    char year[3], month[3], day[3], hour[3], minute[3], second[3];

    // Extract components from the timestamp
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
    struct tm t;
    t.tm_year = yearInt;
    t.tm_mon = atoi(month) - 1;
    t.tm_mday = atoi(day);
    t.tm_hour = atoi(hour);
    t.tm_min = atoi(minute);
    t.tm_sec = atoi(second);

    /** @todo check the winter saving time 
     * If timestamps from meter are in DST (S) => 1
     * Wintertime should be 0
     */
    t.tm_isdst = 1;

    return mktime(&t);
}