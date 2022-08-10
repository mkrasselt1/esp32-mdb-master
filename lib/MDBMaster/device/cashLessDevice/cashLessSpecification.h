/*

*/
#ifndef CASHLESS_SPEC_H
#define CASHLESS_SPEC_H

#include "../../mdbStructure.h"

// Cashless  Command set
#define CL_CMD_RESET 0x10
#define CL_CMD_SETUP 0x11
#define CL_CMD_SETUP_DATA 0x00
#define CL_CMD_SETUP_PRICES 0x01
#define CL_CMD_POLL 0x12
#define CL_CMD_VEND 0x13
#define CL_CMD_VEND_REQUEST 0x00
#define CL_CMD_VEND_CANCEL 0x01
#define CL_CMD_VEND_SUCCESS 0x02
#define CL_CMD_VEND_FAILURE 0x03
#define CL_CMD_VEND_SESSION_COM 0x04
#define CL_CMD_VEND_CASH_SALE 0x05
#define CL_CMD_VEND_NEG_VEND 0x06
#define CL_CMD_READER 0x14
#define CL_CMD_READER_DISABLE 0x00
#define CL_CMD_READER_ENABLE 0x01
#define CL_CMD_READER_CANCEL 0x02
#define CL_CMD_READER_DATA_E_RE 0x03

#define CL_CMD_REVALUE 0x15
#define CL_CMD_REVALUE_REQ 0x00
#define CL_CMD_REVALUE_LIMIT 0x01

#define CL_CMD_EXPANS 0x17
#define CL_CMD_EXPANS_ID 0x00
#define CL_CMD_EXPANS_RE_USERF 0x01
#define CL_CMD_EXPANS_WR_USERF 0x02
#define CL_CMD_EXPANS_WR_TIME 0x03
#define CL_CMD_EXPANS_FEAT_EN 0x04
#define CL_CMD_EXPANS_REQ2REC 0xFA
#define CL_CMD_EXPANS_RETRY_DEN 0xFB
#define CL_CMD_EXPANS_SEND_BLK 0xFC
#define CL_CMD_EXPANS_OK2SEND 0xFD
#define CL_CMD_EXPANS_REQ2SEND 0xFE

#define CL_CMD_EXPANS_DIAG 0xFF

#define CL_CMD_NOT_IMPLEMENTED 0xFF

// Coin Changer Response set
#define CL_RESP_JUST_RESET 0x00

#define CL_MDB_ACK 0x00
#define CL_MDB_NACK 0xFF

const mdbCommandDescriptor cashLessCommands[]{
    //---CMD Name-------------------CMD---SubCMD--------Lvl-----Len--Action
    {"Reset", 0x00, NO_SUBCMD, 1, 1, CL_CMD_RESET},
    // {"Setup",                       0x01, NO_SUBCMD,    2,      23, CL_CMD_SETUP_CFG},
    // {"Tube Status",                 0x02, NO_SUBCMD,    2,      18, CL_CMD_TUBE_STATUS},
    // {"Poll",                        0x03, NO_SUBCMD,    2,      16, CL_CMD_POLL},
    // {"Coin Type",                   0x04, NO_SUBCMD,    2,      4,  CL_CMD_COIN_ACL},
    // {"Dispense",                    0x05, NO_SUBCMD,    2,      1,  CL_CMD_DISPENSE},
    // {"Expansion Id",                0x06, 0x00,         3,      1,  CL_CMD_EXPANS_ID},
    // {"Expansion Feat",              0x06, 0x01,         3,      1,  CL_CMD_EXPANS_FEAT_E},
    // {"Expansion Debug",             0x06, 0x02,         3,      1,  CL_CMD_EXPANS_DEBUG},
    {"EOL", 0x00, 0x00, 0, 0, CL_CMD_NOT_IMPLEMENTED}};

