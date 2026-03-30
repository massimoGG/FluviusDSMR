/**
 * calculateHash.c - Calculates the hash value of the provided array of OID keys
 *  to have a more efficient lookup on the microcontroller
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

int main(const int argc, char *argv[])
{

    char *lines[] = {
        "0-0:1.0.0",   // DATE_TIME_STAMP
        "0-0:96.1.1",  // EQUIPMENT_IDENTIFIER
        "1-0:1.8.1",   // METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_1
        "1-0:1.8.2",   // METER_READING_ELECTRICITY_DELIVERED_TO_CLIENT_TARIFF_2
        "1-0:2.8.1",   // METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_1
        "1-0:2.8.2",   // METER_READING_ELECTRICITY_DELIVERED_BY_CLIENT_TARIFF_2
        "0-0:96.14.0", // TARIFF_INDICATOR_ELECTRICITY
        "1-0:1.7.0",   // ACTUAL_ELECTRICITY_POWER_DELIVERED
        "1-0:2.7.0",   // ACTUAL_ELECTRICITY_POWER_RECEIVED
        "1-0:32.7.0",  // INSTANTANEOS_VOLTAGE_L1
        "1-0:52.7.0",  // INSTANTANEOS_VOLTAGE_L2
        "1-0:72.7.0",  // INSTANTANEOS_VOLTAGE_L3
        "1-0:31.7.0",  // INSTANTANEOS_CURRENT_L1
        "1-0:51.7.0",  // INSTANTANEOS_CURRENT_L2
        "1-0:71.7.0",  // INSTANTANEOS_CURRENT_L3
        "1-0:21.7.0",  // INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L1
        "1-0:41.7.0",  // INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L2
        "1-0:61.7.0",  // INSTANTANEOUS_ACTIVE_POSITIVE_POWER_L3
        "1-0:22.7.0",  // INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L1
        "1-0:42.7.0",  // INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L2
        "1-0:62.7.0",  // INSTANTANEOUS_ACTIVE_NEGATIVE_POWER_L3
        "0-1:24.1.0",  // DEVICE_TYPE
        "1-0:1.4.0",   // CURRENT_AVERAGE_DEMAND_ACTIVE_ENERGY_IMPORT
        "1-0:1.6.0",   // MAXIMUM_DEMAND_RUNNING_MONTH
        "0-0:98.1.0",  // MAXIMUM_DEMAND_LAST_13_MONTHS
        "0-0:96.13.0", // TEXT_MESSAGE_MAX_1024
        "0-0:17.0.0",  // whatever this is, but it's 999
        "0-0:96.1.4",  // idk what this is either
        "0-1:24.2.3",  // gas
    };

#define len 29

    unsigned short *hashes = malloc(sizeof(unsigned short) * len);

    printf("Calculating hashes\n");

    // write hashes
    FILE *f = fopen("hashes.out", "w");

    for (int i = 0; i < len; i++)
    {
        unsigned short hash_i = hash(lines[i], strlen(lines[i]));
        printf("[%d]The hash of '%s' is %lu\n", i, lines[i], hash_i);
        hashes[i] = hash_i;
        fprintf(f, "%s = %d,\n", lines[i], hash_i);
    }
    fclose(f);

    printf("Checking hashes\n");

    // Iterate through the hashs to find duplicates
    for (int currentHash = 0; currentHash < len; currentHash++)
    {
        // Iterate through all hashes
        for (int i = 0; i < len && i != currentHash; i++)
        {
            if (hashes[currentHash] == hashes[i])
            {
                fprintf(stderr, "ERROR: Duplicate hash detected! %d with %d\n", currentHash, i);
                return EXIT_FAILURE;
            }
        }
    }

    printf("No duplicates detected! Hashing OK!\n");

    return EXIT_SUCCESS;
}
