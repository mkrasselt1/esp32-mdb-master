#include "coinChanger.h"

void coinChanger::registerPaymentInterface(IPayment * payment)
{
    _paymentInterface = payment;
}

bool coinChanger::canRevalue()
{
    bool hasCoinsLeft = false;
    for (size_t i = 0; i < 16; i++)
    {
        if( (Tubes[i].active) && (!Tubes[i].isToken) ){
            if(Tubes[i].amount > 0)
            {
                hasCoinsLeft = true;
            }
        }
    }
    
    return hasCoinsLeft;
}

bool coinChanger::reportCashsale()
{
    return false;
}

bool coinChanger::pendingEscrowRequest()
{
    bool tmp = escrowPending;
    escrowPending = false;
    return tmp;
}

const char* coinChanger::getDeviceName()
{
    if(mdbAddress == MDB_CHANGER){
        return "Coin Changer #1";
    }else{
        return "Coin Changer invalid Address";
    }
    
}

mdbCurrency coinChanger::getCurrentFunds()
{
    return currentFunds;
}

mdbCurrency coinChanger::getRevalueMax()
{
    mdbCurrency tubeVal(0.0f, 1, 2);
    for (size_t i = 0; i < 16; i++)
    {
        if( (Tubes[i].active) && (!Tubes[i].isToken) ){
            for (size_t j = 0; j < Tubes[i].amount; j++)
            {
                tubeVal.add(Tubes[i].value);
            }                      
        }
    }
    return tubeVal;
}

void coinChanger::clearCurrentFunds()
{
    currentFunds.setValue(0.0);
}




