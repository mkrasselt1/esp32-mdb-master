#include "universalSatelliteDevice.h"


uint8_t universalSatelliteDevice::handleDevice()
{
    //todo: find out what needs to be done
    switch (state)
    {
    case MDBDeviceStates::SEARCHING:
        requ->cmd = USD_REQU_CMD_RESET;
        return USD_REQU_CMD_RESET_LEN;

    case MDBDeviceStates::UNINITIALIZED:
        requ->cmd = USD_REQU_CMD_POLL;
        return USD_REQU_CMD_POLL_LEN;

    case MDBDeviceStates::SETUP:
        switch (USDSetupState)
        {
        case UniversalSatelliteDeviceSetup::SETUP_DATA:
            requ->cmd = USD_REQU_CMD_SETUP;
            requ->sub = 1; // VMC Feature Lvl
            requ->SetupConfigData.DecimalPlaces = 2;
            requ->SetupConfigData.ScalingFactor = EC16(1);
            requ->SetupConfigData.VMCMaxApproveDenyTime = 20;

            properties.scaleFactor = 1;
            properties.decimalPlaces = 2;
            return USD_REQU_CMD_SETUP_CFG_LEN;
            break;

        case UniversalSatelliteDeviceSetup::SETUP_EXPANSION:
            requ->cmd = USD_REQU_CMD_EXPANSION;
            requ->sub = USD_REQU_SUB_EXPANSION_REQUEST_ID;
            return USD_REQU_CMD_EXPANSION_REQUEST_ID_LEN;

        case UniversalSatelliteDeviceSetup::FEATURE_ENABLE:
            requ->cmd = USD_REQU_CMD_EXPANSION;
            requ->sub = USD_REQU_SUB_EXPANSION_FEATURE_ENABLE;
            requ->EnableOptionalFeatureBits.OptionalFeatureBits_CanControlPricing = 0;
            requ->EnableOptionalFeatureBits.OptionalFeatureBits_CanFTL = 0;
            requ->EnableOptionalFeatureBits.OptionalFeatureBits_CanSelectItems = 1;
            requ->EnableOptionalFeatureBits.OptionalFeatureBitsPad = 0;
            return USD_REQU_CMD_EXPANSION_FEATURE_ENABLE_LEN;

        case UniversalSatelliteDeviceSetup::DEVICE_ENABLE:
            requ->cmd = USD_REQU_CMD_CONTROL;
            requ->sub = USD_REQU_SUB_CONTROL_ENABLE;
            return USD_REQU_CMD_CONTROL_ENABLE_LEN;

        default:
            break;
        }
        break;

    case MDBDeviceStates::READY:
        switch (USDState)
        {
        case UniversalSatelliteDevice::DEVICE_INACTIVE:
            //sum ting wong
            reset();
            break;

        case UniversalSatelliteDevice::DEVICE_DISABLED:
            //nothing todo
            break;

        case UniversalSatelliteDevice::DEVICE_ENABLED:
            requ->cmd = USD_REQU_CMD_POLL;
            return USD_REQU_CMD_POLL_LEN;
            break;        
        
        default:
            break;
        }
        
    
    default:
        break;
    }

    return 0;
}

void universalSatelliteDevice::handleResponseDevice(uint8_t *respData, uint16_t respLen)
{
    // Serial.println("Decoded:");
    MDBResponses::rType responseType = MDBResponses::NACK;
    if(respLen == 1){
        switch (respData[0])
        {
        case USD_RESP_ACK:
            Serial.println("ACK");
            responseType = MDBResponses::ACK;
            break;

        case USD_RESP_NACK:
            Serial.println("NACK");
            increaseErrorCounter();
            break;
        
        default:
            Serial.println("Unknown Response");
            increaseErrorCounter();
            break;
        }
        
    }else{
        //decode packet depending on header and length
        resp = (USDDataResponse*)respData;
        responseType = MDBResponses::DATA;
    }

    switch (lastCmd)
    {
    case USD_REQU_CMD_RESET:
        onResponse_Reset(responseType);
        break;

    case USD_REQU_CMD_SETUP:
        onResponse_Setup(responseType);
        break;

    case USD_REQU_CMD_POLL:
        onResponse_Poll(responseType);
        break;    

    case USD_REQU_CMD_VEND:
        onResponse_Vend(responseType);
        break;

    case USD_REQU_CMD_FUNDS:
        onResponse_Funds(responseType);
        break;

    case USD_REQU_CMD_CONTROL:
        onResponse_Control(responseType);
        break;

    case USD_REQU_CMD_EXPANSION:
        onResponse_Expansion(responseType);
        break;
    
    default:
        break;
    }
}

void universalSatelliteDevice::resetDevice()
{

}


