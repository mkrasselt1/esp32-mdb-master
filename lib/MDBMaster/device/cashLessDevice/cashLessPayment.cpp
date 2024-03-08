#include "cashLess.h"

void mdbCashLess::registerPaymentInterface(IPayment * payment)
{
    _paymentInterface = payment;
}

bool mdbCashLess::canRevalue()
{
    return (properties.canRevalue && sessionCanRevalue);
}

bool mdbCashLess::reportCashsale()
{
    return false;
}

const char* mdbCashLess::getDeviceName()
{
    if(mdbAddress == MDB_CASHLESS_1){
        return "Cashless Device #1";
    }else if(mdbAddress == MDB_CASHLESS_2){
        return "Cashless Device #2";
    }else{
        return "Cashless Device invalid Address";
    }
    
}

mdbCurrency mdbCashLess::getCurrentFunds()
{
    return currentFunds;
}

void mdbCashLess::clearCurrentFunds()
{
    //do nothing
}


