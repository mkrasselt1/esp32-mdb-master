#include "universalSatelliteDevice.h"

universalSatelliteDevice::universalSatelliteDevice(uint8_t addr) : mdbDevice(addr),IPaymentDevice()
{
    requ = (USDDataRequest*)sendData;
}

void universalSatelliteDevice::onResponse_Reset(MDBResponses::rType responseType)
{
    Serial.println("Response Reset");
    if(responseType == MDBResponses::ACK)
    {
        Serial.println("OK");
        state = MDBDeviceStates::UNINITIALIZED;
    }
}

void universalSatelliteDevice::onResponse_Setup(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == USD_RESP_SETUP_DATA)
        {
            Serial.println("USD Config:");
            Serial.printf("\tFeature Lvl:        %d\r\n", resp->SetupData.FeatureLevel);
            Serial.printf("\tMaximum Price:      %.2f\r\n", mdbCurrency(resp->SetupData.MaximumPrice, properties.scaleFactor, properties.decimalPlaces).getValue());
            Serial.printf("\tItem Number:        %d\r\n", EC16(resp->SetupData.ItemNumber));
            Serial.printf("\tMax Response Time:  %d\r\n", resp->SetupData.MaximumResponseTime);
            
            properties.featureLevel = resp->SetupData.FeatureLevel;
            USDSetupState = UniversalSatelliteDeviceSetup::SETUP_EXPANSION;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_Poll(MDBResponses::rType responseType)
{
    char tmp[33] = {0};
    if(responseType == MDBResponses::ACK){
        //do nothing
    }
    if(responseType == MDBResponses::DATA){
        switch (resp->cmd)
        {
        case USD_RESP_JUST_RESET:
            Serial.println("Just Reset");
            state = MDBDeviceStates::SETUP;
            USDSetupState = UniversalSatelliteDeviceSetup::SETUP_DATA;
            USDState = UniversalSatelliteDevice::DEVICE_INACTIVE;
            break;

        case USD_RESP_VEND_REQUEST:
            //Handle Requested vend
            break;

        case USD_RESP_VEND_HOME_SUCCESS:
            //Vend or Homing was Successful
            break;

        case USD_RESP_VEND_HOME_FAIL:
            //Vend or Homing Failed
            break;

        case USD_RESP_SETUP_DATA:
            onResponse_Setup(responseType);
            break;

        case USD_RESP_ITEM_PRICE_REQUEST:
            //USD Requesting Item Price Info
            break;

        case USD_RESP_ERROR:
            //USD Reports Error
            break;

        case USD_RESP_PERIPHERAL_ID:
            onResponse_ExpansionID(responseType);
            break;

        case USD_RESP_STATUS_RESPONSE:
            //USD Reports requested Status
            break;


        default:
            Serial.println("Unknown / not implemented Response Type");
            break;
        }
    }
}

void universalSatelliteDevice::onResponse_Vend(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case USD_REQU_SUB_VEND_REQUESTED_APPROVED:
        onResponse_VendRequestedApproved(responseType);
        break;

    case USD_REQU_SUB_VEND_REQUESTED_DISAPPROVED:
        onResponse_VendRequestedDisapproved(responseType);
        break;

    case USD_REQU_SUB_VEND_VEND_ITEM:
        onResponse_VendVendItem(responseType);
        break;

    case USD_REQU_SUB_VEND_HOME_ITEM:
        onResponse_VendHomeItem(responseType);
        break;

    case USD_REQU_SUB_VEND_REQUEST_ITEM_STATUS:
        onResponse_VendRequestItemStatus(responseType);
        break;
    
    default:
        break;
    }
}

