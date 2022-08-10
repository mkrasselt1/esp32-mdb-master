#include "cashLess.h"
CashLess::State mdbCashLess::state = CashLess::State::MISSING;
uint8_t mdbCashLess::featureLevel = 0;
uint8_t mdbCashLess::decimals = 0;
uint16_t mdbCashLess::cashLessFunds = 0;
uint16_t mdbCashLess::cashLessFundsPrevious = 0;
int32_t mdbCashLess::cashLessRevalueLimit = -1;
uint16_t mdbCashLess::cashLessRevalue = 0;
uint16_t mdbCashLess::cashLessRevaluePrevious = 0;
uint16_t mdbCashLess::scaleFactor = 0;
char mdbCashLess::displayMessage[32] = {'\0'};
uint16_t mdbCashLess::displayTimeout = 0;
unsigned long mdbCashLess::nextPoll = 0;
long mdbCashLess::timeout = 0;
bool mdbCashLess::messageDisplayed = true;
uint16_t mdbCashLess::priceToApprove = 0;
uint16_t mdbCashLess::productToApprove = 0;
int8_t mdbCashLess::approved = 0;

int16_t mdbCashLess::productCashSale = -1;
uint16_t mdbCashLess::priceCashSale = 0;
bool mdbCashLess::isVendSuccess = 0;

#define CL_DEBG false

void mdbCashLess::init()
{
#if CL_DEBG
    Serial.println("[CARD]\t Init:");
#endif
    mdbCashLess::state = CashLess::State::MISSING;
    mdbCashLess::nextPoll = millis() + 1000;
    mdbCashLess::timeout = millis() + 500; // to trigger full reset
    mdbCashLess::cashLessRevalueLimit = -1;
}

bool mdbCashLess::available()
{
    return (mdbCashLess::state != CashLess::State::MISSING);
}

uint8_t mdbCashLess::reset(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Reset 0x10\r\n");
#endif
    // Send Reset Message
    toSend[0] = CL_CMD_RESET;
    return 1;
}

uint8_t mdbCashLess::setupData(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Setup Data 0x1100\r\n");
#endif
    // Send Setup Message
    toSend[0] = CL_CMD_SETUP;
    toSend[1] = CL_CMD_SETUP_DATA;
    toSend[2] = 3;
    toSend[3] = 16;         // Display Cols
    toSend[4] = 2;          // Display Rows
    toSend[5] = 0b00000001; // Display full ASCII
    return 6;
}

uint8_t mdbCashLess::setupPrices(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Setup Prices 0x1101\r\n");
#endif
    // Send Setup Message
    toSend[0] = CL_CMD_SETUP;
    toSend[1] = CL_CMD_SETUP_PRICES;
    toSend[2] = highByte(80); // Max Price
    toSend[3] = lowByte(80);  // Max Price
    toSend[4] = highByte(0);  // Min Price
    toSend[5] = lowByte(0);   // Min Price
    // if (featureLevel == 3 && expandedCurrency)
    // {
    //     toSend[6] = highByte(1978); // Currency Code for Euro
    //     toSend[7] = lowByte(1978);  // Currency Code for Euro
    //     return 8;
    // }
    return 6;
}

uint8_t mdbCashLess::enable(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Enable Reader 0x1401\r\n");
#endif
    // Send reader enable Message
    toSend[0] = CL_CMD_READER;
    toSend[1] = CL_CMD_READER_ENABLE;
    return 2;
}

uint8_t mdbCashLess::revalueLimitRequest(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Revalue Limit Request 0x1401\r\n");
#endif
    // Send reader enable Message
    toSend[0] = CL_CMD_REVALUE;
    toSend[1] = CL_CMD_REVALUE_LIMIT;
    return 2;
}

uint8_t mdbCashLess::sessionEnd(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Session End 0x1304\r\n");
#endif
    // Send reader enable Message
    mdbCashLess::setFunds(0);
    mdbCashLess::approved = 0;
    toSend[0] = CL_CMD_VEND;
    toSend[1] = CL_CMD_VEND_SESSION_COM;
    return 2;
}

uint8_t mdbCashLess::disable(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Disable Reader 0x1500\r\n");
#endif
    // Send reader disable Message
    toSend[0] = CL_CMD_READER;
    toSend[1] = CL_CMD_READER_DISABLE;
    return 2;
}

