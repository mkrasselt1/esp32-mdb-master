#include "billValidator.h"
uint8_t mdbBillValidator::state = BV_STATE_MISSING;
uint8_t mdbBillValidator::featureLevel = 0;
uint8_t mdbBillValidator::decimals = 0;
uint16_t mdbBillValidator::capacity = 0;
uint8_t mdbBillValidator::billChannelValue[16] = {0};
uint8_t mdbBillValidator::fakeCoins = 0;
uint8_t mdbBillValidator::escrowCapable = 0;
uint16_t mdbBillValidator::billFunds = 0;
uint16_t mdbBillValidator::billPayOut = 0;
uint16_t mdbBillValidator::billPayOutPrevious = 0;
uint16_t mdbBillValidator::scaleFactor = 0;

unsigned long mdbBillValidator::nextPoll = 0;
long mdbBillValidator::timeout = 0;
long mdbBillValidator::nextTubePoll = 20000;

#define BV_DEBG true

void mdbBillValidator::init()
{
    mdbBillValidator::state = BV_STATE_MISSING;
    mdbBillValidator::nextPoll = millis() + 1000;
    mdbBillValidator::timeout = millis() + 500;
}

uint8_t mdbBillValidator::reset(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Reset 0x30\r\n");
#endif
    // Send Reset Message
    toSend[0] = BV_CMD_RESET;
    return 1;
}

uint8_t mdbBillValidator::setup(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Setup 0x31\r\n");
#endif
    // Send Setup Message
    toSend[0] = BV_CMD_SETUP_CFG;
    return 1;
}

uint8_t mdbBillValidator::billEnable(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Bill Enable 0x34\r\n");
#endif
    // Todo: select coin channels to be accepted
    toSend[0] = BV_CMD_BILL_TYPE;
    toSend[1] = 0b11111111; // enable bill acceptance b15-b0 high to low value
    toSend[2] = 0b11111111;
    toSend[3] = 0b11111111; // enable bill escrow b15-b0 high to low value
    toSend[4] = 0b11111111;
    return 5;
}

uint8_t mdbBillValidator::stacker(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Stacker Status 0x36\r\n");
#endif
    toSend[0] = BV_CMD_STACKER;
    return 1;
}

uint8_t mdbBillValidator::poll(uint16_t *toSend)
{
#if BV_DEBG
    // Serial.printf("\t[BILL] Poll 0x33\r\n");
#endif
    toSend[0] = BV_CMD_POLL;
    return 1;
}

uint16_t mdbBillValidator::getFunds()
{
    return billFunds;
}

void mdbBillValidator::setFunds(uint16_t newFunds)
{
    billFunds = newFunds;
}

void mdbBillValidator::setPayOut(uint16_t payout)
{
    billPayOut = payout;
    state = BV_STATE_ESCROW;
}

bool mdbBillValidator::updateFunds(uint32_t *fundStore)
{
    if (billFunds != *fundStore)
    {
        *fundStore = getFunds();
        return true;
    }
    return false;
}

bool mdbBillValidator::getPayOutChanged()
{
    if (billPayOut != billPayOutPrevious)
    {
        billPayOutPrevious = billPayOut;
        return true;
    }
    return false;
}

uint8_t mdbBillValidator::featureId(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Feature ID 0x0F00\r\n");
#endif
    // Send feature id message
    toSend[0] = BV_CMD_EXPANS;
    toSend[1] = BV_CMD_EXPANS_ID;
    return 2;
}

uint8_t mdbBillValidator::featureEnable(uint16_t *toSend, bool AlternatePayout, bool ManualInfill, bool Diagnose, bool FTL)
{
#if BV_DEBG
    Serial.printf("\t[BILL] FeatureEnable 0x0F01\r\n");
#endif
    // Send feature enable Message
    toSend[0] = BV_CMD_EXPANS;
    toSend[1] = BV_CMD_EXPANS_FEAT_E;
    toSend[2] = 0x00;
    toSend[3] = 0x00 | (FTL << 3) | (Diagnose << 2) | (ManualInfill << 1) | AlternatePayout;
    return 4;
}

