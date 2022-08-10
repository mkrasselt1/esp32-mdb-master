#include "mdbMaster.h"

long mdbMaster::time = 0;
long mdbMaster::timeout = 0;
unsigned long mdbMaster::nextPoll = 0;
bool mdbMaster::dataReceived = 0;
MDB::ACTIVE_DEVICE mdbMaster::activeDevice = MDB::ACTIVE_DEVICE::NONE;
MDB::ACTIVE_DEVICE mdbMaster::vendAuthDevice = MDB::ACTIVE_DEVICE::NONE;
uint8_t mdbMaster::deviceResponse[40] = {'\0'};
uint8_t mdbMaster::deviceResponseLength = 0;
uint8_t mdbMaster::recFrame[40] = {'\0'};
uint8_t mdbMaster::recLen = 0;

uint32_t mdbMaster::coinFunds = 0;
uint32_t mdbMaster::cashLessFunds = 0;
MDB::MDBCommState mdbMaster::state = MDB::MDBCommState::IDLE;

char mdbMaster::displayText[32] = {'\0'};
bool mdbMaster::displayTextShown = true;
bool mdbMaster::cleanScreen = true;
long mdbMaster::displayTimeout = 0;
long mdbMaster::sessionTimeout = 0;
uint16_t mdbMaster::productToApprove = 0;
uint16_t mdbMaster::priceToApprove = 0;
uint8_t mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Expired;

void mdbMaster::init()
{
    mdbDriver::start();
    mdbCoinChanger::init();
    mdbCashLess::init();
    mdbMaster::state = MDB::MDBCommState::IDLE;
};

bool mdbMaster::isNewText()
{
    bool returnV = displayTextShown;
    displayTextShown = true;
    if (!returnV)
    {
        mdbMaster::cleanScreen = false;
    }
    return !returnV;
}

uint32_t mdbMaster::getFunds()
{
    if ((coinFunds || cashLessFunds) && (cashLessFunds != 0xFFFFFFFF))
    {
        return coinFunds + cashLessFunds;
    }
    else if (!coinFunds && cashLessFunds == 0xFFFFFFFF)
    {
        return cashLessFunds;
    }
    return 0;
}

void mdbMaster::approve(uint16_t product, uint16_t price)
{
    // starts a authorisation process for the devices
    mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Awaiting;
    mdbCashLess::setVendRequest(product, price);
    mdbMaster::productToApprove = product;
    mdbMaster::priceToApprove = price;
}

bool mdbMaster::expiredApproval()
{
    // check for non existing decision
    return mdbMaster::vendApproveResult == MDB_MASTER_VEND_APPROVE_RESULT_Expired;
}

bool mdbMaster::hasApproved(bool *result)
{
    // find out if decision was made
    switch (mdbMaster::vendApproveResult)
    {
    case MDB_MASTER_VEND_APPROVE_RESULT_Granted:
        mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Expired;
        char temp[17];
        sprintf(temp, "Produkt: %d", mdbMaster::productToApprove);
        strcpy(&displayText[16], "Kauf erfolgreich");
        displayTimeout = millis() + 15000;
        displayTextShown = false;
        *result = true;
        return true;
    case MDB_MASTER_VEND_APPROVE_RESULT_Deny:
        mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Expired;
        sprintf(displayText, "Produkt: %d", mdbMaster::productToApprove);
        sprintf(&displayText[16], "Preis: %.2f Euro", ((float)mdbMaster::priceToApprove) / 100);
        displayTimeout = millis() + 15000;
        displayTextShown = false;
        *result = false;
        return true;
    case MDB_MASTER_VEND_APPROVE_RESULT_Awaiting:
        return false;
    default:
        *result = false;
        return false;
    }
    return false;
}

void mdbMaster::payOut(uint16_t payoutAmount)
{
    mdbCoinChanger::setPayOut(payoutAmount);
}

