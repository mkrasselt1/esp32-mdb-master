#ifndef MDB_MASTER_H
#define MDB_MASTER_H

#include "driver/mdbDriver.h"
#include "device/coinChanger/coinChanger.h"
#include "device/cashLessDevice/cashLess.h"
#include "device/billValidator/billValidator.h"

#define MDB_MASTER_VEND_APPROVE_RESULT_Expired 0
#define MDB_MASTER_VEND_APPROVE_RESULT_Awaiting 1
#define MDB_MASTER_VEND_APPROVE_RESULT_Granted 2
#define MDB_MASTER_VEND_APPROVE_RESULT_Deny 3

namespace MDB{
    enum MDBCommState{
        IDLE,
        POLL,
        WAIT
    };
    enum ACTIVE_DEVICE{
        NONE,
        CASHLESS,
        COIN_CHANGER,
        BILL_VALIDATOR
    };}
class mdbMaster{
    public:
        static void init();
        static void pollAll();
        static void start();
        static void receive(uint16_t newByte);
        static void payOut(uint16_t payoutAmount);
        static bool isNewText();
        static bool expiredApproval();
        static bool hasApproved(bool *result);
        static uint32_t getFunds();
        static void approve(uint16_t product, uint16_t price);
        static void setFund(uint16_t fund);
        static void finishedProduct();
        static char displayText[32];
        static bool cleanScreen;
        static long displayTimeout;
                
    private:
        static MDB::MDBCommState state;
        static MDB::ACTIVE_DEVICE vendAuthDevice;
        static long time;
        static long timeout;
        static long sessionTimeout;
        static unsigned long nextPoll;
        static MDB::ACTIVE_DEVICE activeDevice;

        static uint16_t priceToApprove;
        static uint16_t productToApprove;
        static uint8_t vendApproveResult;
        static bool dataReceived;
        
        static uint32_t billFunds;

        static uint8_t deviceResponse[40];
        static uint8_t deviceResponseLength;
        static uint8_t recFrame[40];
        static uint8_t recLen;
        
        static bool displayTextShown;
        
        static void updateScreen();
        static void findNextPollDevice();
};


#endif