uint8_t mdbBillValidator::escrow(uint16_t *toSend)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Escrow ID 0x35\r\n");
#endif
    if (billPayOut) {
    //     for (int chanel = 0x0F; chanel >= 0; chanel--)
    //     {
    //         int channelValue = mdbBillValidator::getBillChannelValue(chanel);
    //         int coinTubeAmount = stackLevel[chanel] - 2;
    //         if (channelValue && billPayOut >= channelValue && coinTubeAmount > 2)
    //         {
    //             int amount = billPayOut / channelValue;
    //             amount = min(coinTubeAmount, amount); // limit coin amount to tube content
    //             amount = min(amount, 10);             // limit payout coin amount
    //             stackLevel[chanel] -= amount;
    //             billPayOut -= amount * channelValue;
    //             Serial.printf("\t[BILL] Payout: %d Cent x %d Times\r\n", channelValue, amount);
    //             Serial.printf("\t[BILL] Remaining Payout: %d\r\n", billPayOut);
    //             // state = BV_STATE_ACTIVE;
    //             toSend[0] = BV_CMD_DISPENSE;
    //             toSend[1] = (amount << 4) | chanel;
    //             return 2;
    //         }
    //     }
    //     // no change coin left available
    //     state = BV_STATE_ACTIVE;
        billPayOut = 0;
        toSend[0] = BV_CMD_ESCROW;
        toSend[1] = BV_CMD_ESCROW_STACK;
        state = BV_STATE_ACTIVE;
        //todo start timeout 
        return 2;
    }
    else
    {
        state = BV_STATE_ACTIVE;
    }
    toSend[0] = BV_MDB_ACK;
    return poll(toSend);
}

uint16_t mdbBillValidator::getBillChannelValue(uint8_t chanel)
{
    if (chanel > (sizeof(billChannelValue) / sizeof(uint8_t)))
    {
        return 0;
    }
    // // Todo: maximum accepted Value inklusive Token)
    // if ((scaleFactor * billChannelValue[chanel]) > 200)
    // {
    //     return 0;
    // }
    return (uint16_t)scaleFactor * billChannelValue[chanel];
}

uint8_t mdbBillValidator::loop(uint16_t *toSend)
{
    mdbBillValidator::nextPoll = millis() + 100;
    mdbBillValidator::timeout = millis() + 50;
    // if(mdbBillValidator::timeout){
    //     state = BV_STATE_MISSING;
    //     nextPoll += 10000;
    // }

    // if (nextTubePoll <= millis() && state != BV_STATE_ESCROW && state != BV_STATE_MISSING)
    // {
    //     state = BV_STATE_TUBE_STATUS;
    //     nextTubePoll += 2000;
    // }

    if (billFunds)
    {
        billPayOut = billFunds;
        billFunds = 0;
        state = BV_STATE_ESCROW;
    }

#if BV_DEBG
    // Serial.printf("\t[BILL] Loop\r\n");
#endif

    switch (state)
    {
    case BV_STATE_MISSING:
        state = BV_STATE_RESET;
        return mdbBillValidator::reset(toSend);
    case BV_STATE_RESET:
        return mdbBillValidator::poll(toSend);
    case BV_STATE_SETUP:
        return mdbBillValidator::setup(toSend);
    case BV_STATE_FEATURE_ID:
        return mdbBillValidator::featureId(toSend);
    case BV_STATE_FEATURE_ENABLE:
        return mdbBillValidator::featureEnable(toSend, 0, 0, 0, 0); // 1, 1, 1, 1);
    case BV_STATE_TUBE_STATUS:
        return mdbBillValidator::stacker(toSend);
    case BV_STATE_BILL_ENABLE:
        state = BV_STATE_ACTIVE;
        return mdbBillValidator::billEnable(toSend);
    case BV_STATE_ESCROW:
        return mdbBillValidator::escrow(toSend);
    case BV_STATE_ACTIVE:
        return mdbBillValidator::poll(toSend);
    default:
        return mdbBillValidator::reset(toSend);
    }
    return 0;
}

void mdbBillValidator::response(uint8_t *received, uint8_t len)
{
    // #if BV_DEBG
    //     Serial.printf("\t[BILL] State: %d \r\n", state);
    // #endif
    mdbBillValidator::timeout = 0;
    switch (state)
    {
    case BV_STATE_MISSING:
    {
        if (received[0] == BV_MDB_ACK)
        {
            state = BV_STATE_SETUP;
            nextPoll = millis() + 750; // longer time for startup
        }
        else
        {
            state = BV_STATE_MISSING;
        }
        break;
    }
    case BV_STATE_RESET:
    {
        if (received[0] == BV_RESP_JUST_RESET)
        {
            state = BV_STATE_SETUP;
        }
        else
        {
            state = BV_STATE_MISSING;
        }
        break;
    }
    case BV_STATE_SETUP:
    {
        mdbBillValidator::responseSetup(received, len);
    }
    break;
    case BV_STATE_FEATURE_ID:
    {
        // state = BV_STATE_FEATURE_ENABLE; //todo find bug
        state = BV_STATE_BILL_ENABLE;
        break;
    }
    case BV_STATE_FEATURE_ENABLE:
    {
        // state = BV_STATE_TUBE_STATUS; von oben
        state = BV_STATE_BILL_ENABLE;
        break;
    }
    case BV_STATE_ESCROW:
    case BV_STATE_ACTIVE:
    {
        mdbBillValidator::responsePoll(received, len);
        break;
    }
    case BV_STATE_TUBE_STATUS:
    {
        mdbBillValidator::responseStacker(received, len);
        state = BV_STATE_BILL_ENABLE;
        break;
    }
    default:
        Serial.write((char *)received, len);
        Serial.println();
    }
}

