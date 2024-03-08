#include "coinChanger.h"

coinChanger::coinChanger(uint8_t addr) : mdbDevice(addr),IPaymentDevice(), currentFunds(0.0f, 1, 2)
{
    requ = (COINDataRequest*)sendData;
}

void coinChanger::onResponse_Reset(MDBResponses::rType responseType)
{
    DBG("Response Reset");
    if(responseType == MDBResponses::ACK)
    {
        DBG("OK");
        state = MDBDeviceStates::UNINITIALIZED;
        for (size_t i = 0; i < 16; i++)
        {
            Tubes[i].active = false;
            Tubes[i].amount = 0;
            Tubes[i].canRouteTo = false;
            Tubes[i].value = mdbCurrency();
            Tubes[i].type = i;
        }
    }
}

void coinChanger::onResponse_Setup(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        properties.featureLevel = resp->setupData.ChangerFeatureLevel;
        properties.countryCode = EC16(resp->setupData.CurrencyCode);
        properties.scaleFactor = resp->setupData.CoinScalingFactor;
        properties.decimalPlaces = resp->setupData.DecimalPlaces;
        properties.routingInfo = EC16(resp->setupData.CoinTypeRouting);

        currentFunds.setup(properties.scaleFactor, properties.decimalPlaces);

        DBG("Coin Changer Config:");
        DBGf("\tFeature Lvl:        %d\r\n", resp->setupData.ChangerFeatureLevel);
        DBGf("\tCountry Code:       %d\r\n", properties.countryCode);
        DBGf("\tScale Factor:       %d\r\n", resp->setupData.CoinScalingFactor);
        DBGf("\tDecimal Places:     %d\r\n", resp->setupData.DecimalPlaces);
        DBGf("\tRoutingInfo:        %d\r\n", properties.routingInfo);
        
        for (size_t i = 0; i < responseLen - 7; i++)
        {
            uint16_t temp = resp->setupData.CoinTypeCredit[i];
            temp = EC16(temp);

            Tubes[i].active = true;
            Tubes[i].isToken = false;
            Tubes[i].canRouteTo = (properties.routingInfo & (1<<i));
            Tubes[i].amount = 0;
            Tubes[i].value = mdbCurrency();

            if(resp->setupData.CoinTypeCredit[i] == 0x00){
                DBGf("Coin %2d UNUSED\r\n", i);
                Tubes[i].active = false;
            }else if(resp->setupData.CoinTypeCredit[i] == 0xFF){
                Serial.printf("Coin %2d TOKEN %s\r\n", i, (Tubes[i].canRouteTo ? "Route Tube" : "Route Cashbox"));
                Tubes[i].isToken = true;
            }else{
                Serial.printf("Coin %2d %.2f EUR\r\n", i, mdbCurrency(temp, properties.scaleFactor, properties.decimalPlaces).getValue());
                Tubes[i].value = mdbCurrency(temp, properties.scaleFactor, properties.decimalPlaces);
            }
            
        }
        

        if(properties.featureLevel == 3){
            //needs further expansion setup
            changerSetupState = CoinChangerSetup::SETUP_EXPANSION;
        }else{
            changerSetupState = CoinChangerSetup::COIN_TYPE;
        }
    }else{
        increaseErrorCounter();
    }
}

void coinChanger::onResponse_Poll(MDBResponses::rType responseType)
{
    payoutBusy = false;
    // DBG("Response Poll");
    if(responseType == MDBResponses::ACK){
        //do nothing
        // DBG("OK");
    }
    if(responseType == MDBResponses::DATA){
        uint8_t nextIdx = 0;
        fundsChanged = false;
        
        while( (nextIdx = handleActivity(nextIdx)) );

        if(fundsChanged)
        {
            if(_paymentInterface) _paymentInterface->fundsChanged(this);
        }
    }

    if((!payoutBusy) && (changerState == CoinChanger::WAIT_PAYOUT_BUSY))
    {
        changerState = CoinChanger::PAYOUT;
    }
}

