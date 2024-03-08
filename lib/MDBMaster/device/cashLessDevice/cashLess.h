#ifndef CASHLESS_DEVICE_H
#define CASHLESS_DEVICE_H

#include <Arduino.h>
#include "mdbDevice.h"
#include "IPayment.h"
#include "cashLessSpecification.h"


namespace CashLessSetup
{
    enum State
    {
        SETUP_DATA,
        SETUP_PRICES,
        SETUP_EXPANSION,
        FEATURE_ID,
        FEATURE_ENABLE,
        DEVICE_ENABLE,
        DEVICE_DISABLE
    };
};

namespace CashLessReader
{
    enum State
    {
        DEVICE_INACTIVE,
        DEVICE_DISABLED,
        DEVICE_ENABLED,
        SESSION_IDLE,
        SESSION_CANCELLING,
        VEND_REQUESTING,
        VEND,
        REVALUE,
        REVALUE_REQUESTING,
        NEGATIVE_VEND
    };
};

namespace CLEvents
{
    #define NUM_CL_EVTS 4
    enum evt
    {
        SETUP_COMPLETED,
        FUNDS,
        VEND_ACCEPT,
        VEND_DENIED
    };
}

typedef struct{
    uint8_t decimalPlaces;
    uint8_t scaleFactor;
    uint8_t featureLevel;
    bool canRevalue;
} cashLessProperties;



class mdbCashLess : public mdbDevice, public IPaymentDevice
{
public:
    //mdbDevice Interface:
    mdbCashLess(uint8_t addr);
    uint8_t handleDevice();
    void handleResponseDevice(uint8_t *respData, uint16_t respLen);
    void resetDevice();

    //IPaymentDevice Interface:
    void registerPaymentInterface(IPayment * payment);
    bool canRevalue();
    bool reportCashsale();
    const char* getDeviceName();
    mdbCurrency getCurrentFunds();
    void clearCurrentFunds();

private:
    CashLessSetup::State CLSetupState = CashLessSetup::SETUP_DATA;
    CashLessReader::State CLReaderState = CashLessReader::DEVICE_INACTIVE;
    CashLessDataRequest* requ;
    CashLessDataResponse* resp;
    cashLessProperties properties = {0};
    mdbProduct *currentProduct;
    mdbCurrency currentFunds;
    bool sessionCanRevalue = true;

    void onResponse_Reset(MDBResponses::rType responseType);
    
    void onResponse_Setup(MDBResponses::rType responseType);
    void onResponse_SetupConfig(MDBResponses::rType responseType);
    void onResponse_SetupMaxMin(MDBResponses::rType responseType);
    
    void onResponse_Poll(MDBResponses::rType responseType);
    
    void onResponse_Vend(MDBResponses::rType responseType);
    void onResponse_VendRequest(MDBResponses::rType responseType);
    void onResponse_VendCancel(MDBResponses::rType responseType);
    void onResponse_VendSuccess(MDBResponses::rType responseType);
    void onResponse_VendFailure(MDBResponses::rType responseType);
    void onResponse_VendSessionComplete(MDBResponses::rType responseType);
    void onResponse_VendCashSale(MDBResponses::rType responseType);
    void onResponse_VendNegativeVendRequest(MDBResponses::rType responseType);


    void onResponse_Reader(MDBResponses::rType responseType);
    void onResponse_ReaderEnable(MDBResponses::rType responseType);
    void onResponse_ReaderDisable(MDBResponses::rType responseType);
    void onResponse_ReaderCancel(MDBResponses::rType responseType);

    void onResponse_Revalue(MDBResponses::rType responseType);
    void onResponse_RevalueRequest(MDBResponses::rType responseType);
    void onResponse_RevalueLimitRequest(MDBResponses::rType responseType);

    void onResponse_Expansion(MDBResponses::rType responseType);
    void onResponse_ExpansionID(MDBResponses::rType responseType);
};

#endif