#include <array>
#include <cstring>
#include <memory>
#include <utility>

#include <lib/common.h>
#include "DSMR.hpp"

class OID_itf
{
public:
    /** @brief Parses the given \p line */
    virtual void parse(std::string &line) = 0;
    
    /** @brief Gets an Influx readable */
    virtual std::string get(void) = 0;
};

/**
 * @brief Date-time stamp of P1 message (0-0:1.0.0*255)
 *
 */
class Timestamp : public OID_itf
{
public:
    Timestamp(std::string title) : m_title{title} {}

    void parse(std::string &line)
    {
        /* Parse from 'S' or 'W' */
        m_value = m_title + "=" + line.substr(1, line.find_first_of("SW")-1);
    }

    std::string get(void)
    {
        return m_value;
    }

private:
    std::string m_title{};
    std::string m_value{};
};

/**
 * @brief 1-0:1.6.0*255
 *(TST)(F5(3,3)) Unit kW
 */
class TimestampedFloat : public OID_itf
{
public:
    TimestampedFloat(std::string title) : m_title{title} {}

    void parse(std::string &line)
    {
        /*(260330214500S)(03.655*kW) */
        /* Timestamp then value */

        /* Timestamp */
        std::size_t l = line.find(')');
        /* Timestamp */
        m_value = m_title + "_timestamp=" + line.substr(1, l - 2);

        m_value += ","+m_title+"_value=";

        std::size_t r = line.find('*', l + 1);
        m_value += line.substr(l + 2, r - l - 2);
    }

    std::string get(void)
    {
        return m_value;
    }

private:
    std::string m_title{};
    std::string m_value{};
};

/**
 * @todo 
 * @brief 0-0:98.1.0*255
 *0-0:98.1.0(7)(1-0:1.6.0)(1-0:1.6.0)(231101000000W)(632525252525W)(00.000*kW)(251001000000S)(250913201500S)(02.508*kW)(251101000000W)(251025123000S)(02.567*kW)(251201000000W)(251106180000W)(02.918*kW)(260101000000W)(251226153000W)(03.743*kW)(260201000000W)(260111151500W)(04.563*kW)(260301000000W)(260211100000W)(03.271*kW)
 */
class MaximumDemandOfLast13Months : public OID_itf
{
public:
    MaximumDemandOfLast13Months(std::string title) : m_title{title} {}

    void parse(std::string &)
    {
    }

    std::string get(void)
    {
        return m_value;
    }

private:
    std::string m_title{};
    std::string m_value{};
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

    void parse(std::string &line)
    {
        std::size_t l = line.find('*');
        m_value = m_title + "=" + line.substr(1, l - 1);
    }

    std::string get(void)
    {
        return m_value;
    }

private:
    std::string m_title{};
    std::string m_value{};
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


#if 0
int fetchValue(COSEMType type, const char *line, size_t lineLength, size_t *nextValue)
{
    size_t characterOffset = 0;

    if (type == DOUBLE_LONG)
    {
        characterOffset = getByToken(line, lineLength, 0, '*');
        *nextValue = characterOffset + 2;
        return characterOffset;
    }
    if (type == TIMESTAMP)
    {
        // Daylight Saving Time
        // Active: S
        // Not active: W
        characterOffset = getByToken(line, lineLength, 0, 'S');
        if (characterOffset == lineLength)
        {
            // S not found -> W
            characterOffset = getByToken(line, lineLength, 0, 'W');
        }

        *nextValue = characterOffset + 3;
        return characterOffset;
    }
    if (type == BIT_STRING)
        return getByToken(line, lineLength, 0, ')');

    return 0;
}
#endif

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
int decodeLine(std::string &destBuffer, std::string &line)
{
    /* First decode (key) */
    /* Find first '(' */
    std::size_t pos = line.find('(');
    if (pos == std::string::npos)
    {
        return 0;
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
            /* Parse the argument to its appropriate type */
            p.second->parse(oidValue);

            /* Get the decoded buffer */
            destBuffer += p.second->get();
            destBuffer.push_back(',');

            return 1;
        }
    }

    return 0;
}
