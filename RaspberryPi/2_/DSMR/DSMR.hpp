#ifndef DSMR_H
#define DSMR_H

#include <string>
#include <2_/database/influxline.hpp>
#include <lib/error_codes.h>

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

error_e decodeLine(influx::InfluxLine &dest, const std::string &line);

#endif