uint8_t coinChanger::handleActivity(uint8_t idx)
{
    if(idx >= responseLen){
        return 0;
    }
    if( (resp->activity.data[idx] & COIN_RESP_STATUS_MASK) == COIN_RESP_STATUS){
        //handle status bit
        DBG("Status MSG");
        switch (resp->activity.data[idx])
        {
        case COIN_RESP_STATUS_ESCROW_REQU:
            Serial.println("Escrow Request Event");
            escrowPending = true;
            break;

        case COIN_RESP_STATUS_PAYOUT_BUSY:
            DBG("Payout Busy");
            payoutBusy = true;
            break;

        case COIN_RESP_STATUS_NOCREDIT:
            Serial.println("No Credit Coin Event");
            break;
        
        case COIN_RESP_STATUS_DEFECTIVE_TUBE_SENSOR:
            Serial.println("Tube Sensor Defective");
            break;

        case COIN_RESP_STATUS_DOUBLE_ARRIVAL:
            Serial.println("Double Arrival Event");
            break;

        case COIN_RESP_STATUS_ACCEPTOR_UNPLUGGED:
            Serial.println("Acceptor Unplugged Event");
            break;

        case COIN_RESP_STATUS_TUBE_JAM:
            Serial.println("Tube Jam Event");
            break;

        case COIN_RESP_STATUS_ROM_CHKSUM_ERROR:
            Serial.println("ROM ChkSum Error Event");
            break;

        case COIN_RESP_STATUS_COIN_ROUTING_ERROR:
            Serial.println("Coin Routing Error Event");
            break;

        case COIN_RESP_STATUS_CHANGER_BUSY:
            DBG("Changer Busy");
            break;

        case COIN_RESP_STATUS_JUST_RESET:
            Serial.println("Just Reset");
            state = MDBDeviceStates::SETUP;
            changerSetupState = CoinChangerSetup::SETUP;
            break;

        case COIN_RESP_STATUS_COIN_JAM:
            Serial.println("Coin Jam Event");
            break;

        case COIN_RESP_STATUS_POSSIBLE_CREDIT_REMOVAL:
            Serial.println("Possible Credit Removal Event");
            break;
        
        default:
            Serial.println("Unknown Status");
            break;
        }
        return idx + 1;
    }else if ( (resp->activity.data[idx] & COIN_RESP_COINS_DISPENSED_MASK) == COIN_RESP_COINS_DISPENSED){
        //handle dispensed coins
        Serial.println("Coins Dispensed");
        uint8_t numDispensed = (resp->activity.data[idx] >> 4) & 0x07;
        uint8_t coinType = resp->activity.data[idx] & 0x0F;
        uint8_t tubeStatus = resp->activity.data[idx + 1];

        Tubes[coinType].amount = tubeStatus;

        for (size_t i = 0; i < numDispensed; i++)
        {
            currentFunds.sub(Tubes[coinType].value);
        }

        if(currentFunds.getValue() < 0) currentFunds.setValue(0);
                
        fundsChanged = true;

        Serial.printf("\tNum Dispensed: %d\r\n", numDispensed);
        Serial.printf("\tCoin Type:     %d\r\n", coinType);
        Serial.printf("\tCoins in Tube: %d\r\n", tubeStatus);
        Serial.printf("\tCoin Value:    %.2f EUR\r\n", Tubes[coinType].value.getValue());

        return idx + 2;
    }else if ( (resp->activity.data[idx] & COIN_RESP_COINS_DEPOSITED_MASK) == COIN_RESP_COINS_DEPOSITED){
        //handle deposited coins
        Serial.println("Coins Deposited");
        uint8_t routing = (resp->activity.data[idx] >> 4) & 0x03;
        uint8_t coinType = resp->activity.data[idx] & 0x0F;
        uint8_t tubeStatus = resp->activity.data[idx + 1];

        Tubes[coinType].amount = tubeStatus;

        currentFunds.add(Tubes[coinType].value);
        fundsChanged = true;

        Serial.printf("\tRouting:       %s\r\n", routing == 0b00 ? "Cashbox" : routing == 0b01 ? "Tubes" : routing == 0b10 ? "Not Used" : "Rejected");
        Serial.printf("\tCoin Type:     %d\r\n", coinType);
        Serial.printf("\tCoins in Tube: %d\r\n", tubeStatus);
        Serial.printf("\tCoin Value:    %.2f EUR\r\n", Tubes[coinType].value.getValue());
        return idx + 2;
    }else if ( (resp->activity.data[idx] & COIN_RESP_SLUG_MASK) == COIN_RESP_SLUG){
        //handle slug
        DBG("Slug happened");
        return idx + 1;
    }else{
        DBG("Invalid Response");
        return 0;
    }
    
}

