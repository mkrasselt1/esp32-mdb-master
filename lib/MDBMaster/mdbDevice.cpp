#include "mdbDevice.h"
#include "mdbDevices.h"

mdbDevice::mdbDevice(uint8_t addr)
{
    mdbAddress = addr;
}

bool mdbDevice::handle()
{
    if(cooldown){
        if((millis() - cooldownTime) < UINT32_MAX/2)
        {
            cooldown = false;
        }else{
            return false;
        }
        
    }
    if(active)
    {
        uint8_t lenToSend = handleDevice();
        if(lenToSend){
            //todo: Add Address to packet header & calc chksum
            uint8_t chk = 0;
            lastCmd = sendData[0];
            lastSub = 0;
            if(lenToSend > 1) lastSub = sendData[1];
            for (size_t i = 0; i < lenToSend; i++)
            {
                if(i == 0){
                    //insert device address
                    sendBuffer[i] = (sendData[i] & 0x07) | (mdbAddress & 0xF8) | (0x100);
                }else{
                    sendBuffer[i] = sendData[i];
                }
                
                chk += sendBuffer[i];
            }
            sendBuffer[lenToSend] = chk;
            sendLen = lenToSend + 1;
            setResponseTimeout(200);
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

void mdbDevice::handleResponse(uint8_t *resp, uint16_t respLen)
{
    handleResponseDevice(resp, respLen);
    mdbActive = true;
}

void mdbDevice::setActive()
{
    active = true;
}

void mdbDevice::setInactive()
{
    active = false;
}


void mdbDevice::reset()
{
    state = MDBDeviceStates::SEARCHING;
    resetErrorCounter();
    resetDevice();
}

uint16_t mdbDevice::getErrorCounter()
{
    return errorCounter;
}

void mdbDevice::increaseErrorCounter()
{
    errorCounter++;
    if(errorCounter > MDB_DEVICE_ERROR_LIM){
        Serial.print(getDescriptor() ? getDescriptor()->Name : "UNKNOWN");
        Serial.println(" paused, caused by too many errors");
        Serial.println();
        cooldownTime = millis() + MDB_DEVICE_COOLDOWN;
        cooldown = true;
        mdbActive = false;
        reset();
    }
}

bool mdbDevice::isActive()
{
    return mdbActive;
}

void mdbDevice::resetErrorCounter()
{
    errorCounter = 0;
}

const mdbDeviceDescriptor* mdbDevice::getDescriptor()
{
    for (size_t i = 0; i < sizeof(mdbDevices)/sizeof(mdbDevices[0]); i++)
    {
        if(mdbDevices[i].ID == mdbAddress)
        {
            return &(mdbDevices[i]);
        }
    }
    return nullptr;
}

bool mdbDevice::responseTimeoutEllapsed()
{
    if((millis() - respTimeout) < UINT32_MAX/2)
    {
        return true;
    }
    return false;
}

void mdbDevice::setResponseTimeout(uint32_t t)
{
    respTimeout = millis()+t;
}
