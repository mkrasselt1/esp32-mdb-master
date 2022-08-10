#ifndef MDB_DATA_H
#define MDB_DATA_H

#include <Arduino.h>

typedef union __attribute__ ((packed)) mdbData_t
{
    uint8_t raw[37];

    struct __attribute__ ((packed)){
        uint8_t Command;
        uint8_t Subcommand;
        
        struct __attribute__ ((packed)){
            uint8_t raw[35];
        } Raw;
    };    

}mdbData;





#endif