void coinChanger::onResponse_TubeStatus(MDBResponses::rType responseType)
{
    if(responseType == MDBResponses::DATA)
    {
        DBG("Tube Status:");
        DBGf("\tFull Status:        %d\r\n", EC16(resp->tubeStatus.FullStatus));
        for (size_t i = 0; i < responseLen - 2; i++)
        {
            Tubes[i].amount = resp->tubeStatus.TubeStatus[i];

            if(Tubes[i].active){
                if(Tubes[i].isToken){
                    DBGf("Coin %2d TOKEN AMOUNT: %3d\r\n", i, Tubes[i].amount);
                }else{
                    DBGf("Coin %2d %.2f EUR AMOUNT: %3d\r\n", i, Tubes[i].value.getValue(), Tubes[i].amount);
                }
                
            }            
        }
        changerState = CoinChanger::DEVICE_ENABLED;
    }else{
        increaseErrorCounter();
    }
}

void coinChanger::onResponse_CoinType(MDBResponses::rType responseType)
{
    Serial.println("Response Coin Type");
    if(responseType == MDBResponses::ACK)
    {
        Serial.println("OK");
        state = MDBDeviceStates::READY;
        if(properties.routingInfo){
            changerState = CoinChanger::REQUEST_TUBE_STATUS;
        }else{            
            changerState = CoinChanger::DEVICE_ENABLED;
        }
        
    }else{
        increaseErrorCounter();
    }
}

void coinChanger::onResponse_Dispense(MDBResponses::rType responseType)
{
    DBG("Response Dispense");
    if(responseType == MDBResponses::ACK)
    {
        DBG("OK");    
        changerState = CoinChanger::WAIT_PAYOUT_BUSY;   
        for (size_t i = 0; i < payout.amount; i++)
        {
            currentFunds.sub(payout.value);
        }
        if(_paymentInterface) _paymentInterface->fundsChanged(this);
         
    }else{
        increaseErrorCounter();
    }
}

void coinChanger::onResponse_Expansion(MDBResponses::rType responseType)
{
    switch (lastSub)
    {
    case COIN_REQU_SUB_EXPANSION_REQUEST_ID:
        onResponse_ExpansionID(responseType);
        break;

    case COIN_REQU_SUB_EXPANSION_FEATURE_ENABLE:
        onResponse_ExpansionFeatureEnable(responseType);
        break;
    
    default:
        break;
    }
}

void coinChanger::onResponse_ExpansionID(MDBResponses::rType responseType)
{
    DBG("Response Expansion ID");
    if(responseType == MDBResponses::DATA)
    {
        
        Serial.println("Peripheral ID:");
        Serial.print("\tManufacturer Code:  "); Serial.write(resp->expansionID.ManufacturerCode, 3); Serial.println();
        Serial.print("\tSerial Number:      "); Serial.write(resp->expansionID.SerialNumber, 12); Serial.println();
        Serial.print("\tModel Number:       "); Serial.write(resp->expansionID.ModelNumber, 12); Serial.println();
        Serial.printf("\tSoftware Version:   %04X\r\n", resp->expansionID.SoftwareVersion);
        Serial.printf("\tAlternate Payout:   %s\r\n", resp->expansionID.OptionalFeatureBits_AlternatePayout ? "YES" : "NO");
        Serial.printf("\tExt. Diagnostics:   %s\r\n", resp->expansionID.OptionalFeatureBits_ExtendedDiagnostics ? "YES" : "NO");
        Serial.printf("\tContr. man. Fill:   %s\r\n", resp->expansionID.OptionalFeatureBits_ControlledManualFill ? "YES" : "NO");
        Serial.printf("\tFTL Support:        %s\r\n", resp->expansionID.OptionalFeatureBits_FTLSupport ? "YES" : "NO");
        changerSetupState = CoinChangerSetup::FEATURE_ENABLE;
    }else{
        increaseErrorCounter();
    }
}

void coinChanger::onResponse_ExpansionFeatureEnable(MDBResponses::rType responseType)
{
    DBG("Response Feature Enable");
    if(responseType == MDBResponses::ACK){
        DBG("OK");
        changerSetupState = CoinChangerSetup::COIN_TYPE;        
    }else{
        increaseErrorCounter();
    }
}
