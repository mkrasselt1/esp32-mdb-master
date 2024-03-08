#include "cashLess.h"

mdbCashLess::mdbCashLess(uint8_t addr) : mdbDevice(addr),IPaymentDevice(), currentFunds(0.0f, 1, 2)
{
    requ = (CashLessDataRequest*)sendData;
}

void mdbCashLess::onResponse_Reset(MDBResponses::rType responseType)
{
    if(_paymentInterface) _paymentInterface->endSession(this);
    Serial.println("Response Reset");
    if(responseType == MDBResponses::ACK)
    {
        currentFunds = mdbCurrency(0.0f, 1, 2);
        Serial.println("OK");
        state = MDBDeviceStates::UNINITIALIZED;
    }
}

void mdbCashLess::onResponse_Setup(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case CL_REQU_SUB_SETUP_CFG:
        onResponse_SetupConfig(responseType);
        break;

    case CL_REQU_SUB_SETUP_MAXMIN:
        onResponse_SetupMaxMin(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::onResponse_SetupConfig(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == CL_RESP_READER_CONFIG)
        {
            Serial.println("Reader Config:");
            Serial.printf("\tFeature Lvl:        %d\r\n", resp->ReaderConfigData.ReaderFeatureLevel);
            Serial.printf("\tCountry Code:       %d\r\n", resp->ReaderConfigData.CountryCodeHigh << 8 | resp->ReaderConfigData.CountryCodeLow);
            Serial.printf("\tScale Factor:       %d\r\n", resp->ReaderConfigData.ScaleFactor);
            Serial.printf("\tDecimal Places:     %d\r\n", resp->ReaderConfigData.DecimalPlaces);
            Serial.printf("\tAppMaxResponseTime: %d\r\n", resp->ReaderConfigData.AppMaxResponseTime);
            Serial.printf("\tCan Restore Funds:  %s\r\n", resp->ReaderConfigData.Option0_CanRestoreFunds ? "YES" : "NO");
            Serial.printf("\tCan Multivend:      %s\r\n", resp->ReaderConfigData.Option1_CanMultivend ? "YES" : "NO");
            Serial.printf("\tHas Display:        %s\r\n", resp->ReaderConfigData.Option2_HasOwnDisplay ? "YES" : "NO");
            Serial.printf("\tCan Cashsale:       %s\r\n", resp->ReaderConfigData.Option3_CanCashsale ? "YES" : "NO");

            properties.featureLevel = resp->ReaderConfigData.ReaderFeatureLevel;
            properties.scaleFactor = resp->ReaderConfigData.ScaleFactor;
            properties.decimalPlaces = resp->ReaderConfigData.DecimalPlaces;
            properties.canRevalue = resp->ReaderConfigData.Option0_CanRestoreFunds;

            currentFunds.setup(properties.scaleFactor, properties.decimalPlaces);

            CLSetupState = CashLessSetup::SETUP_EXPANSION;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_SetupMaxMin(MDBResponses::rType responseType)
{
    Serial.println("Response Setup Max Min Prices");
    if(responseType == MDBResponses::ACK){
        //all good
        CLSetupState = CashLessSetup::DEVICE_ENABLE;
        CLReaderState = CashLessReader::DEVICE_DISABLED;
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_Poll(MDBResponses::rType responseType)
{
    char tmp[33] = {0};
    uint32_t duration = 0;
    if(responseType == MDBResponses::ACK){
        //do nothing
    }
    if(responseType == MDBResponses::DATA){
        switch (resp->cmd)
        {
        case CL_RESP_JUST_RESET:
            Serial.println("Just Reset");
            state = MDBDeviceStates::SETUP;
            CLSetupState = CashLessSetup::SETUP_DATA;
            CLReaderState = CashLessReader::DEVICE_INACTIVE;
            break;

        case CL_RESP_READER_CONFIG:
            onResponse_SetupConfig(responseType);
            break;
        
        case CL_RESP_DISPLAY_REQ:
            memcpy(tmp, resp->DisplayRequest.DisplayData, sizeof(resp->DisplayRequest.DisplayData));
            duration = resp->DisplayRequest.DisplayTime;
            duration = duration * 100;
            if(_paymentInterface) _paymentInterface->requestText(this, tmp, duration);
            break;

        case CL_RESP_BEGIN_SESSION:
            if(_paymentInterface){
                currentFunds = mdbCurrency(resp->BeginSession.FundsAvailable, properties.scaleFactor, properties.decimalPlaces);
                if(_paymentInterface->startSession(this)){
                    sessionCanRevalue = true;
                    CLReaderState = CashLessReader::SESSION_IDLE;
                }else{
                    //Abort session if Payment interface can't handle stuff right now
                    CLReaderState = CashLessReader::SESSION_CANCELLING;
                }
            }else{
                //don't allow Session Starts without payment interface
                CLReaderState = CashLessReader::SESSION_CANCELLING;
            }
            break;

        case CL_RESP_SESSION_CANCEL_REQ:
            Serial.println("Session Cancel Request");    
            CLReaderState = CashLessReader::SESSION_CANCELLING;        
            break;

        case CL_RESP_VEND_APPROVED:
            if(CLReaderState == CashLessReader::VEND_REQUESTING){
                if(_paymentInterface) _paymentInterface->vendAccepted(this, mdbCurrency(resp->VendApproved.VendAmount, properties.scaleFactor, properties.decimalPlaces));
                CLReaderState = CashLessReader::VEND;
            }else{
                //we should not be here, Vend approved out of nowhere
                reset();
            }
            break;

        case CL_RESP_VEND_DENIED:
            if(CLReaderState == CashLessReader::VEND_REQUESTING){
                if(_paymentInterface) _paymentInterface->vendDenied(this);
                CLReaderState = CashLessReader::SESSION_CANCELLING;
            }else{
                //we should not be here, Vend approved out of nowhere
                reset();
            }
            break;

        case CL_RESP_REVALUE_APPROVED:
            if(CLReaderState == CashLessReader::REVALUE_REQUESTING){
                if(_paymentInterface) _paymentInterface->revalueAccepted(this);
                CLReaderState = CashLessReader::SESSION_IDLE;
            }else{
                //we should not be here, Vend approved out of nowhere
                reset();
            }
            break;

        case CL_RESP_REVALUE_DENIED:
            if(CLReaderState == CashLessReader::REVALUE_REQUESTING){
                if(_paymentInterface) _paymentInterface->revalueDenied(this);
                CLReaderState = CashLessReader::SESSION_IDLE;
                sessionCanRevalue = false;
            }else{
                //we should not be here, Vend approved out of nowhere
                reset();
            }
            break;

        case CL_RESP_REVALUE_LIMIT:
            if(CLReaderState == CashLessReader::REVALUE_REQUESTING){
                //do something
            }else{
                //we should not be here, Vend approved out of nowhere
                reset();
            }
            break;


        default:
            Serial.println("Unknown / not implemented Response Type");
            break;
        }
    }
}

void mdbCashLess::onResponse_Vend(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case CL_REQU_SUB_VEND_REQUEST:
        onResponse_VendRequest(responseType);
        break;

    case CL_REQU_SUB_VEND_CANCEL:
        onResponse_VendCancel(responseType);
        break;

    case CL_REQU_SUB_VEND_SUCCESS:
        onResponse_VendSuccess(responseType);
        break;

    case CL_REQU_SUB_VEND_FAILURE:
        onResponse_VendFailure(responseType);
        break;

    case CL_REQU_SUB_VEND_SESSION_COMPLETE:
        onResponse_VendSessionComplete(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::onResponse_VendRequest(MDBResponses::rType responseType)
{
    //Vend request might respond with data or ack
    Serial.println("Response Vend Request");
    if(responseType == MDBResponses::ACK){
        CLReaderState = CashLessReader::VEND_REQUESTING;
    }else if(responseType == MDBResponses::DATA){
        if(resp->cmd == CL_RESP_VEND_APPROVED)
        {
            if(_paymentInterface) _paymentInterface->vendAccepted(this, mdbCurrency(resp->VendApproved.VendAmount, properties.scaleFactor, properties.decimalPlaces));
            CLReaderState = CashLessReader::VEND;
        }else if(resp->cmd == CL_RESP_VEND_DENIED)
        {
            if(_paymentInterface) _paymentInterface->vendDenied(this);
            CLReaderState = CashLessReader::SESSION_CANCELLING;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_VendCancel(MDBResponses::rType responseType)
{

}

void mdbCashLess::onResponse_VendSuccess(MDBResponses::rType responseType)
{
    Serial.println("Response Vend Success");
    if(responseType == MDBResponses::ACK){
        //all good
        CLReaderState = CashLessReader::SESSION_CANCELLING;
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_VendFailure(MDBResponses::rType responseType)
{
    Serial.println("Response Vend Failure");
    if(responseType == MDBResponses::ACK){
        //all good
        CLReaderState = CashLessReader::SESSION_CANCELLING;
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_VendSessionComplete(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == CL_RESP_END_SESSION)
        {
            Serial.println("End Session");
            CLReaderState = CashLessReader::DEVICE_ENABLED;
            currentFunds = mdbCurrency(0.0f, properties.scaleFactor, properties.decimalPlaces);
            if(_paymentInterface) _paymentInterface->endSession(this);
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_VendCashSale(MDBResponses::rType responseType)
{

}

void mdbCashLess::onResponse_VendNegativeVendRequest(MDBResponses::rType responseType)
{

}

void mdbCashLess::onResponse_Expansion(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case CL_REQU_SUB_EXPANSION_ID:
        onResponse_ExpansionID(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::onResponse_ExpansionID(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        if(resp->cmd == CL_RESP_PERIPHERAL_ID)
        {
            Serial.println("Peripheral ID:");
            Serial.print("\tManufacturer Code:  "); Serial.write(resp->PeripheralID.ManufacturerCode, 3); Serial.println();
            Serial.print("\tSerial Number:      "); Serial.write(resp->PeripheralID.SerialNumber, 12); Serial.println();
            Serial.print("\tModel Number:       "); Serial.write(resp->PeripheralID.ModelNumber, 12); Serial.println();
            Serial.printf("\tSoftware Version:   %04X\r\n", EC16(resp->PeripheralID.SoftwareVersion));
            Serial.printf("\tFTL Support:        %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanFTL ? "YES" : "NO");
            Serial.printf("\tMonetary Format:    %s\r\n", resp->PeripheralID.OptionalFeatureBits_MonetaryFormat ? "32Bit" : "16Bit");
            Serial.printf("\tCan MultiCurrency:  %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanMultiCurrency ? "YES" : "NO");
            Serial.printf("\tCan Negative Vend:  %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanNegativeVend ? "YES" : "NO");
            Serial.printf("\tCan Data Entry:     %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanDataEntry ? "YES" : "NO");
            Serial.printf("\tCan Always Idle:    %s\r\n", resp->PeripheralID.OptionalFeatureBits_CanAlwaysIdle ? "YES" : "NO");

            CLSetupState = CashLessSetup::SETUP_PRICES;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_Reader(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case CL_REQU_SUB_READER_DISABLE:
        onResponse_ReaderDisable(responseType);
        break;

    case CL_REQU_SUB_READER_ENABLE:
        onResponse_ReaderEnable(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::onResponse_ReaderEnable(MDBResponses::rType responseType)
{
    Serial.println("Response Reader Enable");
    if(responseType == MDBResponses::ACK){
        //all good
        CLReaderState = CashLessReader::DEVICE_ENABLED;
        state = MDBDeviceStates::READY;
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_ReaderDisable(MDBResponses::rType responseType)
{
    Serial.println("Response Reader Disable");
    if(responseType == MDBResponses::ACK){
        //all good
        CLReaderState = CashLessReader::DEVICE_DISABLED;
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_Revalue(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case CL_REQU_SUB_REVALUE_REQUEST:
        onResponse_RevalueRequest(responseType);
        break;

    case CL_REQU_SUB_REVALUE_LIMIT:
        onResponse_RevalueLimitRequest(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::onResponse_RevalueRequest(MDBResponses::rType responseType)
{
    //Vend request might respond with data or ack
    Serial.println("Response Revalue Request");
    if(responseType == MDBResponses::ACK){
        Serial.println("Waiting for Response in POLL");
        CLReaderState = CashLessReader::REVALUE_REQUESTING;
    }else if(responseType == MDBResponses::DATA){
        if(resp->cmd == CL_RESP_REVALUE_APPROVED)
        {
            if(_paymentInterface) _paymentInterface->revalueAccepted(this);
            CLReaderState = CashLessReader::SESSION_IDLE;
        }else if(resp->cmd == CL_RESP_REVALUE_DENIED)
        {
            if(_paymentInterface) _paymentInterface->revalueDenied(this);
            CLReaderState = CashLessReader::SESSION_IDLE;
            sessionCanRevalue = false;
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

void mdbCashLess::onResponse_RevalueLimitRequest(MDBResponses::rType responseType)
{
    //Vend request might respond with data or ack
    Serial.println("Response Revalue Limit Request");
    if(responseType == MDBResponses::ACK){
        CLReaderState = CashLessReader::REVALUE_REQUESTING;
    }else if(responseType == MDBResponses::DATA){
        if(resp->cmd == CL_RESP_REVALUE_LIMIT)
        {
            
        }else{
            //reader sent wrong response
            increaseErrorCounter();
        }
    }else{
        increaseErrorCounter();
    }
}

