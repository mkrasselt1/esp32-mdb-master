#include "universalSatelliteDevice.h"

void universalSatelliteDevice::registerPaymentInterface(IPayment * payment)
{
    _paymentInterface = payment;
}

bool universalSatelliteDevice::requestVend(uint8_t vendID, mdbProduct product, mdbCurrency price)
{
    return false;
}

bool universalSatelliteDevice::canRevalue()
{
    return false;
}

bool universalSatelliteDevice::reportCashsale()
{
    return false;
}

const char* universalSatelliteDevice::getDeviceName()
{
    if(mdbAddress == MDB_UNI_SAT_DEV_1){
        return "Universal Satellite Device #1";
    }else if(mdbAddress == MDB_UNI_SAT_DEV_2){
        return "Universal Satellite Device #2";
    }else if(mdbAddress == MDB_UNI_SAT_DEV_3){
        return "Universal Satellite Device #3";
    }else{
        return "Universal Satellite Device invalid Address";
    }
    
}


