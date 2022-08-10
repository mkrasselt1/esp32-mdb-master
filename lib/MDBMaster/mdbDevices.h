#ifndef MDB_DEVICES_H
#define MDB_DEVICES_H

#include "mdbStructure.h"

const mdbDeviceDescriptor mdbDevices[] = {
    {"Cashless Device #1", MDB_CASHLESS_1, nullptr},
    {"Cashless Device #2", MDB_CASHLESS_2, nullptr},
    {"Communications Gateway", MDB_COMM_GATEWAY, nullptr},
    {"Age Verification Device", MDB_AGE_VER_DEV, nullptr},
    {"Display", MDB_DISPLAY, nullptr},
    {"Changer", MDB_CHANGER, nullptr},    
    {"Energy Management System", MDB_ENERGY_MGMT, nullptr},
    {"Bill Validator", MDB_BILL_VALIDATOR, nullptr},    
    {"Universal Satellite Device #1", MDB_UNI_SAT_DEV_1, nullptr},
    {"Universal Satellite Device #2", MDB_UNI_SAT_DEV_2, nullptr},
    {"Universal Satellite Device #3", MDB_UNI_SAT_DEV_3, nullptr},
    {"Coin Hopper #1", MDB_COIN_HOPPER_1, nullptr},    
    {"Coin Hopper #2", MDB_COIN_HOPPER_2, nullptr},
    {"Reserved #1", MDB_RESERVED_1, nullptr},
    {"Reserved #2", MDB_RESERVED_2, nullptr},
    {"Reserved #3", MDB_RESERVED_3, nullptr},
    {"Experimental #1", MDB_EXPERIMENTAL_1, nullptr},
    {"Experimental #2", MDB_EXPERIMENTAL_2, nullptr},
    {"Vending Machine Specific #1", MDB_VMC_SPECIFIC_1, nullptr},
    {"Vending Machine Specific #2", MDB_VMC_SPECIFIC_2, nullptr}
};


#endif