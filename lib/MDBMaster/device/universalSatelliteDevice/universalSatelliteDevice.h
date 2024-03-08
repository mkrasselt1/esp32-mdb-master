#ifndef USD_DEVICE_H
#define USD_DEVICE_H

#include <Arduino.h>
#include "mdbDevice.h"
#include "IPayment.h"
#include "universalSatelliteDeviceSpecification.h"


namespace UniversalSatelliteDeviceSetup
{
    enum State
    {
        SETUP_DATA,
        SETUP_EXPANSION,
        FEATURE_ENABLE,
        DEVICE_ENABLE,
        DEVICE_DISABLE
    };
};

namespace UniversalSatelliteDevice
{
    enum State
    {
        DEVICE_INACTIVE,
        DEVICE_DISABLED,
        DEVICE_ENABLED,
        IDLE,
        DISPENSING,
        HOMEING
    };
};

typedef struct{
    uint8_t decimalPlaces;
    uint16_t scaleFactor;
    uint8_t featureLevel;
} universalSatelliteDeviceProperties;



class universalSatelliteDevice : public mdbDevice, public IPaymentDevice
{
public:
    //mdbDevice Interface:
    universalSatelliteDevice(uint8_t addr);
    uint8_t handleDevice();
    void handleResponseDevice(uint8_t *respData, uint16_t respLen);
    void resetDevice();

    //IPaymentDevice Interface:
    void registerPaymentInterface(IPayment * payment);
    bool requestVend(uint8_t vendID, mdbProduct product, mdbCurrency price);
    bool canRevalue();
    bool reportCashsale();
    const char* getDeviceName();
    void update();

private:
    UniversalSatelliteDeviceSetup::State USDSetupState = UniversalSatelliteDeviceSetup::SETUP_DATA;
    UniversalSatelliteDevice::State USDState = UniversalSatelliteDevice::DEVICE_INACTIVE;
    USDDataRequest* requ;
    USDDataResponse* resp;
    universalSatelliteDeviceProperties properties = {0};
    mdbProduct *currentProduct;

    void onResponse_Reset(MDBResponses::rType responseType);
    
    void onResponse_Setup(MDBResponses::rType responseType);
    
    void onResponse_Poll(MDBResponses::rType responseType);
    
    void onResponse_Vend(MDBResponses::rType responseType);
    void onResponse_VendRequestedApproved(MDBResponses::rType responseType);
    void onResponse_VendRequestedDisapproved(MDBResponses::rType responseType);
    void onResponse_VendVendItem(MDBResponses::rType responseType);
    void onResponse_VendHomeItem(MDBResponses::rType responseType);
    void onResponse_VendRequestItemStatus(MDBResponses::rType responseType);

    void onResponse_Funds(MDBResponses::rType responseType);
    void onResponse_FundsAvailable(MDBResponses::rType responseType);
    void onResponse_FundsItemPrice(MDBResponses::rType responseType);

    void onResponse_Control(MDBResponses::rType responseType);
    void onResponse_ControlDisable(MDBResponses::rType responseType);
    void onResponse_ControlEnable(MDBResponses::rType responseType);

    void onResponse_Expansion(MDBResponses::rType responseType);
    void onResponse_ExpansionID(MDBResponses::rType responseType);
    void onResponse_ExpansionFeatureEnable(MDBResponses::rType responseType);
    void onResponse_ExpansionNumberOfDataBlocks(MDBResponses::rType responseType);
    void onResponse_ExpansionSendBlock(MDBResponses::rType responseType);
    void onResponse_ExpansionRequestBlock(MDBResponses::rType responseType);

};

#endif