void mdbMaster::pollAll()
{
    mdbMaster::updateScreen();
    switch (mdbMaster::state)
    {
    case MDB::MDBCommState::IDLE:
    {
        mdbMaster::findNextPollDevice();
    }
    break;
    case MDB::MDBCommState::POLL:
    {
        switch (mdbMaster::activeDevice)
        {
        case MDB::ACTIVE_DEVICE::COIN_CHANGER:
        {
            uint16_t toSend[33];
            uint16_t len = mdbCoinChanger::loop(toSend);
            mdbMaster::timeout = mdbCoinChanger::timeout;
            mdbDriver::sendPacket(toSend, len);
        }
        break;
        case MDB::ACTIVE_DEVICE::CASHLESS:
        {
            uint16_t toSend[33];
            uint16_t len = mdbCashLess::loop(toSend);
            mdbMaster::timeout = mdbCashLess::timeout;
            mdbDriver::sendPacket(toSend, len);
        }
        break;
        case MDB::ACTIVE_DEVICE::NONE:
            break;
        }
        mdbMaster::state = MDB::MDBCommState::WAIT;
    }
    break;
    case MDB::MDBCommState::WAIT:
    {
        if (dataReceived && timeout > 0)
        {
            switch (activeDevice)
            {
            case MDB::ACTIVE_DEVICE::CASHLESS:
            {
                dataReceived = false;
                mdbCashLess::response(recFrame, recLen);
                recLen = 0;
                mdbMaster::activeDevice = MDB::ACTIVE_DEVICE::NONE;
            }
            break;
            case MDB::ACTIVE_DEVICE::COIN_CHANGER:
            {
                dataReceived = false;
                mdbCoinChanger::response(recFrame, recLen);
                recLen = 0;
                mdbMaster::activeDevice = MDB::ACTIVE_DEVICE::NONE;
            }
            break;
            default:
            {
                Serial.println("received data discarded");
                dataReceived = false;
            }
            }
            // Serial.println("SW to IDLE");
            mdbMaster::state = MDB::MDBCommState::IDLE;
        }
        break;
    }
    default:
        // Serial.println("SW to IDLE");
        mdbMaster::state = MDB::MDBCommState::IDLE;
    }
    if (timeout > 0 && timeout <= millis() && activeDevice != MDB::ACTIVE_DEVICE::NONE)
    {
        switch (activeDevice)
        {
        case MDB::ACTIVE_DEVICE::CASHLESS:
            // Serial.println("timeout for active device");
            mdbCashLess::init();
            break;
        case MDB::ACTIVE_DEVICE::COIN_CHANGER:
            // Serial.println("timeout for active device");
            mdbCoinChanger::init();
            break;
        case MDB::ACTIVE_DEVICE::NONE:
            break;
        }
        // Serial.println("timeout for active device");
        mdbMaster::activeDevice = MDB::ACTIVE_DEVICE::NONE;
        mdbMaster::state = MDB::MDBCommState::IDLE;
    }

    if (mdbCashLess::displayRequestMessageAvailable())
    {
        memcpy(displayText, mdbCashLess::displayMessage, 32);
        displayTimeout = millis() + (mdbCashLess::displayTimeout * 100);
        displayTextShown = false;
    }

    if (coinFunds > 0) // coin funds available
    {
        // invalidate unrealistic funds
        if (coinFunds > 10000)
        { // Todo: Maximum 100€
            coinFunds = 0;
        }
        if (coinFunds && mdbCashLess::hasActiveSession() && mdbMaster::vendAuthDevice == MDB::ACTIVE_DEVICE::NONE)
        {
            mdbCashLess::setRevalue(coinFunds);
            mdbCoinChanger::setFunds(0); // ToDO: eval revalue result denied/accept
        }
    }

    if (mdbCashLess::hasActiveSession() && !sessionTimeout)
    {
        sessionTimeout = millis() + 25000;
    }
    if (mdbCashLess::hasActiveSession() && sessionTimeout<1000)
    {
        mdbCashLess::reqSessionComplete();
        sessionTimeout = 0;
    }
    mdbCoinChanger::updateFunds(&coinFunds);
    mdbCashLess::updateFunds(&cashLessFunds);
    // mdbCashLess::updateRevalue(&cashLessFunds);

    if (mdbMaster::vendApproveResult == MDB_MASTER_VEND_APPROVE_RESULT_Awaiting)
    {
        bool result = false;
        if (mdbCashLess::hasApproved(&result))
        {
            if (result)
            {
                mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Granted;
                mdbMaster::vendAuthDevice = MDB::ACTIVE_DEVICE::CASHLESS;
            }
            else
            {
                if (coinFunds >= priceToApprove)
                {
                    mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Granted;
                    mdbMaster::vendAuthDevice = MDB::ACTIVE_DEVICE::COIN_CHANGER;
                }
                else
                {
                    mdbMaster::vendApproveResult = MDB_MASTER_VEND_APPROVE_RESULT_Deny;
                    mdbMaster::vendAuthDevice = MDB::ACTIVE_DEVICE::NONE;
                }
            }
        }
    }
};