uint8_t mdbCashLess::readerCancel(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Session Cancel 0x00\r\n");
#endif
    // Send Setup Message
    toSend[0] = CL_CMD_READER;
    toSend[1] = CL_CMD_READER_CANCEL;
    return 2;
}
void mdbCashLess::reqSessionComplete()
{
    mdbCashLess::state = CashLess::State::SESSION_END;
}
bool mdbCashLess::hasActiveSession()
{
    return mdbCashLess::state == CashLess::State::SESSION_IDLE;
}

uint8_t mdbCashLess::cashSale(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t CashSale 0x1305\r\n");
#endif
    // Send Setup Message
    toSend[0] = CL_CMD_VEND;
    toSend[1] = CL_CMD_VEND_CASH_SALE;
    toSend[2] = highByte(priceCashSale * scaleFactor);
    toSend[3] = lowByte(priceCashSale * scaleFactor);
    toSend[4] = highByte(productCashSale);
    toSend[5] = lowByte(productCashSale);
    productCashSale = -1;
    priceCashSale = -1;
    return 6;
}

void mdbCashLess::setVendRequest(uint16_t product, uint16_t price)
{
    if (state == CashLess::State::SESSION_IDLE)
    {
        state = CashLess::State::VEND_REQ;
        mdbCashLess::priceToApprove = price;
        mdbCashLess::productToApprove = product;
        mdbCashLess::approved = -1;
        mdbCashLess::isVendSuccess = false;
    }
    else
    {
        mdbCashLess::approved = 0;
    }
}

void mdbCashLess::logCashSale(uint16_t product, uint16_t price)
{
    mdbCashLess::priceCashSale = price;
    mdbCashLess::productCashSale = product;
}

uint16_t mdbCashLess::getVendPrice()
{
    return mdbCashLess::priceToApprove;
}

void mdbCashLess::setVendSuccess()
{
    mdbCashLess::isVendSuccess = 1;
}

uint8_t mdbCashLess::poll(uint16_t *toSend)
{
#if CL_DEBG
    // Serial.printf("[CARD]\t Poll 0x12\r\r\n");
    Serial.print(".");
#endif
    // Send Setup Message
    toSend[0] = CL_CMD_POLL;
    return 1;
}

bool mdbCashLess::displayRequestMessageAvailable()
{
    if (!mdbCashLess::messageDisplayed)
    {
        mdbCashLess::messageDisplayed = true;
        return true;
    }
    return false;
}

uint16_t mdbCashLess::getFunds()
{
    return cashLessFunds;
}

uint16_t mdbCashLess::getRevalue()
{
    return cashLessRevalue;
}

void mdbCashLess::setFunds(uint16_t newFunds)
{
    cashLessFunds = newFunds;
}

void mdbCashLess::setRevalue(uint16_t revalue)
{
#if CL_DEBG
    Serial.printf("[card] setting revalue (%d)\r\n", revalue);
#endif
    cashLessRevalue = revalue;
    if (cashLessRevalue)
    {
        state = CashLess::State::REVALUE;
    }
}

bool mdbCashLess::updateFunds(uint32_t *fundStore)
{
    if (cashLessFunds != *fundStore)
    {
        *fundStore = cashLessFunds;
        return true;
    }
    return false;
}

bool mdbCashLess::updateRevalue(uint32_t *revalueStore)
{
    if (cashLessRevalue != *revalueStore)
    {
        *revalueStore = cashLessRevalue;
        return true;
    }
    return false;
}

bool mdbCashLess::hasApproved(bool *result)
{
    *result = false;
    if (mdbCashLess::approved > 0)
    {
        *result = true;
        return true;
    }
    if (mdbCashLess::approved == 0)
    {
        *result = false;
        return true;
    }

    return false;
}

uint8_t mdbCashLess::featureId(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Feature ID 0x1700\r\n");
#endif
    // WiFi.macAddress();
    // ESP.getEfuseMac();

    const char * mac = WiFi.macAddress().c_str();

    // Send feature id message
    toSend[0] = CL_CMD_EXPANS;
    toSend[1] = CL_CMD_EXPANS_ID;
    toSend[2] = 'D';
    toSend[3] = 'I';
    toSend[4] = 'Y';
    toSend[5] = mac[0]; //Todo echte Nummer senden
    toSend[6] = mac[1];
    toSend[7] = mac[3];
    toSend[8] = mac[4];
    toSend[9] = mac[6];
    toSend[10] = mac[7];
    toSend[11] = mac[9];
    toSend[12] = mac[10];
    toSend[13] = mac[12];
    toSend[14] = mac[13];
    toSend[15] = '0';
    toSend[16] = '0';
    toSend[17] = 'E';
    toSend[18] = 'S';
    toSend[19] = 'P';
    toSend[20] = 'M';
    toSend[21] = 'D';
    toSend[22] = 'B';
    toSend[23] = 'M';
    toSend[24] = 'A';
    toSend[25] = 'S';
    toSend[26] = 'T';
    toSend[27] = 'E';
    toSend[28] = 'R';
    toSend[29] = 0x00;
    toSend[30] = 0x01;
    return 31;
}

