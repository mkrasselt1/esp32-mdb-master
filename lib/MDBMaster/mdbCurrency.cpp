#include "mdbCurrency.h"

#define EC16(w) ((w>>8)|((w<<8)&0xFF00))
#define EC32(dw) (((dw<<24)&0xFF000000)|((dw>>8)&0xFF00)|((dw<<8)&0xFF0000)|(dw>>24))

mdbCurrency::mdbCurrency(uint16_t value, uint8_t scaleFactor, uint8_t decimalPlaces)
{
    _value = EC16(value);
    _scaleFactor = scaleFactor;
    _decimalPlaces = decimalPlaces;

    _value *= _scaleFactor;
    for (size_t i = 0; i < _decimalPlaces; i++)
    {
        _value /= 10.0;
    }
}

mdbCurrency::mdbCurrency(float value, uint8_t scaleFactor, uint8_t decimalPlaces)
{
    _value = value;
    _scaleFactor = scaleFactor;
    _decimalPlaces = decimalPlaces;
}

mdbCurrency::mdbCurrency(const mdbCurrency &oldval)
{
    _scaleFactor = oldval._scaleFactor;
    _decimalPlaces = oldval._decimalPlaces;
    _value = oldval._value;
}

mdbCurrency::mdbCurrency()
{
    _value = 0;
    _scaleFactor = 1;
    _decimalPlaces = 2;
}



uint16_t mdbCurrency::getValue16BitMDB()
{
    uint16_t outval = 0;
    _value = roundf(_value * 100) / 100;
    float tmp = _value;

    tmp /= _scaleFactor;
    for (size_t i = 0; i < _decimalPlaces; i++)
    {
        tmp *= 10.0;
    }
    outval = tmp;

    if(tmp < 0){
        outval = 0;
    }
    if(tmp > UINT16_MAX){
        outval = 0xFFFF - 1;
    }
    return EC16(outval);
}

uint32_t mdbCurrency::getValue32BitMDB()
{
    uint32_t outval = 0;
    _value = roundf(_value * 100) / 100;
    float tmp = _value;
    
    tmp /= _scaleFactor;
    for (size_t i = 0; i < _decimalPlaces; i++)
    {
        tmp *= 10.0;
    }
    outval = tmp;

    if(tmp < 0){
        outval = 0;
    }
    if(tmp > UINT32_MAX){
        outval = 0xFFFF - 1;
    }
    return EC32(outval);
}

float mdbCurrency::getValue()
{
    _value = roundf(_value * 100) / 100;
    return _value;
}

void mdbCurrency::setValue(float value)
{
    _value = value;
    _value = roundf(_value * 100) / 100;
}

void mdbCurrency::add(const mdbCurrency &rhs)
{
    _value += rhs._value;
    _value = roundf(_value * 100) / 100;
}

void mdbCurrency::sub(const mdbCurrency &rhs)
{
    _value -= rhs._value;
    _value = roundf(_value * 100) / 100;
}

void mdbCurrency::setup(uint8_t scaleFactor, uint8_t decimalPlaces)
{
    _scaleFactor = scaleFactor;
    _decimalPlaces = decimalPlaces;
}