void mdbBillValidator::responseStacker(uint8_t *received, uint8_t len)
{
#if BV_DEBG
    Serial.println("\t[BILL] stack status response:");
#endif
    mdbDataRespBV_t frame;
    memcpy(frame.raw, received, len - 1);
#if BV_DEBG
    Serial.printf("\t[BILL] Stack Full %01X, %hu \r\n", frame.billStackerStatus.billStackerFull & 0x80 >> 7, frame.billStackerStatus.billStackerFull & 0x7F);
#endif
}

void mdbBillValidator::responsePoll(uint8_t *received, uint8_t len)
{
    // #if BV_DEBG
    //     Serial.printf("\t[BILL] Poll Response(State: %d): \r\n", state);
    // #endif
    mdbDataRespBV_t frame;
    memcpy(frame.raw, received, len - 1);
    uint8_t activity = frame.billValidatorPollResult.activity[0];
    // if(len > 1 ) Serial.printf("\t[BILL] #Activities:%d\r\n", len);
    
    for (uint8_t x = 0; x < len; x++)
    {
        activity = frame.billValidatorPollResult.activity[x];
        if (activity == BV_MDB_ACK)
        {
            // Nothing to Report
            if (state == BV_STATE_ACTIVE && billPayOut)
            {
                state = BV_STATE_ESCROW;
            }
            return;
        } else 
        if (activity & 0x80)
        {
            Serial.println("\t[BILL] bill accepted");
            uint8_t type = activity & 0x0F;
            uint8_t routing = (frame.billValidatorPollResult.activity[x] & 0x70) >> 4;
            Serial.printf("\taccepted: %d bill, ", getBillChannelValue(type));
            billFunds += getBillChannelValue(type);
            Serial.print(" bill deposited to ");

            switch (routing)
            {
            case 0b000:
                Serial.println("BILL STACKED");
                break;
            case 0b001:
                Serial.println("ESCROW POSITION");
                break;
            case 0b010:
                Serial.println("BILL RETURNED");
                break;
            case 0b011:
                Serial.println("BILL TO RECYCLER");
                break;
            case 0b100:
                Serial.println("DISABLED BILL REJECTED");
                break;
            case 0b101:
                Serial.println("BILL TO RECYCLER – MANUAL FILL");
                break;
            case 0b110:
                Serial.println("MANUAL DISPENSE");
                break;
            case 0b111:
                Serial.println("TRANSFERRED FROM RECYCLER TO CASHBOX");
                break;
            default:
                Serial.println("Unknown");
                break;
            }
        } 
        else {
            switch (activity)
            {
            case 0b00000001:
            {
                Serial.println("\tmotor defect");
                // billPayOut = billFunds;
                // billFunds = 0;
                // state = BV_STATE_ESCROW;
                break;
            }
            case 0b00000010:
                Serial.println("\tsensor problem");
                break;
            case 0b00000011:
                Serial.println("\tvalidator busy");
                break;
            case 0b00000100:
                Serial.println("\trom checksum error");
                break;
            case 0b00000101:
                Serial.println("\tvalidator jammed");
                break;
            case 0b00000110:
                Serial.println("\tvalidator was reset");
                state = BV_STATE_RESET;
                break;
            case 0b00000111:
                Serial.println("\tbill removed (forcefully?)");
                break;
            case 0b00001000:
                Serial.println("\tcashbox out of position");
                break;
            case 0b00001001:
                Serial.println("\tvalidator disabled");
                break;
            case 0b00001010:
                Serial.println("\tinvalid escrow request");
                break;
            case 0b00001011:
                Serial.println("\tbill rejected");
                break;
            case 0b00001100:
                Serial.println("\ttheft attempt");
                break;
            case 0b00100001:
                Serial.println("escrow request");
                state = BV_STATE_ESCROW; //??correct
                break;
            case 0b00100010:
                Serial.println("dispenser payout busy");
                break;
            case 0b00100011:
                Serial.println("dispenser busy");
                break;
            case 0b00100100:
                Serial.println("defective dispense sensor");
                break;
            case 0b00100110:
                Serial.println("dispenser start error");
                break;
            case 0b00100111:
                Serial.println("dispenser payout jam");
                break;
            case 0b00101000:
                Serial.println("dispenser rom checksum error");
                break;
            case 0b00101001:
                Serial.println("dispenser disabled");
                break;
            case 0b00101010:
                Serial.println("bill waiting");
                break;
            case 0b00101111:
                Serial.println("fill requested");
                // todo: request dispenser status
                break;
            default:
                Serial.print(activity, BIN);
                Serial.println("-Data");
            }
            //(010xxxxx) = Number of attempts to input a bill while validator is disabled
        }
    }
}