typedef union __attribute__((packed)) mdbDataRespCL_t
{
    uint8_t raw[37];

    struct __attribute__((packed))
    {
#define CL_RESP_READER_CONFIG 0x01
        uint8_t ConfigData;
        uint8_t featureLevel;
        uint8_t countryCodeHigh;
        uint8_t countryCodeLow;
        uint8_t scaleFactor;
        uint8_t decimalPlaces;
        uint8_t appMaxResponseTime;
        uint8_t Option7Unused : 1;
        uint8_t Option6Unused : 1;
        uint8_t Option5Unused : 1;
        uint8_t Option4Unused : 1;
        uint8_t Option3FundRestore : 1;
        uint8_t Option2MultivendCapable : 1;
        uint8_t Option1ReaderDisplay : 1;
        uint8_t Option0CashSale : 1;

    } CashLessSetup;

    // struct __attribute__((packed))
    // {
    //     #define CL_RESP_CONFIG_PRICES 0x01
    //     uint16_t MaxPrice;
    //     uint16_t MinPrice;
    // } CashLessMaxMinPrice;

    // struct __attribute__((packed))
    // {
    //     uint32_t MaxPrice;
    //     uint32_t MinPrice;
    //     uint16_t CurrencyCode;
    // } CashLessLvl3MaxMinPrice;

    struct __attribute__((packed))
    {
#define CL_RESP_DISP_REQ 0x02
        uint8_t DispRequ;
        uint8_t DisplayTime;
        char DisplayMessage[32];
    } CashLessDisplayRequest;

    struct __attribute__((packed))
    {
#define CL_RESP_BEGIN_SESSION 0x03
        uint8_t BeginSession;
        uint8_t fundsLB;
        uint8_t fundsHB;
    } CashLessBeginnSession;

    struct __attribute__((packed))
    {
#define CL_RESP_REVALUE_LIMIT 0x0F
        uint8_t RevalueLimit;
        uint8_t limitLB;
        uint8_t limitHB;
    } CashLessRevalueLimit;

    struct __attribute__((packed))
    {
#define CL_RESP_REVALUE_LIMIT 0x0F
        uint8_t RevalueLimit;
        uint8_t limitLB;
        uint8_t limitMLB;
        uint8_t limitMHB;
        uint8_t limitHB;
    } CashLessRevalueLimitExpCurrency;
    struct __attribute__((packed))
    {
#define CL_RESP_BEGIN_SESSION 0x03
        uint8_t BeginSession;
        uint16_t funds;
        uint8_t paymentMediaId[4];
        uint8_t paymentType;
        uint16_t paymentData;
    } CashLessLvl3BeginnSession;

    struct __attribute__((packed))
    {
#define CL_RESP_BEGIN_SESSION 0x03
        uint8_t BeginSession;
        uint16_t funds;
        uint8_t paymentMediaId[4];
        uint8_t paymentType;
        uint16_t paymentData;
        uint16_t userLanguage;
        uint16_t UserCurrency;
        uint8_t CardOptions;
    } CashLessLvl3BeginnSessionExpandedCurrency;

#define CL_RESP_SESSION_CANCEL 0x04

#define CL_RESP_VEND_APPROVED 0x05
    struct __attribute__((packed))
    {
        uint8_t VendApproved;
        uint16_t amount;
    } CashLessVendApproved;

    struct __attribute__((packed))
    {
        uint8_t VendApproved;
        uint32_t amount;
    } CashLessLvl3VendApprovedExpandedCurrency;

#define CL_RESP_VEND_DENIED 0x06

#define CL_RESP_END_SESSION 0x07

#define CL_RESP_CANCELED 0x08

#define CL_RESP_EXPANSION_ID 0x09
    struct __attribute__((packed))
    {
        uint8_t ExpansionId;
        char manufacturer[3];
        char serialNumber[12];
        char model[12];
        uint8_t softwareVersionHighByte;
        uint8_t softwareVersionLowByte;
    } CashLessExpansionID;

#define CL_RESP_REVALUE_APPROVED 0x0D
#define CL_RESP_REVALUE_DENIED 0x0E

    struct __attribute__((packed))
    {
        uint8_t ExpansionId;
        char manufacturer[4];
        char serialNumber[12];
        char model[12];
        uint8_t softwareVersionHighByte;
        uint8_t softwareVersionLowByte;
        uint8_t OptFeature31 : 1;
        uint8_t OptFeature30 : 1;
        uint8_t OptFeature29 : 1;
        uint8_t OptFeature28 : 1;
        uint8_t OptFeature27 : 1;
        uint8_t OptFeature26 : 1;
        uint8_t OptFeature25 : 1;
        uint8_t OptFeature24 : 1;
        uint8_t OptFeature23 : 1;
        uint8_t OptFeature22 : 1;
        uint8_t OptFeature21 : 1;
        uint8_t OptFeature20 : 1;
        uint8_t OptFeature19 : 1;
        uint8_t OptFeature18 : 1;
        uint8_t OptFeature17 : 1;
        uint8_t OptFeature16 : 1;
        uint8_t OptFeature15 : 1;
        uint8_t OptFeature14 : 1;
        uint8_t OptFeature13 : 1;
        uint8_t OptFeature12 : 1;
        uint8_t OptFeature11 : 1;
        uint8_t OptFeature10 : 1;
        uint8_t OptFeature09 : 1;
        uint8_t OptFeature08 : 1;
        uint8_t OptFeature07 : 1;
        uint8_t OptFeature06 : 1;
        uint8_t OptFeature05AlwaysIdle : 1;
        uint8_t OptFeature04DataEntry : 1;
        uint8_t OptFeature03NegativeVend : 1;
        uint8_t OptFeature02MultiCurrency : 1;
        uint8_t OptFeature01FourByteNumbers : 1;
        uint8_t OptFeature00FileTransferLayer : 1;
    } CashLessLvl3ExpansionID;

#define CL_RESP_DATA_ENTRY_CANCEL 0x13

    struct __attribute__((packed))
    {
        uint8_t activity[16];
    } CashLessPollResult;
} CashLessDataResponse;
#endif