uint8_t mdbCashLess::featureEnable(uint16_t *toSend, bool alwaysIdle, bool dataEntry, bool negativeVend, bool multiCurrency, bool largeNumber, bool fileTransferLayer)
{
#if CL_DEBG
    Serial.printf("[CARD]\t FeatureEnable 0x0F01\r\n");
#endif
    // Send feature enable Message
    toSend[0] = CL_CMD_EXPANS;
    toSend[1] = CL_CMD_EXPANS_FEAT_EN;
    toSend[2] = 0x00;
    toSend[3] = 0x00;
    toSend[4] = 0x00;
    toSend[5] = 0x00 | (alwaysIdle << 5) | (dataEntry << 4) | (negativeVend << 3) | (multiCurrency << 2) | (largeNumber << 1) | fileTransferLayer;
    return 6;
}

uint8_t mdbCashLess::revalue(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Revalue 0x15\r\n");
#endif
    if (cashLessRevalue)
    {
        toSend[0] = CL_CMD_REVALUE;
        toSend[1] = CL_CMD_REVALUE_REQ;

        // if (extendedCurrency)
        // {
        toSend[2] = highByte(cashLessRevalue * scaleFactor);
        toSend[3] = lowByte(cashLessRevalue * scaleFactor);
        // cashLessRevalue = 0;
        return 4;
        // }
        // else
        // {
        //     toSend[1] = (cashLessRevalue * scaleFactor) >> 0 & 0xFF;
        //     toSend[2] = (cashLessRevalue * scaleFactor) >> 8 & 0xFF;
        //     toSend[3] = (cashLessRevalue * scaleFactor) >> 16 & 0xFF;
        //     toSend[4] = (cashLessRevalue * scaleFactor) >> 24 & 0xFF;
        //     return 5;
        // }
    }
    toSend[0] = CL_MDB_ACK;
    return 1;
}

uint8_t mdbCashLess::vendRequest(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Vend Request 0x1300 Prod %d Price %d\r\n", mdbCashLess::productToApprove, mdbCashLess::priceToApprove);
#endif
    toSend[0] = CL_CMD_VEND;
    toSend[1] = CL_CMD_VEND_REQUEST;
    toSend[2] = highByte(mdbCashLess::priceToApprove * scaleFactor);
    toSend[3] = lowByte(mdbCashLess::priceToApprove * scaleFactor);
    toSend[4] = highByte(mdbCashLess::productToApprove);
    toSend[5] = lowByte(mdbCashLess::productToApprove);
    return 6;
}

uint8_t mdbCashLess::vendSuccess(uint16_t *toSend)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Vend Success 0x1302 \r\n");
#endif
    toSend[0] = CL_CMD_VEND;
    toSend[1] = CL_CMD_VEND_SUCCESS;
    toSend[2] = highByte(mdbCashLess::productToApprove);
    toSend[3] = lowByte(mdbCashLess::productToApprove);
    return 4;
}

