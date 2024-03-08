#ifndef MDB_STRUCTURE_H
#define MDB_STRUCTURE_H
#include <Arduino.h>

/*
----MDB ADDRESSES from specification Section 2.3-------
*/
#define MDB_CHANGER         0x08
#define MDB_CASHLESS_1      0x10
#define MDB_COMM_GATEWAY    0x18
#define MDB_DISPLAY         0x20
#define MDB_ENERGY_MGMT     0x28
#define MDB_BILL_VALIDATOR  0x30
#define MDB_RESERVED_1      0x38
#define MDB_UNI_SAT_DEV_1   0x40
#define MDB_UNI_SAT_DEV_2   0x48
#define MDB_UNI_SAT_DEV_3   0x50
#define MDB_COIN_HOPPER_1   0x58
#define MDB_CASHLESS_2      0x60
#define MDB_AGE_VER_DEV     0x68
#define MDB_COIN_HOPPER_2   0x70
#define MDB_RESERVED_2      0x78
#define MDB_RESERVED_3      0xD8
#define MDB_EXPERIMENTAL_1  0xE0
#define MDB_EXPERIMENTAL_2  0xE8
#define MDB_VMC_SPECIFIC_1  0xF0
#define MDB_VMC_SPECIFIC_2  0xF8


struct mdbDeviceDescriptor
{
    const char * Name;
    uint8_t ID;
};




#endif