void universalSatelliteDevice::onResponse_VendRequestedApproved(MDBResponses::rType responseType)
{
    Serial.println("Response Vend Requested Approved");
    if(responseType == MDBResponses::ACK){
        USDState = UniversalSatelliteDevice::DISPENSING;
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_VendRequestedDisapproved(MDBResponses::rType responseType)
{
    Serial.println("Response Vend Requested Disapproved");
    if(responseType == MDBResponses::ACK){
        USDState = UniversalSatelliteDevice::IDLE;
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_VendVendItem(MDBResponses::rType responseType)
{
    Serial.println("Response Vend Item Request");
    if(responseType == MDBResponses::ACK){
        USDState = UniversalSatelliteDevice::DISPENSING;
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_VendHomeItem(MDBResponses::rType responseType)
{
    Serial.println("Response Home Item Request");
    if(responseType == MDBResponses::ACK){
        USDState = UniversalSatelliteDevice::HOMEING;
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_VendRequestItemStatus(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == USD_RESP_STATUS_RESPONSE)
        {
            Serial.println("Item Status:");
            Serial.printf("\tItem Number:        %04X\r\n", EC16(resp->StatusResponse.ItemNumber));
            Serial.printf("\tSold Out:           %s\r\n", resp->StatusResponse.ReasonBits_SoldOut ? "YES" : "NO");
            Serial.printf("\tMotor/Actuator Jam: %s\r\n", resp->StatusResponse.ReasonBits_Jam ? "YES" : "NO");
            Serial.printf("\tNon-Existend:       %s\r\n", resp->StatusResponse.ReasonBits_NonExistend ? "YES" : "NO");
            Serial.printf("\tInvalid Range:      %s\r\n", resp->StatusResponse.ReasonBits_InvalidRange ? "YES" : "NO");
            Serial.printf("\tHealth/Safety Err:  %s\r\n", resp->StatusResponse.ReasonBits_HealthSafety ? "YES" : "NO");
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}


void universalSatelliteDevice::onResponse_Funds(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case USD_REQU_SUB_FUNDS_AVAILABLE:
        onResponse_FundsAvailable(responseType);
        break;

    case USD_REQU_SUB_FUNDS_ITEM_PRICE:
        onResponse_FundsItemPrice(responseType);
        break;
    
    default:
        break;
    }
}

void universalSatelliteDevice::onResponse_FundsAvailable(MDBResponses::rType responseType)
{
    Serial.println("Response Funds Available");
    if(responseType == MDBResponses::ACK){
        Serial.println("OK");
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_FundsItemPrice(MDBResponses::rType responseType)
{
    Serial.println("Response Item Price");
    if(responseType == MDBResponses::ACK){
        Serial.println("OK");
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_Control(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case USD_REQU_SUB_CONTROL_DISABLE:
        onResponse_ControlDisable(responseType);
        break;

    case USD_REQU_SUB_CONTROL_ENABLE:
        onResponse_ControlEnable(responseType);
        break;
    
    default:
        break;
    }
}

void universalSatelliteDevice::onResponse_ControlDisable(MDBResponses::rType responseType)
{
    Serial.println("Response Control Disable");
    if(responseType == MDBResponses::ACK){
        Serial.println("OK");
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_ControlEnable(MDBResponses::rType responseType)
{
    Serial.println("Response Control Enable");
    if(responseType == MDBResponses::ACK){
        Serial.println("OK");
        state = MDBDeviceStates::READY;
        USDState = UniversalSatelliteDevice::DEVICE_ENABLED;
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_Expansion(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case USD_REQU_SUB_EXPANSION_REQUEST_ID:
        onResponse_ExpansionID(responseType);
        break;

    case USD_REQU_SUB_EXPANSION_FEATURE_ENABLE:
        onResponse_ExpansionFeatureEnable(responseType);
        break;
    
    default:
        break;
    }
}

void universalSatelliteDevice::onResponse_ExpansionID(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == USD_RESP_PERIPHERAL_ID)
        {
            Serial.println("Peripheral ID:");
            Serial.print("\tManufacturer Code:  "); Serial.write(resp->PeripheralID.ManufacturerIDCode, 3); Serial.println();
            Serial.print("\tSerial Number:      "); Serial.write(resp->PeripheralID.SerialNumber, 12); Serial.println();
            Serial.print("\tModel Number:       "); Serial.write(resp->PeripheralID.ModelNumber, 12); Serial.println();
            Serial.printf("\tSoftware Version:  %04X\r\n", EC16(resp->PeripheralID.SoftwareVersion));
            Serial.printf("\tCan Store Prices:  %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanControlPricing ? "YES" : "NO");
            Serial.printf("\tCan Select Items:  %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanSelectItems ? "YES" : "NO");
            Serial.printf("\tCan FTL:           %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanFTL ? "YES" : "NO");

            USDSetupState = UniversalSatelliteDeviceSetup::FEATURE_ENABLE;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void universalSatelliteDevice::onResponse_ExpansionFeatureEnable(MDBResponses::rType responseType)
{
    Serial.println("Response Control Enable");
    if(responseType == MDBResponses::ACK){
        Serial.println("OK");
    }else{
        increaseErrorCounter();
    }
}
