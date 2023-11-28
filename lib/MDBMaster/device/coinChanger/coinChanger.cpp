#include "coinChanger.h"
uint8_t mdbCoinChanger::state = CC_STATE_MISSING;
uint8_t mdbCoinChanger::featureLevel = 0;
uint8_t mdbCoinChanger::decimals = 0;
uint8_t mdbCoinChanger::coinChannelValue[16] = {0};
uint8_t mdbCoinChanger::tubeLevel[16] = {0};
uint8_t mdbCoinChanger::fakeCoins = 0;
uint16_t mdbCoinChanger::coinFunds = 0;
uint16_t mdbCoinChanger::coinPayOut = 0;
uint16_t mdbCoinChanger::coinPayOutPrevious = 0;
uint16_t mdbCoinChanger::scaleFactor = 0;
unsigned long mdbCoinChanger::nextPoll = 0;
long mdbCoinChanger::timeout = 0;
long mdbCoinChanger::nextTubePoll = 20000;

#define CC_DEBG false

void mdbCoinChanger::init()
{
    mdbCoinChanger::state = CC_STATE_MISSING;
    mdbCoinChanger::nextPoll = millis() + 1000;
    mdbCoinChanger::timeout = millis() + 500;
}

uint8_t mdbCoinChanger::reset(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Reset 0x08\r\n");
#endif
    // Send Reset Message
    toSend[0] = CC_CMD_RESET;
    return 1;
}

uint8_t mdbCoinChanger::setup(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Setup 0x09\r\n");
#endif
    // Send Setup Message
    toSend[0] = CC_CMD_SETUP_CFG;
    return 1;
}

uint8_t mdbCoinChanger::coinEnable(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Coin Enable 0x0C\r\n");
#endif
    // Todo: select coin channels to be accepted
    toSend[0] = CC_CMD_COIN_ACC;
    toSend[1] = 0xFF;
    toSend[2] = 0xFF;
    toSend[3] = 0xFF;
    toSend[4] = 0xFF;
    return 5;
}

uint8_t mdbCoinChanger::tubeStatus(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Tube Status 0x0A\r\n");
#endif
    toSend[0] = CC_CMD_TUBE_STATUS;
    return 1;
}

uint8_t mdbCoinChanger::poll(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Poll 0x0B\r\n");
#endif
    toSend[0] = CC_CMD_POLL;
    return 1;
}

uint16_t mdbCoinChanger::getFunds()
{
    return coinFunds;
}

void mdbCoinChanger::setFunds(uint16_t newFunds)
{
    coinFunds = newFunds;
}

void mdbCoinChanger::setPayOut(uint16_t payout)
{
    coinPayOut = payout;
    state = CC_STATE_ESCROW;
}

bool mdbCoinChanger::updateFunds(uint32_t *fundStore)
{
    if (coinFunds != *fundStore)
    {
        *fundStore = getFunds();
        return true;
    }
    return false;
}

bool mdbCoinChanger::getPayOutChanged()
{
    if (coinPayOut != coinPayOutPrevious)
    {
        coinPayOutPrevious = coinPayOut;
        return true;
    }
    return false;
}

uint8_t mdbCoinChanger::featureId(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Feature ID 0x0F00\r\n");
#endif
    // Send feature id message
    toSend[0] = CC_CMD_EXPANS;
    toSend[1] = CC_CMD_EXPANS_ID;
    return 2;
}

uint8_t mdbCoinChanger::featureEnable(uint16_t *toSend, bool AlternatePayout, bool ManualInfill, bool Diagnose, bool FTL)
{
#if CC_DEBG
    Serial.printf("\t[COIN] FeatureEnable 0x0F01\r\n");
#endif
    // Send feature enable Message
    toSend[0] = CC_CMD_EXPANS;
    toSend[1] = CC_CMD_EXPANS_FEAT_E;
    toSend[2] = 0x00;
    toSend[3] = 0x00 | (FTL << 3) | (Diagnose << 2) | (ManualInfill << 1) | AlternatePayout;
    return 4;
}

