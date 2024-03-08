#include "cashLess.h"


uint8_t mdbCashLess::handleDevice()
{
    //todo: find out what needs to be done
    switch (state)
    {
    case MDBDeviceStates::SEARCHING:
        requ->cmd = CL_REQU_CMD_RESET;
        return CL_REQU_CMD_RESET_LEN;

    case MDBDeviceStates::UNINITIALIZED:
        requ->cmd = CL_REQU_CMD_POLL;
        return CL_REQU_CMD_POLL_LEN;

    case MDBDeviceStates::SETUP:
        switch (CLSetupState)
        {
        case CashLessSetup::SETUP_DATA:
            requ->cmd = CL_REQU_CMD_SETUP;
            requ->sub = CL_REQU_SUB_SETUP_CFG;
            requ->SetupConfigData.VMCFeatureLevel = 3;
            requ->SetupConfigData.RowsOnDisplay = 2;
            requ->SetupConfigData.ColumnsOnDisplay = 16;
            requ->SetupConfigData.DisplayInformation = 0x01; //full ASCII
            return CL_REQU_CMD_SETUP_CFG_LEN;
            break;

        case CashLessSetup::SETUP_EXPANSION:
            requ->cmd = CL_REQU_CMD_EXPANSION;
            requ->sub = CL_REQU_SUB_EXPANSION_ID;
            memcpy(requ->ExpansionID.ManufacturerCode, "PNP", strlen("PNP"));
            memset(requ->ExpansionID.ModelNumber, ' ', sizeof(requ->ExpansionID.ModelNumber));
            memcpy(requ->ExpansionID.ModelNumber, "PeanutVMC1.0", strlen("PeanutVMC1"));
            memset(requ->ExpansionID.SerialNumber, ' ', sizeof(requ->ExpansionID.SerialNumber));
            memcpy(requ->ExpansionID.SerialNumber, "1", strlen("1"));
            requ->ExpansionID.SoftwareVersion = EC16(0x0100);
            return CL_REQU_CMD_EXPANSION_ID_LEN;

        case CashLessSetup::SETUP_PRICES:
            requ->cmd = CL_REQU_CMD_SETUP;
            requ->sub = CL_REQU_SUB_SETUP_MAXMIN;
            requ->SetupConfigMaxMinPrices.MinimumPrice = mdbCurrency(0.00f, properties.scaleFactor, properties.decimalPlaces).getValue16BitMDB();
            requ->SetupConfigMaxMinPrices.MaximumPrice = mdbCurrency(10.00f, properties.scaleFactor, properties.decimalPlaces).getValue16BitMDB();
            return CL_REQU_CMD_SETUP_MAXMIN_LEN;

        case CashLessSetup::DEVICE_ENABLE:
            requ->cmd = CL_REQU_CMD_READER;
            requ->sub = CL_REQU_SUB_READER_ENABLE;
            return CL_REQU_CMD_READER_ENABLE_LEN;

        case CashLessSetup::DEVICE_DISABLE:
            requ->cmd = CL_REQU_CMD_READER;
            requ->sub = CL_REQU_SUB_READER_DISABLE;
            return CL_REQU_CMD_READER_DISABLE_LEN;

        default:
            break;
        }
        break;

    case MDBDeviceStates::READY:
        switch (CLReaderState)
        {
        case CashLessReader::DEVICE_INACTIVE:
            //sum ting wong
            reset();
            break;

        case CashLessReader::DEVICE_DISABLED:
            //nothing todo
            break;

        case CashLessReader::DEVICE_ENABLED:
            if(_paymentInterface) _paymentInterface->update(this);
            requ->cmd = CL_REQU_CMD_POLL;
            return CL_REQU_CMD_POLL_LEN;
            break;

        case CashLessReader::SESSION_IDLE:
            //TODO: check Payment interface for pending actions, else poll
            if(_paymentInterface){
                switch (_paymentInterface->updateSession(this))
                {
                case PaymentActions::NO_ACTION:
                    requ->cmd = CL_REQU_CMD_POLL;
                    return CL_REQU_CMD_POLL_LEN;
                
                case PaymentActions::REQUEST_PRODUCT:
                    currentProduct = _paymentInterface->getCurrentProduct(this);
                    if(currentProduct)
                    {
                        requ->cmd = CL_REQU_CMD_VEND;
                        requ->sub = CL_REQU_SUB_VEND_REQUEST;
                        requ->VendRequest.ItemNumber = currentProduct->getProduct16BitMDB();
                        requ->VendRequest.ItemPrice = currentProduct->getPrice()->getValue16BitMDB();
                        return CL_REQU_CMD_VEND_REQUEST_LEN;
                    }
                    break;
                
                case PaymentActions::REQUEST_REVALUE:
                    requ->cmd = CL_REQU_CMD_REVALUE;
                    requ->sub = CL_REQU_SUB_REVALUE_REQUEST;
                    requ->RevalueRequest.RevalueAmount = _paymentInterface->getPendingRevalueAmount(this).getValue16BitMDB();
                    return CL_REQU_CMD_REVALUE_REQUEST_LEN;
                
                default:
                    break;
                }
                requ->cmd = CL_REQU_CMD_POLL;
                return CL_REQU_CMD_POLL_LEN;
            }else{
                requ->cmd = CL_REQU_CMD_POLL;
                return CL_REQU_CMD_POLL_LEN;
            }
            
            break;

        case CashLessReader::REVALUE_REQUESTING:
        case CashLessReader::VEND_REQUESTING:
            //Poll until we obtain response to poll or app max response time runs out
            requ->cmd = CL_REQU_CMD_POLL;
            return CL_REQU_CMD_POLL_LEN;

        case CashLessReader::VEND:
            //TODO: check Payment interface for pending actions, else poll
            if(_paymentInterface){
                switch (_paymentInterface->updateDispense(this))
                {
                case PaymentActions::NO_ACTION:
                    requ->cmd = CL_REQU_CMD_POLL;
                    return CL_REQU_CMD_POLL_LEN;
                
                case PaymentActions::PRODUCT_DISPENSE_SUCCESS:
                    requ->cmd = CL_REQU_CMD_VEND;
                    requ->sub = CL_REQU_SUB_VEND_SUCCESS;
                    requ->VendSuccess.ItemNumber = _paymentInterface->getCurrentProduct(this)->getProduct16BitMDB();
                    return CL_REQU_CMD_VEND_SUCCESS_LEN;
                
                case PaymentActions::PRODUCT_DISPENSE_FAILURE:
                    requ->cmd = CL_REQU_CMD_VEND;
                    requ->sub = CL_REQU_SUB_VEND_FAILURE;
                    return CL_REQU_CMD_VEND_FAILURE_LEN;                    
                
                default:
                    break;
                }
            }

            requ->cmd = CL_REQU_CMD_POLL;
            return CL_REQU_CMD_POLL_LEN;   

        case CashLessReader::SESSION_CANCELLING:
            //TODO: check is cancelling session is allowed
            requ->cmd = CL_REQU_CMD_VEND;
            requ->sub = CL_REQU_SUB_VEND_SESSION_COMPLETE;
            return CL_REQU_CMD_VEND_SESSION_COMPLETE_LEN;
            break;

        
        
        default:
            break;
        }
        
    
    default:
        break;
    }

    return 0;
}

void mdbCashLess::handleResponseDevice(uint8_t *respData, uint16_t respLen)
{
    // Serial.println("Decoded:");
    MDBResponses::rType responseType = MDBResponses::NACK;
    if(respLen == 1){
        switch (respData[0])
        {
        case CL_RESP_ACK:
            responseType = MDBResponses::ACK;
            break;

        case CL_RESP_NACK:
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
        resp = (CashLessDataResponse*)respData;
        responseType = MDBResponses::DATA;
    }

    switch (lastCmd)
    {
    case CL_REQU_CMD_RESET:
        onResponse_Reset(responseType);
        break;

    case CL_REQU_CMD_POLL:
        onResponse_Poll(responseType);
        break;

    case CL_REQU_CMD_SETUP:
        onResponse_Setup(responseType);
        break;

    case CL_REQU_CMD_EXPANSION:
        onResponse_Expansion(responseType);
        break;

    case CL_REQU_CMD_READER:
        onResponse_Reader(responseType);
        break;

    case CL_REQU_CMD_VEND:
        onResponse_Vend(responseType);
        break;

    case CL_REQU_CMD_REVALUE:
        onResponse_Revalue(responseType);
        break;
    
    default:
        break;
    }
}

void mdbCashLess::resetDevice()
{

}


