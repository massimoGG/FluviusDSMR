#ifndef DSMR_H
#define DSMR_H

#ifdef __cplusplus
extern "C" {
#endif
/**
 * OBIS Types
 * Floating point Fn(x,y);
 *  x = minimum of decimals;
 *  y = maximum of decimals
 *  Fn(0,3) = minumum of 0, max of 3 decimals
 * In = Integer number
 *  I4 = YYYY = 4 decimals
 * Sn = Alphanumeric string
 *  S6 = CCCCCC = 6
 * TST = YYMMDDhhmmssX (X=S if DST is active) or
 *  (X=W if DST is not active
 *  DST = Daylight Saving Time
 *
 * COSEM object attributes:
 * | Tag | COSEM Data Type | Value Format |
 * |  0  | null-data       | Empty        |
 * |  3  | boolean         | I1           |
 * |  4  | bit-string      | Sn           |
 * |  5  | double-long     | Fn(x,y)      |
 * |  6  | double long unsigned| Fn(x,y)  |
 * |  7  | floating-point  | Fn(x,y)      |
 * |  9  | octet-string    | Sn           |
 * |  10 | visible-string  | Sn           |
 * |  13 | bcd             | S2           |
 * |  15 | integer         | In           |
 * |  16 | long            | Fn(x,y)      |
 * |  17 | unsigned        | Fn(x,y)      |
 * |  18 | long-unsigned   | Fn(x,y)      |
 * |  20 | long64          | Fn(x,y)      |
 * |  21 | long64-unsigned | Fn(x,y)      |
 * |  22 | enum            | Fn(x,y)      |
 * |  23 | float-32        | Fn(x,y)      |
 * |  24 | float-64        | Fn(x,y)      |
 *
 * Representation of COSEM objects
 *  ID (Mv*U)
 *  1  23  45
 * 1) OBIS reduced ID-code
 * 2) Separator ( ASCII 28
 * 3) COSEM object attribute value
 * 4) Unit of measurement values
 * 5) Separator ) ASCII 29
 *
 * The following table holds data objects represented with P1 Interface
 *  together with OBIS ref-erence including object Attribute and Value
 *  Format for Reduced ID codes. Every line is ended with a CR/LF
 * (Carriage Return / Line Feed).
 */

typedef enum
{
    BIT_STRING = 4,
    DOUBLE_LONG = 5,
    BIT_STRING_DOUBLE = 25,
    TIMESTAMP = 26,
    TIMESTAMP_DOUBLE = 27,
} COSEMType;

/**
 * OID Mapping enum
 */
typedef enum
{
    DATE_TIME_STAMP = 47216,                                        // "0-0:1.0.0" // TST
    EQUIPMENT_IDENTIFIER = 3867,                                    // "0-0:96.1.1" // Sn (n=0..96), tag 9
    METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_1 = 22449, // "1-0:1.8.1" // F9(3,3)
    METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_2 = 22450, // "1-0:1.8.2" // F9(3,3)
    METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_1 = 24129, // "1-0:2.8.1" // F9(3,3)
    METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_2 = 24130, // "1-0:2.8.2" // F9(3,3)
    TARIFF_INDICATOR_ELECTRICITY = 27484,                           // "0-0:96.14.0" // S4
    ACTUAL_ELECTRICITY_POWER_DELIVERED = 22392,                     // "1-0:1.7.0" // F5(3,3)
    ACTUAL_ELECTRICITY_POWER_RECEIVED = 24072,                      // "1-0:2.7.0"  // F5(3,3)
    INSTANTANEOUS_VOLTAGE_L1 = 47722,                               // "1-0:32.7.0" // F4(1,1)
    INSTANTANEOUS_VOLTAGE_L2 = 17482,                               // "1-0:52.7.0" // F4(1,1)
    INSTANTANEOUS_VOLTAGE_L3 = 52778,                               // "1-0:72.7.0" // F4(1,1)
    INSTANTANEOUS_CURRENT_L1 = 44698,                               // "1-0:31.7.0" // F5(1,1)
    INSTANTANEOUS_CURRENT_L2 = 14458,                               // "1-0:51.7.0" // F5(1,1)
    INSTANTANEOUS_CURRENT_L3 = 49754,                               // "1-0:71.7.0" // F5(1,1)
    INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L1 = 59818,                 // "1-0:21.7.0" // F5(3,3)
    INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L2 = 29578,                 // "1-0:41.7.0" // F5(3,3)
    INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L3 = 64874,                 // "1-0:61.7.0" // F5(3,3)
    INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L1 = 62842,                 // "1-0:22.7.0" // F5(3,3)
    INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L2 = 32602,                 // "1-0:42.7.0" // F5(3,3)
    INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L3 = 2362,                  // "1-0:62.7.0" // F5(3,3)
    DEVICE_TYPE = 53290,                                            // "0-1:24.1.0" // F3(0,0)
    CURRENT_AVERAGE_DEMAND_ACTIVE_ENERGY_IMPORT = 22224,            // "1-0:1.4.0" // F5(3,3) Unit: kW ???
    MAXIMUM_DEMAND_RUNNING_MONTH = 22336,                           // "1-0:1.6.0"  // (TST)(F5(3,3)) Unit kW
    MAXIMUM_DEMAND_LAST_13_MONTHS = 9914,                           // "0-0:98.1.0" // (TST)(F5(3,3)) Unit kW
    TEXT_MESSAGE_MAX_1024 = 27394,                                  // "0-0:96.13.0"// Sn (n=0..2048)
} OIDHashes;

/**
 * HashMap to functionpointer
 */
typedef struct hashkeyval
{
    unsigned short hash;

    char *name;
    unsigned int namelen;

    COSEMType type;
    // unsigned char digitWidth; // Total number of digits
    // unsigned char digitPoint; // number of digits after decimal point
    unsigned char next; // Points to the next index inside the OIDMap if this is a multi value

    // void (*handler)(char *line, int lineLength);
} hashkeyval_t;

int decodeLine(char *, char *, int);

#ifdef __cplusplus
}
#endif

#endif