uint8_t mdbCashLess::loop(uint16_t *toSend)
{
    mdbCashLess::nextPoll = millis() + 100;
    // if (mdbCashLess::timeout)
    // {
    //     Serial.println("[CARD]\t Timeout");
    //     state = CashLess::State::MISSING;
    // mdbCashLess::nextPoll += 10000;
    // }
    // #if CL_DEBG
    //     Serial.printf("[CARD]\t[%d] Loop\r\n", state);
    // #endif
    mdbCashLess::timeout = millis() + 100;

    switch (state)
    {
    case CashLess::State::MISSING:
#if CL_DEBG
        Serial.println("[CARD]\t -> reset");
#endif
        return mdbCashLess::reset(toSend);
    case CashLess::State::POLLING:
    case CashLess::State::VEND:
    case CashLess::State::READER_ENABLED:
    case CashLess::State::INACTIVE:
    case CashLess::State::SESSION_IDLE:
        if (productCashSale >= 0)
        {
            return mdbCashLess::cashSale(toSend);
        }
        else if (mdbCashLess::hasActiveSession() && cashLessRevalueLimit < 0)
        {
            return mdbCashLess::revalueLimitRequest(toSend);
        }
        else if (mdbCashLess::isVendSuccess)
        {
            mdbCashLess::isVendSuccess = 0;
            state = CashLess::State::SESSION_END;
            return mdbCashLess::vendSuccess(toSend);
        }
        else
        {
            return mdbCashLess::poll(toSend);
        }
    case CashLess::State::SETUP_DATA:
        return mdbCashLess::setupData(toSend);
    case CashLess::State::SETUP_PRICES:
        return mdbCashLess::setupPrices(toSend);
    case CashLess::State::FEATURE_ID:
        return mdbCashLess::featureId(toSend);
    case CashLess::State::FEATURE_ENABLE:
        return mdbCashLess::featureEnable(toSend, 0, 0, 0, 0, 0, 0);
    case CashLess::State::READER_DISABLED:
        state = CashLess::State::READER_ENABLED;
        return mdbCashLess::enable(toSend);
    case CashLess::State::VEND_REQ:
        state = CashLess::State::SESSION_IDLE;
        return mdbCashLess::vendRequest(toSend);
    case CashLess::State::SESSION_END:
        return mdbCashLess::sessionEnd(toSend);
    case CashLess::State::REVALUE:
        return mdbCashLess::revalue(toSend);
    default:
        Serial.printf("[CARD]\t(%d) default action: reset\r\n", state);
        mdbCashLess::nextPoll = millis() + 2000;
        state = CashLess::State::MISSING;
        return mdbCashLess::reset(toSend);
    }
    return 0;
}

void mdbCashLess::response(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    // Serial.printf("[CARD]\t State-%d:Length %d\r\n", state, len);
#endif
    mdbCashLess::timeout = 0;

    // SpecialCase Just Reste
    if (received[0] == CL_RESP_JUST_RESET && len == 2 && received[1] == CL_MDB_ACK)
    {
        state = CashLess::State::SETUP_DATA;
        return;
    }

    if (len == 1)
    {
        switch (received[0])
        {
        case CL_MDB_ACK:
            mdbCashLess::responsePoll(received, len);
            return;
        default:
            break;
        }
    }
    switch (state)
    {
    case CashLess::State::READER_ENABLED:
    case CashLess::State::POLLING:
    case CashLess::State::FEATURE_ENABLE:
    case CashLess::State::INACTIVE:
    case CashLess::State::SESSION_IDLE:
    case CashLess::State::SESSION_END:
    case CashLess::State::VEND:
    case CashLess::State::VEND_REQ:
    case CashLess::State::SETUP_PRICES:
    case CashLess::State::REVALUE:

    {
        responsePoll(received, len);
    }
    break;
    case CashLess::State::FEATURE_ID:
    {
        responseExpansionId(received, len);
    }
    break;
    case CashLess::State::SETUP_DATA:
    {
        responseSetup(received, len);
    }
    break;
    default:
        break;
    }
}

void mdbCashLess::responseSetup(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Setup Response: %d \r\n", state);
#endif
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    scaleFactor = frame.CashLessSetup.scaleFactor;
    featureLevel = frame.CashLessSetup.featureLevel;
    decimals = frame.CashLessSetup.decimalPlaces;
#if CL_DEBG
    Serial.printf("[CARD]\t Scale %01X; decimal places %01X; feature level %01X  \r\n", scaleFactor, decimals, featureLevel);
    Serial.printf("[CARD]\t Optional Features: \t\t%01X\t%01X\t%01X\t%01X \r\n",
                  frame.CashLessSetup.Option3FundRestore,
                  frame.CashLessSetup.Option2MultivendCapable,
                  frame.CashLessSetup.Option1ReaderDisplay,
                  frame.CashLessSetup.Option0CashSale);
#endif                  
    // Todo: Country Code, Max Response, Misc Options
    state = CashLess::State::SETUP_PRICES;
}

