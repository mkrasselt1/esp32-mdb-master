#ifndef MDB_CASHLESS_H
#define MDB_CASHLESS_H

#include <Arduino.h>
#include <WiFi.h>
#include "cashLessSpecification.h"

namespace CashLess
{
    enum State
    {
        MISSING,
        INACTIVE,
        SETUP_DATA,
        SETUP_PRICES,
        FEATURE_ID,
        FEATURE_ENABLE,
        READER_DISABLED,
        READER_ENABLED,
        SESSION_IDLE,
        SESSION_END,
        POLLING, // for a later result
        VEND,
        VEND_REQ,
        REVALUE,
        NEGATIVE_VEND,
    };
};

class mdbCashLess
{
public:
    static void init();
    static uint8_t reset(uint16_t *toSend);
    static uint8_t setupData(uint16_t *toSend);
    static uint8_t setupPrices(uint16_t *toSend);
    static uint8_t poll(uint16_t *toSend);
    static uint8_t loop(uint16_t *toSend);
    static uint8_t disable(uint16_t *toSend);
    static uint8_t enable(uint16_t *toSend);
    static uint8_t revalueLimitRequest(uint16_t *toSend);
    static uint8_t vendRequest(uint16_t *toSend);
    static uint8_t vendSuccess(uint16_t *toSend);
    static uint8_t sessionEnd(uint16_t *toSend);
    static uint8_t readerCancel(uint16_t *toSend);
    static uint8_t cashSale(uint16_t *toSend);
    static uint8_t featureId(uint16_t *toSend);
    static uint8_t featureEnable(uint16_t *toSend, bool alwaysIdle, bool dataEntry, bool negativeVend, bool multiCurrency, bool largeNumber, bool fileTransferLayer);
    static uint8_t revalue(uint16_t *toSend);

    static void response(uint8_t *received, uint8_t len);
    static void responsePoll(uint8_t *received, uint8_t len);
    static void responseSetup(uint8_t *received, uint8_t len);
    static void responseDisplay(uint8_t *received, uint8_t len);
    static void responseExpansionId(uint8_t *received, uint8_t len);
    static void responseBeginSession(uint8_t *received, uint8_t len);
    static void responseRevalueLimit(uint8_t *received, uint8_t len);
    static void responseCancelSession(uint8_t *received, uint8_t len);
    static void responseEndSession(uint8_t *received, uint8_t len);
    static void responseVendDenied(uint8_t *received, uint8_t len);
    static void responseVendApproved(uint8_t *received, uint8_t len);
    static void responseRevalueDenied(uint8_t *received, uint8_t len);
    static void responseRevalueApproved(uint8_t *received, uint8_t len);

    static bool hasApproved(bool *result);
    static void reqSessionComplete();
    static bool hasActiveSession();
    static void logCashSale(uint16_t product, uint16_t price);

    static bool available();
    static void start();
    static void send(uint16_t data);

    static uint16_t getFunds();
    static void setFunds(uint16_t);
    static bool updateFunds(uint32_t *fundStore);

    static uint16_t getRevalue();
    static void setRevalue(uint16_t);
    static bool updateRevalue(uint32_t *revalueStore);

    static void setVendRequest(uint16_t product, uint16_t price);
    static uint16_t getVendPrice();
    static void setVendSuccess();

    static bool displayRequestMessageAvailable();
    static uint16_t scaleFactor;
    static unsigned long nextPoll;
    static char displayMessage[32];
    static uint16_t displayTimeout;
    static long timeout;

private:
    static long nextTubePoll;
    static bool isVendSuccess;
    static CashLess::State state;
    static uint8_t featureLevel;
    static uint8_t decimals;
    static bool disabled;

    static uint16_t cashLessFunds;
    static uint16_t cashLessFundsPrevious;

    static uint16_t cashLessRevalue;
    static uint16_t cashLessRevaluePrevious;
    static int32_t cashLessRevalueLimit;

    static uint16_t priceCashSale;
    static int16_t productCashSale;

    static uint16_t priceToApprove;
    static uint16_t productToApprove;
    static int8_t approved;

    static bool messageDisplayed;

    static void swSerialTask(void *args);
};

#endif