#ifndef COIN_DEVICE_H
#define COIN_DEVICE_H

#include <Arduino.h>
#include "mdbDevice.h"
#include "IPayment.h"
#include "coinChangerSpecification.h"

#define CHANGER_DEBUG

#ifdef CHANGER_DEBUG
    #define DBG(X) Serial.println(X)
    #define DBGf(...) Serial.printf(__VA_ARGS__)
#else
    #define DBG(X)
    #define DBGf(...)
#endif


namespace CoinChangerSetup
{
    enum State
    {
        SETUP,
        SETUP_EXPANSION,
        FEATURE_ENABLE,
        COIN_TYPE
    };
};

namespace CoinChanger
{
    enum State
    {
        DEVICE_INACTIVE,
        DEVICE_DISABLED,
        DEVICE_ENABLED,
        VEND,
        REQUEST_TUBE_STATUS,
        PAYOUT,
        WAIT_PAYOUT_BUSY
    };
};

typedef struct{
    uint8_t decimalPlaces;
    uint8_t scaleFactor;
    uint8_t featureLevel;
    uint16_t countryCode;
    uint16_t routingInfo;
    uint8_t numCoins;
} CoinChangerProperties;

typedef struct{
    bool active;
    bool isToken;
    bool canRouteTo;
    uint8_t amount;
    mdbCurrency value;
    uint8_t type;
} Tube;



class coinChanger : public mdbDevice, public IPaymentDevice
{
public:
    //mdbDevice Interface:
    coinChanger(uint8_t addr);
    uint8_t handleDevice();
    void handleResponseDevice(uint8_t *respData, uint16_t respLen);
    void resetDevice();

    //IPaymentDevice Interface:
    void registerPaymentInterface(IPayment * payment);
    bool canRevalue();
    bool reportCashsale();
    bool pendingEscrowRequest();
    const char* getDeviceName();
    mdbCurrency getCurrentFunds();
    mdbCurrency getRevalueMax();
    void clearCurrentFunds();


private:
    Tube Tubes[16];
    Tube payout;
    CoinChangerSetup::State changerSetupState = CoinChangerSetup::SETUP;
    CoinChanger::State changerState = CoinChanger::DEVICE_INACTIVE;
    COINDataRequest* requ;
    COINDataResponse* resp;
    uint8_t responseLen = 0;
    CoinChangerProperties properties = {0};
    mdbProduct *currentProduct;
    mdbCurrency currentFunds;
    bool escrowPending = false;
    bool fundsChanged = false;
    bool payoutBusy = false;
    float payoutAmount = 0;

    void calculateDispense();

    void onResponse_Reset(MDBResponses::rType responseType);
    
    void onResponse_Setup(MDBResponses::rType responseType);
    
    
    void onResponse_Poll(MDBResponses::rType responseType);
    uint8_t handleActivity(uint8_t idx);
    
    void onResponse_TubeStatus(MDBResponses::rType responseType);

    void onResponse_CoinType(MDBResponses::rType responseType);

    void onResponse_Dispense(MDBResponses::rType responseType);

    void onResponse_Expansion(MDBResponses::rType responseType);
    void onResponse_ExpansionID(MDBResponses::rType responseType);
    void onResponse_ExpansionFeatureEnable(MDBResponses::rType responseType);
    void onResponse_ExpansionNumberOfDataBlocks(MDBResponses::rType responseType);
    void onResponse_ExpansionSendBlock(MDBResponses::rType responseType);
    void onResponse_ExpansionRequestBlock(MDBResponses::rType responseType);



};

#endif