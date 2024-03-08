#ifndef MDB_MASTER_H
#define MDB_MASTER_H

#include "driver/mdbDriver.h"
#include "mdbDevice.h"
#include "mdbDevices.h"

#include "device/cashLessDevice/cashLess.h"
#include "device/universalSatelliteDevice/universalSatelliteDevice.h"
#include "device/coinChangerDevice/coinChanger.h"

extern mdbCashLess MDBDevice_Cashless_1;
//extern mdbCashLess MDBDevice_Cashless_2;

extern coinChanger MDBDevice_Changer;

extern universalSatelliteDevice MDBDevice_USD_1;
extern universalSatelliteDevice MDBDevice_USD_2;
extern universalSatelliteDevice MDBDevice_USD_3;


class mdbMaster{
    public:
        static void init();
        static void startThread();
        static void handleDevices();
        static void receive(uint16_t newByte);
        static uint8_t availableDevices;
                
    private:        
        static uint8_t deviceResponse[40];
        static uint8_t deviceResponseLength;
        static uint8_t recFrame[40];
        static uint8_t recLen;
        static uint8_t dataReceived;
        static mdbDevice* devices[32];
        static mdbDevice* currentDevice;
};


#endif