#ifndef MDB_DEVICE_H
#define MDB_DEVICE_H

#include <Arduino.h>
#include "mdbStructure.h"

#define MDB_DEVICE_ERROR_LIM    5
#define MDB_DEVICE_COOLDOWN     10000

#define EC16(w) ((w>>8)|((w<<8)&0xFF00))
#define EC32(dw) (((dw<<24)&0xFF000000)|((dw>>8)&0xFF00)|((dw<<8)&0xFF0000)|(dw>>24))

namespace MDBDeviceStates{
    enum deviceState{
        SEARCHING,
        UNINITIALIZED,
        SETUP,
        READY
    };
}

namespace MDBResponses{
    enum rType{
        ACK,
        NACK,
        DATA
    };
}



class mdbDevice{
private:
    virtual uint8_t handleDevice(){return 0;}
    virtual void handleResponseDevice(uint8_t *respData, uint16_t respLen){}
    virtual void resetDevice(){}
    void setResponseTimeout(uint32_t t);
    bool mdbActive = false;

protected:
    bool active = false;
    uint16_t errorCounter = 0;
    MDBDeviceStates::deviceState state = MDBDeviceStates::SEARCHING;
    uint8_t mdbAddress;
    uint32_t respTimeout = 0;
    bool cooldown = false;
    uint32_t cooldownTime = 0;
    uint8_t sendData[36];
    uint8_t lastCmd = 0;
    uint8_t lastSub = 0;
    
public:    
    mdbDevice(uint8_t addr);
    bool handle();
    void setActive();
    void setInactive();
    void reset();
    uint16_t getErrorCounter();
    void increaseErrorCounter();
    void resetErrorCounter();
    const mdbDeviceDescriptor* getDescriptor();
    bool responseTimeoutEllapsed();
    void handleResponse(uint8_t *resp, uint16_t respLen);
    bool isActive();
    

    uint16_t sendBuffer[40];
    uint16_t sendLen = 0;

};




typedef void (*mdbEventHandler)(mdbDevice* dev);


#endif