#include "DSMR.h"
#include "common.h"
#include "hash.h"

struct hashkeyval OIDMap[] = {
    {.hash = DATE_TIME_STAMP,
     .name = "timestamp",
     .namelen = 9,
     .type = TIMESTAMP,
     .next = 0},

    {.hash = MAXIMUM_DEMAND_RUNNING_MONTH,
     .name = "maximum_demand_running_month_timestamp",
     .namelen = 38,
     .type = TIMESTAMP,
     .next = 2},
    // WARN: Update the previous next value to this index if you're adding keys!
    {.hash = 0, // do not care
     .name = "maximum_demand_running_month_value",
     .namelen = 34,
     .type = DOUBLE_LONG,
     .next = 0},

    {.hash = METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_1,
     .name = "meter_electricity_delivered_to_client_tariff_1",
     .namelen = 46,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_2,
     .name = "meter_electricity_delivered_to_client_tariff_2",
     .namelen = 46,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_1,
     .name = "meter_electricity_delivered_by_client_tariff_1",
     .namelen = 46,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_2,
     .name = "meter_electricity_delivered_by_client_tariff_2",
     .namelen = 46,
     .type = DOUBLE_LONG,
     .next = 0},

    {.hash = ACTUAL_ELECTRICITY_POWER_DELIVERED,
     .name = "actual_electricity_power_delivered",
     .namelen = 34,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = ACTUAL_ELECTRICITY_POWER_RECEIVED,
     .name = "actual_electricity_power_received",
     .namelen = 33,
     .type = DOUBLE_LONG,
     .next = 0},

    // {.hash = INSTANTANEOUS_VOLTAGE_L1,
    //  .name = "instantaneous_voltage_L1",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},
    // {.hash = INSTANTANEOUS_VOLTAGE_L2,
    //  .name = "instantaneous_voltage_L2",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},
    // {.hash = INSTANTANEOUS_VOLTAGE_L3,
    //  .name = "instantaneous_voltage_L3",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},
    //
    // {.hash = INSTANTANEOUS_CURRENT_L1,
    //  .name = "instantaneous_current_L1",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},
    // {.hash = INSTANTANEOUS_CURRENT_L2,
    //  .name = "instantaneous_current_L2",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},
    // {.hash = INSTANTANEOUS_CURRENT_L3,
    //  .name = "instantaneous_current_L3",
    //  .namelen = 24,
    //  .type = DOUBLE_LONG,
    //  .next = 0},

    {.hash = INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L1,
     .name = "instantaneous_active_positive_power_L1",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L2,
     .name = "instantaneous_active_positive_power_L2",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L3,
     .name = "instantaneous_active_positive_power_L3",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L1,
     .name = "instantaneous_active_negative_power_L1",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L2,
     .name = "instantaneous_active_negative_power_L2",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},
    {.hash = INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L3,
     .name = "instantaneous_active_negative_power_L3",
     .namelen = 38,
     .type = DOUBLE_LONG,
     .next = 0},

};


void cpy(void *dst, void *src, int byte_count)
{
    char *cdst = dst, *csrc = src;

    while (byte_count--)
        *cdst++ = *csrc++;
}

/**
 * decodeOBISHashKey iterates over the line and finds the
 * indices of the end of the Key and calculates the hash
 */
void decodeOBISHashKey(
    char *line,
    int lineLength,
    int *OIDIndexEnd,
    unsigned short *OIDKeyHash)
{
    // Key
    for (int i = 0; i < lineLength; i++)
    {
        if (line[i] == '(')
        {

            // Key ended one character before
            *OIDIndexEnd = i - 1;
            break;
        }

        // If we're still inside the OID key -> Calculate hash
        // Note: This is some random hash I quickly made to be as fast as possible
        *OIDKeyHash = line[i] - *OIDKeyHash * i;
    }
}

/**
 * findOBISOIDByHash returns
 * @returns the index of the OID if the hash was found or -1
 */
int findOBISOIDByHash(unsigned short hash)
{
#define OIDMapLen sizeof(OIDMap) / sizeof(struct hashkeyval)

    for (long unsigned int i = 0; i < OIDMapLen; i++)
    {
        if (OIDMap[i].hash == hash)
        {
            return i;
        }
    }
    return -1;
}

/**
 * writeKey copies the key to dst with length length and appends =
 * @returns the offset index
 */
int writeKey(char *dst, char *key, int length)
{
#if DEBUG
printLog(__func__, "writeKey ");
printNum(key, length);
#endif
    cpy(dst, key, length);
    dst[length] = '=';
    return length + 1;
}

/**
 * writeValue copies the value to dst with length length and appends ,
 * @returns the offset index
 */
int writeValue(char *dst, char *value, int length)
{
#if DEBUG
printLog(__func__, "writeValue ");
printNum(value, length);
#endif

    cpy(dst, value, length);
    dst[length] = ',';
    return length + 1;
}

/**
 * @returns offset
 */
int fetchValue(COSEMType type, char *line, int lineLength, int *nextValue)
{
    int characterOffset = 0;

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
int decodeLine(char *dstLineBuffer, char *line, int lineLength)
{
    int OIDLength = -1;
    unsigned short keyHash = 0;
    decodeOBISHashKey(line, lineLength, &OIDLength, &keyHash);
    if (OIDLength == -1)
    {
#if DEBUG
        printError(__func__, "\tCouldn't identify line %s! Corrupted?\n", line);
#endif
        return 0;
    }

#if DEBUG
    printLog(__func__, "LINE '%s'", line);

    // Debug Print Key
    line[OIDLength + 1] = 0;
    printLog(__func__, "OID Key [%s] = hash '%d'", line, keyHash);
#endif

    // Index in hashMap
    int kvIndex = findOBISOIDByHash(keyHash);
    if (kvIndex == -1)
    {
        return 0;
    }

    // Pointer moved across the line
    char *remainingLine = line;

    // outputOffset is the offset on 'dstLinebuffer'
    int outputOffset = 0;

    // valueLength is the string length of the value
    int valueLength = 0;

    // nextValueOffset points to the first character of the next value
    int nextValueOffset = OIDLength + 2;

    do
    {
        // the key
        struct hashkeyval *kv = OIDMap + kvIndex;
#if DEBUG
        printLog(__func__, "iteration\thashkeyval is '%s'", kv->name);
#endif

        // Update the buffer pointer
        remainingLine += nextValueOffset;

        // the value
        valueLength = fetchValue(
            kv->type,                     // can get changed when there's a second value
            remainingLine,                // Pointer to first character of value
            lineLength - nextValueOffset, // Calculate remaining length based on pointer offset
            &nextValueOffset);
        if (valueLength == 0)
        {
            break;
        }
        outputOffset += writeKey(dstLineBuffer + outputOffset, kv->name, kv->namelen);
        outputOffset += writeValue(dstLineBuffer + outputOffset, remainingLine, valueLength);

        if (!kv->next)
            break;

        // Point to the next one to decode that value
        kvIndex = kv->next;
    } while (nextValueOffset < lineLength);

    return outputOffset;
}
