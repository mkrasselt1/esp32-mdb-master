#ifndef I_PAYMENT_H
#define I_PAYMENT_H

#include "mdbDevice.h"
#include "mdbCurrency.h"
#include "mdbProduct.h"
class IPaymentDevice;

namespace PaymentActions{
    enum Action{
        NO_ACTION,
        REQUEST_PRODUCT,
        PRODUCT_DISPENSE_SUCCESS,
        PRODUCT_DISPENSE_FAILURE,
        REQUEST_REVALUE
    };
}

class IPayment
{
    public:
        virtual void fundsChanged(IPaymentDevice * dev){}
        virtual bool startSession(IPaymentDevice * dev){return false;}
        virtual void endSession(IPaymentDevice * dev){}
        virtual void vendAccepted(IPaymentDevice * dev, mdbCurrency c){}
        virtual void vendDenied(IPaymentDevice * dev){}
        virtual void revalueAccepted(IPaymentDevice * dev){}
        virtual void revalueDenied(IPaymentDevice * dev){}
        virtual void requestText(IPaymentDevice * dev, char* msg, uint32_t duration){}
        virtual void payoutEnd(IPaymentDevice * dev, bool success){}
        virtual mdbCurrency getPendingRevalueAmount(IPaymentDevice * dev){return mdbCurrency(0.0f, 1, 2);}
        virtual PaymentActions::Action update(IPaymentDevice * dev){return PaymentActions::NO_ACTION;}
        virtual PaymentActions::Action updateSession(IPaymentDevice * dev){return PaymentActions::NO_ACTION;}
        virtual PaymentActions::Action updateDispense(IPaymentDevice * dev){return PaymentActions::NO_ACTION;}
        virtual mdbProduct* getCurrentProduct(IPaymentDevice * dev){return nullptr;}
};

class IPaymentDevice
{
    public:
        virtual void registerPaymentInterface(IPayment * payment){}
        virtual bool canRevalue(){return false;}
        virtual mdbCurrency getRevalueMax(){return mdbCurrency(0.0f, 1, 2);}
        virtual bool reportCashsale(){return false;}
        virtual bool pendingEscrowRequest(){return false;}
        virtual const char* getDeviceName(){return "Unknown Device";}
        virtual mdbCurrency getCurrentFunds(){return mdbCurrency(0.0f, 1, 2);}
        virtual void clearCurrentFunds(){}

    protected:
        IPayment * _paymentInterface = nullptr;
        uint8_t _currentVendID = 0;
};


#endif