uint8_t mdbCoinChanger::escrow(uint16_t *toSend)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Escrow ID 0x0D\r\n");
#endif
    if (coinPayOut)
    {
        for (int chanel = 0x0F; chanel >= 0; chanel--)
        {
            int channelValue = mdbCoinChanger::getCoinChannelValue(chanel);
            int coinTubeAmount = tubeLevel[chanel] - 2;
            if (channelValue && coinPayOut >= channelValue && coinTubeAmount > 2)
            {
                int amount = coinPayOut / channelValue;
                amount = min(coinTubeAmount, amount); // limit coin amount to tube content
                amount = min(amount, 10);             // limit payout coin amount
                tubeLevel[chanel] -= amount;
                coinPayOut -= amount * channelValue;
                Serial.printf("\t[COIN] Payout: %d Cent x %d Times\r\n", channelValue, amount);
                Serial.printf("\t[COIN] Remaining Payout: %d\r\n", coinPayOut);
                // state = CC_STATE_ACTIVE;
                toSend[0] = CC_CMD_DISPENSE;
                toSend[1] = (amount << 4) | chanel;
                return 2;
            }
        }
        // no change coin left available
        state = CC_STATE_ACTIVE;
        coinPayOut = 0;
    }
    else
    {
        state = CC_STATE_ACTIVE;
    }
    // toSend[0] = CC_MDB_ACK;
    return poll(toSend);
}

uint16_t mdbCoinChanger::getCoinChannelValue(uint8_t chanel)
{
    if (chanel > (sizeof(coinChannelValue) / sizeof(uint8_t)))
    {
        return 0;
    }
    // Todo: maximum accepted Value inklusive Token)
    if ((scaleFactor * coinChannelValue[chanel]) > 200)
    {
        return 0;
    }
    return (uint16_t)scaleFactor * coinChannelValue[chanel];
}

uint8_t mdbCoinChanger::loop(uint16_t *toSend)
{
    mdbCoinChanger::nextPoll = millis() + 100;
    mdbCoinChanger::timeout = millis() + 50;
    // if(mdbCoinChanger::timeout){
    //     state = CC_STATE_MISSING;
    //     nextPoll += 10000;
    // }
    
    
    // if (nextTubePoll <= millis() && state != CC_STATE_ESCROW && state != CC_STATE_MISSING)
    // {
    //     state = CC_STATE_TUBE_STATUS;
    //     nextTubePoll += 2000;
    // }
#if CC_DEBG
    // Serial.printf("\t[COIN] Loop\r\n");
#endif
    
    switch (state)
    {
    case CC_STATE_MISSING:
        state = CC_STATE_RESET;
        return mdbCoinChanger::reset(toSend);
    case CC_STATE_RESET:
        return mdbCoinChanger::poll(toSend);
    case CC_STATE_SETUP:
        return mdbCoinChanger::setup(toSend);
    case CC_STATE_FEATURE_ID:
        return mdbCoinChanger::featureId(toSend);
    case CC_STATE_FEATURE_ENABLE:
        return mdbCoinChanger::featureEnable(toSend, 0, 0, 0, 0); // 1, 1, 1, 1);
    case CC_STATE_TUBE_STATUS:
        return mdbCoinChanger::tubeStatus(toSend);
    case CC_STATE_COIN_ENABLE:
        state = CC_STATE_ACTIVE;
        return mdbCoinChanger::coinEnable(toSend);
    case CC_STATE_ESCROW:
        return mdbCoinChanger::escrow(toSend);
    case CC_STATE_ACTIVE:
        return mdbCoinChanger::poll(toSend);
    default:
        return mdbCoinChanger::reset(toSend);
    }
    return 0;
}

