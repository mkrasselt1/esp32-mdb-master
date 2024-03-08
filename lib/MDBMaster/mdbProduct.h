#ifndef MDB_PRODUCT_H
#define MDB_PRODUCT_H

#include <Arduino.h>
#include "mdbCurrency.h"

class mdbProduct
{
    private:
        String _name;
        int _number;
        mdbCurrency _price;

    public:
        mdbProduct(int ProducNumber, const char * Name, mdbCurrency Price);
        mdbProduct();
        uint16_t getProduct16BitMDB();
        int getNumber();
        const char* getName();
        mdbCurrency* getPrice();
        void setPrice(float price);
};


#endif