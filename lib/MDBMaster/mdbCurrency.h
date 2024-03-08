#ifndef MDB_CURRENCY_H
#define MDB_CURRENCY_H

#include <Arduino.h>

class mdbCurrency
{
    private:
        uint8_t _scaleFactor;
        uint8_t _decimalPlaces;
        float _value;

    public:
        mdbCurrency(uint16_t value, uint8_t scaleFactor, uint8_t decimalPlaces);
        mdbCurrency(float value, uint8_t scaleFactor, uint8_t decimalPlaces);
        mdbCurrency(const mdbCurrency &oldval);
        mdbCurrency();
        uint16_t getValue16BitMDB();
        uint32_t getValue32BitMDB();
        float getValue();
        void setValue(float value);
        void add(const mdbCurrency &rhs);
        void sub(const mdbCurrency &rhs);
        void setup(uint8_t scaleFactor, uint8_t decimalPlaces);
};


#endif