void mdbCoinChanger::response(uint8_t *received, uint8_t len)
{
#if CC_DEBG
    Serial.printf("\t[COIN] State: %d \r\n", state);
#endif
    mdbCoinChanger::timeout = 0;
    switch (state)
    {
    case CC_STATE_MISSING:
    {
        if (received[0] == CC_MDB_ACK)
        {
            state = CC_STATE_SETUP;
            nextPoll = millis() + 750; // longer time for startup
        }
        else
        {
            state = CC_STATE_MISSING;
        }
        break;
    }
    case CC_STATE_RESET:
    {
        if (received[0] == CC_RESP_JUST_RESET)
        {
            state = CC_STATE_SETUP;
        }
        else
        {
            state = CC_STATE_MISSING;
        }
        break;
    }
    case CC_STATE_SETUP:
    {
        mdbCoinChanger::responseSetup(received, len);
    }
        break;
    case CC_STATE_FEATURE_ID:
    {
        // state = CC_STATE_FEATURE_ENABLE; //todo find bug
        state = CC_STATE_COIN_ENABLE;
        break;
    }
    case CC_STATE_FEATURE_ENABLE:
    {
        // state = CC_STATE_TUBE_STATUS; von oben
        state = CC_STATE_COIN_ENABLE;
        break;
    }
    case CC_STATE_ESCROW:
    case CC_STATE_ACTIVE:
    {
        mdbCoinChanger::responsePoll(received, len);
        break;
    }
    case CC_STATE_TUBE_STATUS:
    {
        mdbCoinChanger::responseTube(received, len);
        state = CC_STATE_COIN_ENABLE;
        break;
    }
    default:
        Serial.write((char *)received, len);
        Serial.println();
    }
}

void mdbCoinChanger::responseTube(uint8_t *received, uint8_t len)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Tube Response: %d \r\n", state);
#endif
    mdbDataRespCC_t frame;
    memcpy(frame.raw, received, len - 1);
#if CC_DEBG
    Serial.printf("\t[COIN] Tube 00-15 Full: \t\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X \r\n",
                  frame.CoinChangerTubeStatus.CoinB00TubeFull,
                  frame.CoinChangerTubeStatus.CoinB01TubeFull,
                  frame.CoinChangerTubeStatus.CoinB02TubeFull,
                  frame.CoinChangerTubeStatus.CoinB03TubeFull,
                  frame.CoinChangerTubeStatus.CoinB04TubeFull,
                  frame.CoinChangerTubeStatus.CoinB05TubeFull,
                  frame.CoinChangerTubeStatus.CoinB06TubeFull,
                  frame.CoinChangerTubeStatus.CoinB07TubeFull,
                  frame.CoinChangerTubeStatus.CoinB08TubeFull,
                  frame.CoinChangerTubeStatus.CoinB09TubeFull,
                  frame.CoinChangerTubeStatus.CoinB10TubeFull,
                  frame.CoinChangerTubeStatus.CoinB11TubeFull,
                  frame.CoinChangerTubeStatus.CoinB12TubeFull,
                  frame.CoinChangerTubeStatus.CoinB13TubeFull,
                  frame.CoinChangerTubeStatus.CoinB14TubeFull,
                  frame.CoinChangerTubeStatus.CoinB15TubeFull);
#endif

    tubeLevel[0x00] = frame.CoinChangerTubeStatus.CoinB00Level;
    tubeLevel[0x01] = frame.CoinChangerTubeStatus.CoinB01Level;
    tubeLevel[0x02] = frame.CoinChangerTubeStatus.CoinB02Level;
    tubeLevel[0x03] = frame.CoinChangerTubeStatus.CoinB03Level;
    tubeLevel[0x04] = frame.CoinChangerTubeStatus.CoinB04Level;
    tubeLevel[0x05] = frame.CoinChangerTubeStatus.CoinB05Level;
    tubeLevel[0x06] = frame.CoinChangerTubeStatus.CoinB06Level;
    tubeLevel[0x07] = frame.CoinChangerTubeStatus.CoinB07Level;
    tubeLevel[0x08] = frame.CoinChangerTubeStatus.CoinB08Level;
    tubeLevel[0x09] = frame.CoinChangerTubeStatus.CoinB09Level;
    tubeLevel[0x0A] = frame.CoinChangerTubeStatus.CoinB10Level;
    tubeLevel[0x0B] = frame.CoinChangerTubeStatus.CoinB11Level;
    tubeLevel[0x0C] = frame.CoinChangerTubeStatus.CoinB12Level;
    tubeLevel[0x0D] = frame.CoinChangerTubeStatus.CoinB13Level;
    tubeLevel[0x0E] = frame.CoinChangerTubeStatus.CoinB14Level;
    tubeLevel[0x0F] = frame.CoinChangerTubeStatus.CoinB15Level;