void mdbCashLess::responseExpansionId(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.printf("[CARD]\t Response Feature ID: %d \r\n", state);
#endif
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    char tmp[13] = {"\0"};
    memcpy(tmp, frame.CashLessExpansionID.manufacturer, sizeof(frame.CashLessExpansionID.manufacturer));
#if CL_DEBG
    Serial.printf("[CARD]\t Manufacturer: %s\r\n", tmp);
#endif    
    memcpy(tmp, frame.CashLessExpansionID.model, sizeof(frame.CashLessExpansionID.model));
#if CL_DEBG
    Serial.printf("[CARD]\t Model: %s\r\n", tmp);
    Serial.printf("[CARD]\t Software Version: %d\r\n", (frame.CashLessExpansionID.softwareVersionHighByte << 8 | frame.CashLessExpansionID.softwareVersionLowByte));
#endif    
    memcpy(tmp, frame.CashLessExpansionID.serialNumber, sizeof(frame.CashLessExpansionID.serialNumber));
#if CL_DEBG    
    Serial.printf("[CARD]\t Serialnumber: %s\r\n", tmp);
#endif

    state = CashLess::State::FEATURE_ENABLE;
}

void mdbCashLess::responseDisplay(uint8_t *received, uint8_t len)
{
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    mdbCashLess::displayTimeout = frame.CashLessDisplayRequest.DisplayTime;
#if CL_DEBG
    // Serial.printf("[CARD]\t Display Request (%d*100ms)\r\n", frame.CashLessDisplayRequest.DisplayTime);
#endif
    memcpy(mdbCashLess::displayMessage, &frame.CashLessDisplayRequest.DisplayMessage, 32);
    mdbCashLess::messageDisplayed = false;

    char tmp[33];
    memcpy(tmp, frame.CashLessDisplayRequest.DisplayMessage, 32);
    tmp[32] = '\0';
    Serial.printf("[CARD]\t Display Request (%d*100ms) %s\r\n", mdbCashLess::displayTimeout, tmp);
    // Serial.printf("[CARD]\t Für %d millis\r\n", mdbCashLess::displayTimeout);
}

