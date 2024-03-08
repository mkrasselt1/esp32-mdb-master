#include "coinChanger.h"


uint8_t coinChanger::handleDevice()
{
    //todo: find out what needs to be done
    switch (state)
    {
    case MDBDeviceStates::SEARCHING:
        requ->cmd = COIN_REQU_CMD_RESET;
        return COIN_REQU_CMD_RESET_LEN;

    case MDBDeviceStates::UNINITIALIZED:
        requ->cmd = COIN_REQU_CMD_POLL;
        return COIN_REQU_CMD_POLL_LEN;

    case MDBDeviceStates::SETUP:
        switch (changerSetupState)
        {
        case CoinChangerSetup::SETUP:
            requ->cmd = COIN_REQU_CMD_SETUP;
            return COIN_REQU_CMD_SETUP_LEN;
            break;

        case CoinChangerSetup::SETUP_EXPANSION:
            requ->cmd = COIN_REQU_CMD_EXPANSION;
            requ->expansion.subCmd = COIN_REQU_SUB_EXPANSION_REQUEST_ID;
            return COIN_REQU_CMD_EXPANSION_REQUEST_ID_LEN;

        case CoinChangerSetup::FEATURE_ENABLE:
            requ->cmd = COIN_REQU_CMD_EXPANSION;
            requ->expansion.subCmd = COIN_REQU_SUB_EXPANSION_FEATURE_ENABLE;
            return COIN_REQU_CMD_EXPANSION_FEATURE_ENABLE_LEN;

        case CoinChangerSetup::COIN_TYPE:
            requ->cmd = COIN_REQU_CMD_COIN_TYPE;
            requ->coinType.coinEnable = 0xFFFF;
            requ->coinType.manualDispenseEnable = 0xFFFF;
            return COIN_REQU_CMD_COIN_TYPE_LEN;

        default:
            break;
        }
        break;

    case MDBDeviceStates::READY:
        switch (changerState)
        {
        case CoinChanger::DEVICE_INACTIVE:
            //sum ting wong
            reset();
            break;

        case CoinChanger::DEVICE_DISABLED:
            //nothing todo
            break;

        case CoinChanger::DEVICE_ENABLED:
            if(_paymentInterface)
            {
                switch (_paymentInterface->update(this))
                {
                case PaymentActions::NO_ACTION:
                    break;  

                case PaymentActions::REQUEST_PRODUCT:
                    currentProduct = _paymentInterface->getCurrentProduct(this);
                    changerState = CoinChanger::VEND;
                    break;      

                case PaymentActions::REQUEST_REVALUE:
                    changerState = CoinChanger::PAYOUT;
                    payoutAmount = _paymentInterface->getPendingRevalueAmount(this).getValue();
                    break;                   
                
                default:
                    break;
                }   
            }
            requ->cmd = COIN_REQU_CMD_POLL;
            return COIN_REQU_CMD_POLL_LEN;

        case CoinChanger::VEND:
            if(_paymentInterface){
                switch (_paymentInterface->updateDispense(this))
                {
                case PaymentActions::NO_ACTION:
                    break;
                
                case PaymentActions::PRODUCT_DISPENSE_SUCCESS:
                    changerState = CoinChanger::DEVICE_ENABLED;
                    currentFunds.sub(*currentProduct->getPrice());
                    _paymentInterface->endSession(this);
                    break;    

                case PaymentActions::PRODUCT_DISPENSE_FAILURE:
                    changerState = CoinChanger::DEVICE_ENABLED;
                    _paymentInterface->endSession(this);
                    break;                
                
                default:
                    break;
                }                
            }
            requ->cmd = COIN_REQU_CMD_POLL;
            return COIN_REQU_CMD_POLL_LEN;

        case CoinChanger::REQUEST_TUBE_STATUS:
            requ->cmd = COIN_REQU_CMD_TUBE_STATUS;
            return COIN_REQU_CMD_TUBE_STATUS_LEN;
            break;  

        case CoinChanger::PAYOUT:
            {
                
                calculateDispense();
                if(payout.amount == 0)
                {
                    changerState = CoinChanger::DEVICE_ENABLED;
                    requ->cmd = COIN_REQU_CMD_POLL;
                    return COIN_REQU_CMD_POLL_LEN;
                }else{
                    requ->cmd = COIN_REQU_CMD_DISPENSE;
                    requ->dispense.amountCoinType = (payout.amount<<4) | (payout.type & 0xF);
                    return COIN_REQU_CMD_DISPENSE_LEN;
                }
                
            }            
            break;   

        case CoinChanger::WAIT_PAYOUT_BUSY:
            requ->cmd = COIN_REQU_CMD_POLL;
            return COIN_REQU_CMD_POLL_LEN;
        
        default:
            break;
        }
        
    
    default:
        break;
    }

    return 0;
}