#if CC_DEBG
    Serial.printf("\t[COIN] Tube 00-15 level: \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d \r\n",
                  tubeLevel[0x00],
                  tubeLevel[0x01],
                  tubeLevel[0x02],
                  tubeLevel[0x03],
                  tubeLevel[0x04],
                  tubeLevel[0x05],
                  tubeLevel[0x06],
                  tubeLevel[0x07],
                  tubeLevel[0x08],
                  tubeLevel[0x09],
                  tubeLevel[0x0A],
                  tubeLevel[0x0B],
                  tubeLevel[0x0C],
                  tubeLevel[0x0D],
                  tubeLevel[0x0E],
                  tubeLevel[0x0F]);

    float tubeTotal = mdbCoinChanger::getTubeInventory();
    Serial.printf("\t[COIN] Change available %.2f Euro\r\n", tubeTotal);
#endif
}

void mdbCoinChanger::responsePoll(uint8_t *received, uint8_t len)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Poll Response: %d \r\n", state);
#endif
    mdbDataRespCC_t frame;
    memcpy(frame.raw, received, len - 1);
    uint8_t activity = frame.CoinChangerPollResult.activity[0];
    if (activity == CC_MDB_ACK)
    {
        // Nothing to Report
        if (state == CC_STATE_ACTIVE && coinPayOut)
        {
            state = CC_STATE_ESCROW;
        }
        return;
    }
    if (len >= 1)
    {
        // Todo Repeat till end
        Serial.printf("\t[COIN] #Activities:%d\r\n", len);
    }
    if (activity & 0x80)
    {
        Serial.println("\t[COIN] Coins dispensed manually");
        uint16_t amount = (activity & 0x70) >> 4;
        uint8_t type = activity & 0x0F;
        uint8_t remaining = (frame.CoinChangerPollResult.activity[1] & 0x70) >> 4;
        Serial.printf("\tDispend: %d coins of value %d - %d remaining\r\n", amount, getCoinChannelValue(type), remaining);
    }
    else if (activity & 0x20)
    {
        uint8_t amount = (activity & 0x1F);
        if (fakeCoins != amount)
        {
            fakeCoins = amount;
            Serial.printf("\t[COIN] #Slugs %d\r\n", amount);
        }
    }
    else if (activity & 0x40)
    {
        Serial.println("\t[COIN] Coins accepted");
        uint8_t type = activity & 0x0F;
        uint8_t remaining = (frame.CoinChangerPollResult.activity[1] & 0x70) >> 4;
        Serial.printf("\tAccepted: %d coin, %d remaining\r\n", getCoinChannelValue(type), remaining);
        coinFunds += getCoinChannelValue(type);
        Serial.print("\tCoins Deposited to ");
        switch ((activity & 0x30) >> 4)
        {
        case 0x00:
            Serial.println("CASH BOX");
            break;
        case 0x01:
            Serial.println("TUBES");
            break;
        case 0x10:
            Serial.println("NOT USED");
            break;
        case 0x11:
            Serial.println("REJECT");
            break;
        default:
            Serial.println("Unknown");
            break;
        }
    }
    else
    {
        switch (activity)
        {
        case 0b00000001:
        {
            Serial.println("\tEscrow request");
            coinPayOut = coinFunds;
            coinFunds = 0;
            state = CC_STATE_ESCROW;
            break;
        }
        case 0b00000010:
        {
            Serial.println("\tChanger Payout Busy");
            break;
        }
        case 0b00000011:
        {
            Serial.println("\tNo Credit");
            break;
        }
        case 0b00000100:
        {
            Serial.println("\tDefective Tube Sensor");
            break;
        }
        case 0b00000101:
        {
            Serial.println("\tDouble Arrival");
            break;
        }
        case 0b00000110:
        {
            Serial.println("\tAcceptor Unplugged");
            break;
        }
        case 0b00000111:
        {
            Serial.println("\tTube Jam");
            break;
        }
        case 0b00001000:
        {
            Serial.println("\tROM checksum error");
            break;
        }
        case 0b00001001:
        {
            Serial.println("\tCoin Routing Error");
            break;
        }
        case 0b00001010:
        {
            Serial.println("\tChanger Busy");
            break;
        }
        case 0b00001011:
        {
            Serial.println("\tChanger was Reset");
            // state = CC_STATE_MISSING;
            // Todo: Should Reset that thing
            break;
        }
        case 0b00001100:
        {
            Serial.println("\tCoin Jam");
            break;
        }
        case 0b00001101:
        {
            Serial.println("\tPossible credited Coin Removal");
            break;
        }
        default:
            Serial.print(activity, BIN);
            Serial.println("-Data");
        }
    }
}

