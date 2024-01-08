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

uint32_t mdbMaster::billFunds = 0;
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
    mdbBillValidator::init();
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
    if (billFunds)
    {
        mdbBillValidator::setFunds(0);
        mdbBillValidator::setPayOut(1);
        return billFunds;
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
        case MDB::ACTIVE_DEVICE::BILL_VALIDATOR:
            {
                uint16_t toSend[33];
                uint16_t len = mdbBillValidator::loop(toSend);
                mdbMaster::timeout = mdbBillValidator::timeout;
                mdbDriver::sendPacket(toSend, len);
            }
            break;
        case MDB::ACTIVE_DEVICE::NONE:
        default:
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
            case MDB::ACTIVE_DEVICE::BILL_VALIDATOR:
            {
                dataReceived = false;
                mdbBillValidator::response(recFrame, recLen);
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
        case MDB::ACTIVE_DEVICE::BILL_VALIDATOR:
            // Serial.println("timeout for active device");
            mdbBillValidator::init();
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

    mdbBillValidator::updateFunds(&billFunds);
};

void mdbMaster::findNextPollDevice()
{
    if (mdbBillValidator::nextPoll <= millis())
    {
        mdbMaster::nextPoll = millis();
        activeDevice = MDB::ACTIVE_DEVICE::BILL_VALIDATOR;
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
    if (cleanScreen && (billFunds))
    {
        float euroCents = billFunds;
        // Serial.printf("---FUNDS %ld, %ld, %ld, %ld\r\n", cashLessFunds, coinFunds, billFunds, cashLessFunds + coinFunds + billFunds);
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
    default:
        // Serial.println("none");
        mdbCashLess::reqSessionComplete();
        break;
    }
    vendAuthDevice = MDB::ACTIVE_DEVICE::NONE;
}

void mdbMaster::start(){

};

void mdbMaster::setFund(uint16_t fund){};

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