void mdbBillValidator::responseSetup(uint8_t *received, uint8_t len)
{
#if BV_DEBG
    Serial.printf("\t[BILL] Setup Response: %d \r\n", state);
#endif
    mdbDataRespBV_t frame;
    memcpy(frame.raw, received, len - 1);
    scaleFactor = frame.billValidatorSetup.scaleFactorHigh << 8 | frame.billValidatorSetup.scaleFactorLow;
    featureLevel = frame.billValidatorSetup.featureLvl;
    decimals = frame.billValidatorSetup.decimalPlaces;
    capacity = frame.billValidatorSetup.stackCapacityHigh << 8 | frame.billValidatorSetup.stackCapacityLow;
    escrowCapable = frame.billValidatorSetup.escrowCapable;
    if (featureLevel > 2)
        state = BV_STATE_FEATURE_ID;
    else
        state = BV_STATE_TUBE_STATUS;
#if BV_DEBG
    Serial.printf("\t[BILL] Scale %01X; decimal places %01X; capacity %01X; escrow %01X, feature level %01X  \r\n", scaleFactor, decimals, capacity, escrowCapable, featureLevel);
    Serial.printf("\t[BILL] Bill 00-15 security: \t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X\t%01X \r\n",

                  frame.billValidatorSetup.billSecurityLevel00,
                  frame.billValidatorSetup.billSecurityLevel01,
                  frame.billValidatorSetup.billSecurityLevel02,
                  frame.billValidatorSetup.billSecurityLevel03,
                  frame.billValidatorSetup.billSecurityLevel04,
                  frame.billValidatorSetup.billSecurityLevel05,
                  frame.billValidatorSetup.billSecurityLevel06,
                  frame.billValidatorSetup.billSecurityLevel07,
                  frame.billValidatorSetup.billSecurityLevel08,
                  frame.billValidatorSetup.billSecurityLevel09,
                  frame.billValidatorSetup.billSecurityLevel10,
                  frame.billValidatorSetup.billSecurityLevel11,
                  frame.billValidatorSetup.billSecurityLevel12,
                  frame.billValidatorSetup.billSecurityLevel13,
                  frame.billValidatorSetup.billSecurityLevel14,
                  frame.billValidatorSetup.billSecurityLevel15);
    Serial.printf("\t[BILL] Type 00-15 Value: \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",
                  (frame.billValidatorSetup.billValue00 * scaleFactor),
                  (frame.billValidatorSetup.billValue01 * scaleFactor),
                  (frame.billValidatorSetup.billValue02 * scaleFactor),
                  (frame.billValidatorSetup.billValue03 * scaleFactor),
                  (frame.billValidatorSetup.billValue04 * scaleFactor),
                  (frame.billValidatorSetup.billValue05 * scaleFactor),
                  (frame.billValidatorSetup.billValue06 * scaleFactor),
                  (frame.billValidatorSetup.billValue07 * scaleFactor),
                  (frame.billValidatorSetup.billValue08 * scaleFactor),
                  (frame.billValidatorSetup.billValue09 * scaleFactor),
                  (frame.billValidatorSetup.billValue10 * scaleFactor),
                  (frame.billValidatorSetup.billValue11 * scaleFactor),
                  (frame.billValidatorSetup.billValue12 * scaleFactor),
                  (frame.billValidatorSetup.billValue13 * scaleFactor),
                  (frame.billValidatorSetup.billValue14 * scaleFactor),
                  (frame.billValidatorSetup.billValue15 * scaleFactor));
#endif
    memcpy(billChannelValue, &(frame.raw[11]), 16);
}