void mdbCashLess::responsePoll(uint8_t *received, uint8_t len)
{
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    uint8_t activity = received[0];

#if CL_DEBG
    Serial.printf("[CARD]\t Poll Response - State (%d) - Activity (%02X)\r\n", mdbCashLess::state, activity);
#endif

    switch (activity)
    {
    case CL_MDB_ACK: // oder ACK
        switch (state)
        {
        case CashLess::State::SETUP_PRICES:
            if (featureLevel >= 3)
            {
                state = CashLess::State::FEATURE_ID;
            }
            else
            {
                state = CashLess::State::READER_DISABLED;
            }
            return;
        case CashLess::State::FEATURE_ID:
            state = CashLess::State::FEATURE_ENABLE;
            return;
        case CashLess::State::FEATURE_ENABLE:
            state = CashLess::State::READER_DISABLED;
            return;
        case CashLess::State::READER_ENABLED:
            state = CashLess::State::POLLING;
            return;
        case CashLess::State::SESSION_IDLE:
            state = CashLess::State::SESSION_IDLE;
            return;
        default:
            state = CashLess::State::POLLING;
            // Ack, answer follows with next poll
            return;
        }
        break;
    case CL_RESP_READER_CONFIG:
        mdbCashLess::responseSetup(received, len);
        break;
    case CL_RESP_DISP_REQ:
        mdbCashLess::responseDisplay(received, len);
        break;
    case CL_RESP_BEGIN_SESSION:
        mdbCashLess::responseBeginSession(received, len);
        break;
    case CL_RESP_SESSION_CANCEL:
        mdbCashLess::responseCancelSession(received, len);
        break;
    case CL_RESP_END_SESSION:
        mdbCashLess::responseEndSession(received, len);
        break;
    case CL_RESP_VEND_APPROVED:
        mdbCashLess::responseVendApproved(received, len);
        break;
    case CL_RESP_VEND_DENIED:
        mdbCashLess::responseVendDenied(received, len);
        break;
    case CL_RESP_REVALUE_APPROVED:
        mdbCashLess::responseRevalueApproved(received, len);
        break;
    case CL_RESP_REVALUE_LIMIT:
        mdbCashLess::responseRevalueLimit(received, len);
        break;
    case CL_RESP_REVALUE_DENIED:
        mdbCashLess::responseRevalueDenied(received, len);
        break;
    case CL_RESP_EXPANSION_ID:
        mdbCashLess::responseExpansionId(received, len);
        break;
    case CL_RESP_DATA_ENTRY_CANCEL:
        Serial.println("[CARD]\t Data Entry Cancel, whatever ?!");
        break;
    default:
        Serial.println("[CARD]\t Poll Result:");
        for (int i = 0; i < len; i++)
        {
            Serial.print(frame.raw[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    // if (len >= 1)
    // {
    //     // Todo Repeat till end
    //     Serial.printf("[CARD]\t #Activities:%d\r\n", len);
    // }
}

void mdbCashLess::responseBeginSession(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t Begin Session");
#endif

    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    if (featureLevel < 3)
    {
        uint16_t funds = frame.CashLessBeginnSession.fundsHB << 8 | frame.CashLessBeginnSession.fundsLB;
        mdbCashLess::setFunds(funds);
    }
    else
    {
#define EC16(w) ((w >> 8) | ((w << 8) & 0xFF00))
        // if(ExpandedCurrencyMode){
        mdbCashLess::setFunds(EC16(frame.CashLessLvl3BeginnSession.funds));
        // #if CL_DEBG
        //             Serial.print("[CARD]\t Media Id:");
        //             Serial.println(frame.CashLessLvl3BeginnSession.paymentMediaId);
        // #endif
        // }
    }
    mdbCashLess::state = CashLess::State::SESSION_IDLE;
}

void mdbCashLess::responseCancelSession(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t End of Session requested");
#endif
    mdbCashLess::approved = 0;
    mdbCashLess::setFunds(0);
    mdbCashLess::state = CashLess::State::SESSION_END;
}

void mdbCashLess::responseEndSession(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t End Session");
#endif

    mdbCashLess::setFunds(0);
    mdbCashLess::approved = 0;
    mdbCashLess::cashLessRevalueLimit = -1;
    mdbCashLess::state = CashLess::State::POLLING;
}

void mdbCashLess::responseVendDenied(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t Vend Denied");
#endif
    mdbCashLess::approved = 0;
}

void mdbCashLess::responseVendApproved(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t Vend Approved");
#endif
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    // uint32_t amount = frame.CashLessVendApproved.amount;
    // if(featureLevel>2 && expandedCurrency){
    //     frame.CashLessLvl3VendApprovedExpandedCurrency.amount;
    // }
    // uint16_t amount = frame.CashLessVendApproved.amount;
    // if (amount != 0xFFFF || amount != 0x0000)
    // {
    //     mdbCashLess::cashLessFunds -= amount;
    // }
    // mdbCashLess::priceToApprove = amount;
    mdbCashLess::approved = 1;
}

void mdbCashLess::responseRevalueApproved(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t Revalue Approved");
#endif
    // mdbDataRespCL_t frame;
    // memcpy(frame.raw, received, len - 1);
    // // uint32_t amount = frame.CashLessVendApproved.amount;
    // if(featureLevel>2 && expandedCurrency){
    //     frame.CashLessLvl3VendApprovedExpandedCurrency.amount;
    // }
    // uint16_t amount = frame.CashLessVendApproved.amount;
    // if (amount != 0xFFFF || amount != 0x0000)
    // {
    //     mdbCashLess::cashLessFunds -= amount;
    // }
    // mdbCashLess::priceToApprove = amount;
    mdbCashLess::cashLessRevalue = 0;
    mdbCashLess::state = CashLess::State::SESSION_IDLE;
}

void mdbCashLess::responseRevalueDenied(uint8_t *received, uint8_t len)
{
#if CL_DEBG
    Serial.println("[CARD]\t Revalue Denied");
#endif
    mdbCashLess::approved = 0;
    mdbCashLess::state = CashLess::State::SESSION_IDLE;
}

void mdbCashLess::responseRevalueLimit(uint8_t *received, uint8_t len)
{
    mdbDataRespCL_t frame;
    memcpy(frame.raw, received, len - 1);
    // if(expandedCurrency){
    cashLessRevalueLimit = frame.CashLessRevalueLimit.limitHB << 8 | frame.CashLessRevalueLimit.limitLB;
    // }else{
    // cashLessRevalueLimit = frame.CashLessRevalueLimitExpCurrency.limitHB << 24 | frame.CashLessRevalueLimitExpCurrency.limitMHB << 16| frame.CashLessRevalueLimitExpCurrency.limitMLB << 8 | frame.CashLessRevalueLimitExpCurrency.limitLB;
    // }
#if CL_DEBG
    Serial.printf("[CARD]\t Revalue Limit (%d) \r\n", cashLessRevalueLimit);
#endif
}
