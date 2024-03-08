#include "mdbMaster.h"

mdbCashLess MDBDevice_Cashless_1(MDB_CASHLESS_1);
//mdbCashLess MDBDevice_Cashless_2(MDB_CASHLESS_2);

universalSatelliteDevice MDBDevice_USD_1(MDB_UNI_SAT_DEV_1);
universalSatelliteDevice MDBDevice_USD_2(MDB_UNI_SAT_DEV_2);
universalSatelliteDevice MDBDevice_USD_3(MDB_UNI_SAT_DEV_3);

coinChanger MDBDevice_Changer(MDB_CHANGER);


uint8_t mdbMaster::deviceResponse[40] = {'\0'};
uint8_t mdbMaster::deviceResponseLength = 0;
uint8_t mdbMaster::recFrame[40] = {'\0'};
uint8_t mdbMaster::recLen = 0;
uint8_t mdbMaster::dataReceived = 0;
mdbDevice* mdbMaster::devices[32] = {nullptr};
mdbDevice* mdbMaster::currentDevice = nullptr;

uint8_t mdbMaster::availableDevices = 0;

void mdbMaster::init()
{
    mdbDriver::start();
    devices[0] = &MDBDevice_Cashless_1;
    // devices[1] = &MDBDevice_Cashless_2;

    devices[2] = &MDBDevice_USD_1;
    devices[3] = &MDBDevice_USD_2;
    devices[4] = &MDBDevice_USD_3;
    
    devices[5] = &MDBDevice_Changer;

    MDBDevice_Changer.setActive();

    MDBDevice_Cashless_1.setActive();
    // MDBDevice_Cashless_2.setActive();

    // MDBDevice_USD_1.setActive();
    // MDBDevice_USD_2.setActive();
    // MDBDevice_USD_3.setActive();

    // for (size_t i = 0; i < 32; i++)
    // {
    //     devices[i] = new universalSatelliteDevice(i<<3);
    //     devices[i]->setActive();
    // }
    
};

void mdbMaster::handleDevices()
{
    bool printDbg = false;
    for (size_t i = 0; i < 32; i++)
    {
        dataReceived = 0;
        if(devices[i]){
            if(devices[i]->handle()){
                currentDevice = devices[i];
                mdbDriver::sendPacket(currentDevice->sendBuffer, currentDevice->sendLen);
                

                while((!dataReceived) && (!currentDevice->responseTimeoutEllapsed()));
                // printDbg = false;
                // if((!dataReceived) || ((dataReceived) && recFrame[0] != 0x02)){
                //     printDbg = true;
                // }

                if(printDbg){
                    Serial.println();
                    Serial.println(currentDevice->getDescriptor() ? currentDevice->getDescriptor()->Name : "UNKNOWN");
                    Serial.print("[VMC -> Periph]: ");
                    for (size_t i = 0; i < currentDevice->sendLen; i++)
                    {
                        Serial.printf("%02X ", currentDevice->sendBuffer[i]);
                    }
                    Serial.println();
                }


                if(!dataReceived){
                    currentDevice->increaseErrorCounter();
                    if(printDbg){
                        Serial.println("[Periph -> VMC]: NO Response");
                        Serial.println();
                    }
                    
                }else{
                    
                    if(printDbg){
                        Serial.print("[Periph -> VMC]:  ");
                        for (size_t i = 0; i < recLen; i++)
                        {
                            Serial.printf("%02X ", recFrame[i]);
                        }
                        Serial.println();
                    }            

                    uint16_t errBefore = currentDevice->getErrorCounter();        
                    currentDevice->handleResponse(recFrame, recLen);
                    if(errBefore == currentDevice->getErrorCounter()){
                        currentDevice->resetErrorCounter();
                    }
                }

                if(devices[i]->isActive())
                {
                    availableDevices |= (1<<i);
                }else{
                    availableDevices &= ~(1<<i);
                }

                delay(100);
            }
        }
    }
    
}


void mdbMaster::startThread(){

};


void mdbMaster::receive(uint16_t newByte)
{
    if (deviceResponseLength >= (sizeof(deviceResponse) / sizeof(uint8_t)))
    {
        deviceResponseLength = 0;
    }
    deviceResponse[deviceResponseLength] = newByte & 0xFF;
    if (newByte & 0x100)
    { // checks for checksum in last byte
        uint16_t tmp = 0, chkSum = 0;
        if (deviceResponseLength)
        {
            for (uint8_t i = 0; i < deviceResponseLength; i++)
            {
                tmp = deviceResponse[i];
                chkSum += tmp & 0xFF;
            }
        }

        chkSum &= 0xFF;
        chkSum |= 0x100;
        // checksum ok
        if (newByte == chkSum)
        {
            if (deviceResponseLength)
            { // send ack on longer messages
                // delay(5);
                mdbDriver::send(MDB_ACK);
            }

            memcpy(recFrame, deviceResponse, sizeof(recFrame[0]) * deviceResponseLength + 1);
            recLen = deviceResponseLength + 1;
            dataReceived = 1;
        }
        else
        {
            // checksum failed
            if (deviceResponseLength)
            { // send nack on longer messages
                // delay(5);
                mdbDriver::send(MDB_NACK);
                if(currentDevice){
                    currentDevice->increaseErrorCounter();
                }
            }
        }

        deviceResponseLength = 0;
    }
    else
    {
        deviceResponseLength++;
    }
}