void coinChanger::handleResponseDevice(uint8_t *respData, uint16_t respLen)
{
    // Serial.println("Decoded:");
    responseLen = 0;
    MDBResponses::rType responseType = MDBResponses::NACK;
    if(respLen == 1){
        switch (respData[0])
        {
        case COIN_RESP_ACK:
            // DBG("ACK");
            responseType = MDBResponses::ACK;
            break;

        case COIN_RESP_NACK:
            DBG("NACK");
            increaseErrorCounter();
            break;
        
        default:
            DBG("Unknown Response");
            increaseErrorCounter();
            break;
        }
        
    }else{
        //decode packet depending on header and length
        resp = (COINDataResponse*)respData;
        responseType = MDBResponses::DATA;
        responseLen = respLen - 1;
    }

    switch (lastCmd)
    {
    case COIN_REQU_CMD_RESET:
        onResponse_Reset(responseType);
        break;

    case COIN_REQU_CMD_SETUP:
        onResponse_Setup(responseType);
        break;

    case COIN_REQU_CMD_TUBE_STATUS:
        onResponse_TubeStatus(responseType);
        break;

    case COIN_REQU_CMD_POLL:
        onResponse_Poll(responseType);
        break;    

    case COIN_REQU_CMD_COIN_TYPE:
        onResponse_CoinType(responseType);
        break;

    case COIN_REQU_CMD_DISPENSE:
        onResponse_Dispense(responseType);
        break;

    case COIN_REQU_CMD_EXPANSION:
        onResponse_Expansion(responseType);
        break;
    
    default:
        break;
    }
}

void coinChanger::resetDevice()
{

}

void coinChanger::calculateDispense()
{
    //calculate mos suitable coin type and amount to dispense
    int lastCoin = -1;
    for (size_t i = 0; i < 16; i++)
    {
        if( (Tubes[i].active) && (!Tubes[i].isToken) && (Tubes[i].amount > 0) && (Tubes[i].value.getValue() > 0) )
        {
            if(Tubes[i].value.getValue() < payoutAmount)
            {
                if(lastCoin < 0)
                {
                    lastCoin = i;
                }else{
                    if(Tubes[i].value.getValue() > Tubes[lastCoin].value.getValue())
                    {
                        lastCoin = i;
                    }
                }
            }            
        }
    }
    if(lastCoin < 0)
    {
        payout.amount = 0;
        if(payoutAmount)
        {
            if(_paymentInterface) _paymentInterface->payoutEnd(this, false);
        }else{
            if(_paymentInterface) _paymentInterface->payoutEnd(this, true);
        }
        Serial.println("End of Dispense");
    }else{
        payout = Tubes[lastCoin];
        payout.amount = payoutAmount / Tubes[lastCoin].value.getValue();
        payoutAmount -= (Tubes[lastCoin].value.getValue() * payout.amount);
        Serial.printf("Dispensing COIN %2d x %2d\r\n", payout.type, payout.amount);
    }
}


