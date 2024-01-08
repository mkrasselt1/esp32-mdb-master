#ifndef MDB_BILLVALIDATOR_H
#define MDB_BILLVALIDATOR_H

#include <Arduino.h>
#include "billValidatorSpecification.h"

class mdbBillValidator{
    public:
        static void init();
        static uint8_t  reset(uint16_t* toSend);
        static uint8_t  setup(uint16_t* toSend);
        static uint8_t  security(uint16_t* toSend);
        static uint8_t  billEnable(uint16_t* toSend);
        static uint8_t  stacker(uint16_t* toSend);
        static uint8_t  poll(uint16_t* toSend);
        static uint8_t  loop(uint16_t* toSend);
        static uint8_t  escrow(uint16_t* toSend);
        static uint8_t featureId(uint16_t *toSend);
        static uint8_t featureEnable(uint16_t *toSend, bool AlternatePayout, bool ManualInfill, bool Diagnose, bool FTl);
        static void response(uint8_t* received, uint8_t len);
        static void responsePoll(uint8_t* received, uint8_t len);
        static void responseStacker(uint8_t* received, uint8_t len);
        static void responseSetup(uint8_t* received, uint8_t len);
        static void start();
        static void send(uint16_t data);
        static void disable();
        static void enable();
        static uint16_t getFunds();
        static void setFunds(uint16_t);
        static void setPayOut(uint16_t);
        static void setInhibit(uint8_t);
        static bool updateFunds(uint32_t *fundStore);
        static bool getPayOutChanged();
        static uint16_t scaleFactor;
        static unsigned long nextPoll;
        static long timeout;
    private:
        static long nextTubePoll;
        static uint8_t inhibit;
        static uint8_t inhibitChanged;
        static uint8_t state;
        static uint8_t featureLevel;
        static uint8_t decimals;
        static uint16_t capacity;
        static uint8_t escrowCapable;
        static uint8_t billChannelValue[16];
        static uint16_t billFunds;
        static uint16_t billPayOut;
        static uint16_t billPayOutPrevious;
        static void swSerialTask(void* args);
        static uint16_t getBillChannelValue(uint8_t chanel);
        static uint16_t getStackInventory();
        static bool disabled;
        
};


#endif