void mdbMaster::findNextPollDevice()
{
    if (mdbCashLess::nextPoll <= millis())
    {
        mdbMaster::nextPoll = millis();
        activeDevice = MDB::ACTIVE_DEVICE::CASHLESS;
        timeout = 0;
        mdbMaster::state = MDB::MDBCommState::POLL;
    }
    else if (mdbCoinChanger::nextPoll <= millis())
    {
        mdbMaster::nextPoll = millis();
        activeDevice = MDB::ACTIVE_DEVICE::COIN_CHANGER;
        timeout = 0;
        mdbMaster::state = MDB::MDBCommState::POLL;
    }
    else
    {
        mdbMaster::activeDevice = MDB::ACTIVE_DEVICE::NONE;
    }
}

void mdbMaster::updateScreen()
{
    // Display Timeout Elapsed and Screen contains something
    if (displayTimeout <= millis() && !cleanScreen)
    {
        strcpy(displayText, "                               ");
        displayTextShown = false;
        cleanScreen = true;
    }
    // if funds available - show them
    if (cleanScreen && (coinFunds || cashLessFunds))
    {
        float euroCents = cashLessFunds + coinFunds;
        // Serial.printf("---FUNDS %ld, %ld, %ld\r\n", cashLessFunds, coinFunds, cashLessFunds + coinFunds);
        euroCents /= 100;
        sprintf(displayText, "Guthaben:       %.2f Euro      ", euroCents);
        mdbMaster::displayTimeout = millis() + 500;
        displayTextShown = false;
    }
}

void mdbMaster::finishedProduct()
{
    // which device has auth the sale ?
    switch (vendAuthDevice)
    {
    case MDB::ACTIVE_DEVICE::CASHLESS:
        // Serial.println("cashless:");
        mdbCashLess::setVendSuccess();
        mdbCashLess::reqSessionComplete();
        break;
    case MDB::ACTIVE_DEVICE::COIN_CHANGER:
        // Serial.println("coins");
        if (coinFunds > 0 && priceToApprove >= 0 && coinFunds >= priceToApprove)
        {
            coinFunds -= priceToApprove;
            mdbCoinChanger::setFunds(coinFunds);
        }
        mdbCashLess::logCashSale(productToApprove, priceToApprove);
        break;
    default:
        // Serial.println("none");
        mdbCashLess::reqSessionComplete();
        break;
    }
    vendAuthDevice = MDB::ACTIVE_DEVICE::NONE;
}

void mdbMaster::start(){

};

void mdbMaster::setFund(uint16_t fund)
{
    if (fund >= 0)
    {
        cashLessFunds = fund;
    }
};

void mdbMaster::receive(uint16_t newByte)
{
    if (deviceResponseLength >= (sizeof(deviceResponse) / sizeof(uint8_t)))
    {
        deviceResponseLength = 0;
    }
    deviceResponse[deviceResponseLength] = newByte & 0xFF;
    if (newByte & 0x100)
    { // checks for checksum in last byte
        uint16_t tmp = 0, chkSum = 0;
        if (deviceResponseLength)
        {
            for (uint8_t i = 0; i < deviceResponseLength; i++)
            {
                tmp = deviceResponse[i];
                chkSum += tmp & 0xFF;
            }
        }

        chkSum &= 0xFF;
        chkSum |= 0x100;
        // checksum ok
        if (newByte == chkSum)
        {
            if (deviceResponseLength)
            { // send ack on longer messages
                delay(5);
                mdbDriver::send(MDB_ACK);
            }

            memcpy(recFrame, deviceResponse, sizeof(recFrame[0]) * deviceResponseLength + 1);
            recLen = deviceResponseLength + 1;
            dataReceived = 1;
        }
        else
        {
            // checksum failed
            if (deviceResponseLength)
            { // send nack on longer messages
                delay(5);
                mdbDriver::send(MDB_NACK);
            }
        }

        deviceResponseLength = 0;
    }
    else
    {
        deviceResponseLength++;
    }
}