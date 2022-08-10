#ifndef MDB_COINCHANGER_H
#define MDB_COINCHANGER_H

#include <Arduino.h>
#include "coinChangerSpecification.h"

class mdbCoinChanger{
    public:
        static void init();
        static uint8_t  reset(uint16_t* toSend);
        static uint8_t  setup(uint16_t* toSend);
        static uint8_t  coinEnable(uint16_t* toSend);
        static uint8_t  tubeStatus(uint16_t* toSend);
        static uint8_t  poll(uint16_t* toSend);
        static uint8_t  loop(uint16_t* toSend);
        static uint8_t  escrow(uint16_t* toSend);
        static uint8_t featureId(uint16_t *toSend);
        static uint8_t featureEnable(uint16_t *toSend, bool AlternatePayout, bool ManualInfill, bool Diagnose, bool FTl);
        static void response(uint8_t* received, uint8_t len);
        static void responsePoll(uint8_t* received, uint8_t len);
        static void responseTube(uint8_t* received, uint8_t len);
        static void responseSetup(uint8_t* received, uint8_t len);
        static void start();
        static void send(uint16_t data);
        static void disable();
        static void enable();
        static uint16_t getFunds();
        static void setFunds(uint16_t);
        static void setPayOut(uint16_t);
        static bool updateFunds(uint32_t *fundStore);
        static bool getPayOutChanged();
        static uint16_t scaleFactor;
        static unsigned long nextPoll;
        static long timeout;
    private:
        static long nextTubePoll;
        static uint8_t tubeLevel[16];
        static uint8_t fakeCoins;
        static uint8_t state;
        static uint8_t featureLevel;
        static uint8_t decimals;
        static uint8_t coinChannelValue[16];
        static uint16_t coinFunds;
        static uint16_t coinPayOut;
        static uint16_t coinPayOutPrevious;
        static void swSerialTask(void* args);
        static uint16_t getCoinChannelValue(uint8_t chanel);
        static uint16_t getTubeInventory();
        static bool disabled;
        
};


#endif