#include "mdbProduct.h"

#define EC16(w) ((w>>8)|((w<<8)&0xFF00))

mdbProduct::mdbProduct(int ProducNumber, const char * Name, mdbCurrency Price)
:_name(Name), _number(ProducNumber), _price(Price)
{
}

mdbProduct::mdbProduct():
_name("empty"),
_number(0),
_price(0.0f, 1, 2)
{

}

uint16_t mdbProduct::getProduct16BitMDB()
{
    uint16_t outval = _number;
    if(_number < 0){
        outval = 0;
    }
    if(_number > UINT16_MAX){
        outval = 0xFFFF;
    }
    return EC16(outval);
}

int mdbProduct::getNumber()
{
    return _number;
}

const char* mdbProduct::getName()
{
    return _name.c_str();
}

mdbCurrency* mdbProduct::getPrice()
{
    return &_price;
}

void mdbProduct::setPrice(float price)
{
    _price.setValue(price);
}
