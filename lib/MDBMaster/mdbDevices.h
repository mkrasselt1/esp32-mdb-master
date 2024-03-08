#ifndef MDB_DEVICES_H
#define MDB_DEVICES_H

#include "mdbStructure.h"

const mdbDeviceDescriptor mdbDevices[] = {
    {"Cashless Device #1", MDB_CASHLESS_1},
    {"Cashless Device #2", MDB_CASHLESS_2},
    {"Communications Gateway", MDB_COMM_GATEWAY},
    {"Age Verification Device", MDB_AGE_VER_DEV},
    {"Display", MDB_DISPLAY},
    {"Changer", MDB_CHANGER},    
    {"Energy Management System", MDB_ENERGY_MGMT},
    {"Bill Validator", MDB_BILL_VALIDATOR},    
    {"Universal Satellite Device #1", MDB_UNI_SAT_DEV_1},
    {"Universal Satellite Device #2", MDB_UNI_SAT_DEV_2},
    {"Universal Satellite Device #3", MDB_UNI_SAT_DEV_3},
    {"Coin Hopper #1", MDB_COIN_HOPPER_1},    
    {"Coin Hopper #2", MDB_COIN_HOPPER_2},
    {"Reserved #1", MDB_RESERVED_1},
    {"Reserved #2", MDB_RESERVED_2},
    {"Reserved #3", MDB_RESERVED_3},
    {"Experimental #1", MDB_EXPERIMENTAL_1},
    {"Experimental #2", MDB_EXPERIMENTAL_2},
    {"Vending Machine Specific #1", MDB_VMC_SPECIFIC_1},
    {"Vending Machine Specific #2", MDB_VMC_SPECIFIC_2}
};


#endif