void mdbCoinChanger::responseSetup(uint8_t *received, uint8_t len)
{
#if CC_DEBG
    Serial.printf("\t[COIN] Setup Response: %d \r\n", state);
#endif
    mdbDataRespCC_t frame;
    memcpy(frame.raw, received, len - 1);
    scaleFactor = frame.CoinChangerSetup.scaleFactor;
    featureLevel = frame.CoinChangerSetup.CoinChangerFeatureLvl;
    decimals = frame.CoinChangerSetup.decimalPlaces;
    if (featureLevel > 2)
        state = CC_STATE_FEATURE_ID;
    else
        state = CC_STATE_TUBE_STATUS;
    Serial.printf("\t[COIN] Scale %01X; decimal places %01X; feature level %01X  \r\n", scaleFactor, decimals, featureLevel);
    Serial.printf("\t[COIN] Coin 00-15 slot: \t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X \r\n",

                  frame.CoinChangerSetup.CoinB00RouteToTube,
                  frame.CoinChangerSetup.CoinB01RouteToTube,
                  frame.CoinChangerSetup.CoinB02RouteToTube,
                  frame.CoinChangerSetup.CoinB03RouteToTube,
                  frame.CoinChangerSetup.CoinB04RouteToTube,
                  frame.CoinChangerSetup.CoinB05RouteToTube,
                  frame.CoinChangerSetup.CoinB06RouteToTube,
                  frame.CoinChangerSetup.CoinB07RouteToTube,
                  frame.CoinChangerSetup.CoinB08RouteToTube,
                  frame.CoinChangerSetup.CoinB09RouteToTube,
                  frame.CoinChangerSetup.CoinB10RouteToTube,
                  frame.CoinChangerSetup.CoinB11RouteToTube,
                  frame.CoinChangerSetup.CoinB12RouteToTube,
                  frame.CoinChangerSetup.CoinB13RouteToTube,
                  frame.CoinChangerSetup.CoinB14RouteToTube,
                  frame.CoinChangerSetup.CoinB15RouteToTube);
    Serial.printf("\t[COIN] Coin 00-15 Value: \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",
                  (frame.CoinChangerSetup.coinValueScaledB00 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB01 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB02 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB03 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB04 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB05 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB06 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB07 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB08 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB09 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB10 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB11 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB12 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB13 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB14 * scaleFactor),
                  (frame.CoinChangerSetup.coinValueScaledB15 * scaleFactor));
    memcpy(coinChannelValue, &(frame.raw[7]), 16);
}

uint16_t mdbCoinChanger::getTubeInventory()
{
    float tubeTotal = 0;
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x00) * tubeLevel[0x00];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x01) * tubeLevel[0x01];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x02) * tubeLevel[0x02];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x03) * tubeLevel[0x03];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x04) * tubeLevel[0x04];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x05) * tubeLevel[0x05];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x06) * tubeLevel[0x06];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x07) * tubeLevel[0x07];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x08) * tubeLevel[0x08];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x09) * tubeLevel[0x09];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0A) * tubeLevel[0x0A];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0B) * tubeLevel[0x0B];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0C) * tubeLevel[0x0C];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0D) * tubeLevel[0x0D];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0E) * tubeLevel[0x0E];
    tubeTotal += mdbCoinChanger::getCoinChannelValue(0x0F) * tubeLevel[0x0F];
    tubeTotal /= 100;
    return tubeTotal;
}
