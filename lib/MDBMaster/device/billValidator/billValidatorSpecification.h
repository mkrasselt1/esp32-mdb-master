#ifndef BILLVALIDATOR_SPEC_H
#define BILLVALIDATOR_SPEC_H

#include "../../mdbStructure.h"

// Bill Validator MDB States

#define BV_STATE_MISSING 0
#define BV_STATE_RESET 1
#define BV_STATE_SETUP 2
#define BV_STATE_FEATURE_ID 3
#define BV_STATE_FEATURE_ENABLE 4
#define BV_STATE_TUBE_STATUS 5
#define BV_STATE_BILL_ENABLE 6
#define BV_STATE_ACTIVE 7
#define BV_STATE_INACTIVE 8
#define BV_STATE_ESCROW 9

// Bill Validator Command set
#define BV_CMD_RESET 0x30

#define BV_CMD_SETUP_CFG 0x31

#define BV_CMD_SECURITY 0x32

#define BV_CMD_POLL 0x33

#define BV_CMD_BILL_TYPE 0x34

#define BV_CMD_ESCROW 0x35
#define BV_CMD_ESCROW_RETURN 0x00
#define BV_CMD_ESCROW_STACK  0x01

#define BV_CMD_STACKER 0x36

#define BV_CMD_EXPANS                 0x37
#define BV_CMD_EXPANS_ID              0x00
#define BV_CMD_EXPANS_FEAT_E          0x01
#define BV_CMD_EXPANS_ID_OPT          0x02
#define BV_CMD_EXPANS_RECYCLER_SETUP  0x03
#define BV_CMD_EXPANS_RECYCLER_ENABLE 0x04
#define BV_CMD_EXPANS_BILL_DISPENSE_S 0x05
#define BV_CMD_EXPANS_BILL_DISP_TYPE  0x06
#define BV_CMD_EXPANS_BILL_DISP_VALUE 0x07
#define BV_CMD_EXPANS_BILL_DISP_STATE 0x08
#define BV_CMD_EXPANS_BILL_DISP_S_VAL 0x09
#define BV_CMD_EXPANS_BILL_DISP_CANCL 0x0A


#define BV_CMD_NOT_IMPLEMENTED 0xFF

// Coin Changer Response set
#define BV_RESP_JUST_RESET 0x00

#define BV_MDB_ACK 0x00
#define BV_MDB_NACK 0xFF

const mdbCommandDescriptor billValidatorCommands[]{
    //---CMD Name-------------------CMD---SubCMD------Lvl--Len--Action
    {"Reset",                       0x00, NO_SUBCMD,    1,   1, BV_CMD_RESET},
    {"Setup",                       0x01, NO_SUBCMD,    2,   1, BV_CMD_SETUP_CFG},
    {"Security",                    0x02, NO_SUBCMD,    2,   3, BV_CMD_SECURITY},
    {"Poll",                        0x03, NO_SUBCMD,    2,   1, BV_CMD_POLL},
    {"Bill Type",                   0x04, NO_SUBCMD,    2,   5, BV_CMD_BILL_TYPE},
    {"Escrow",                      0x05, NO_SUBCMD,    2,   2, BV_CMD_ESCROW},
    {"Stacker",                     0x06, NO_SUBCMD,    3,   3, BV_CMD_STACKER},
    {"Expansion ID",                0x07, 0x00,         3,   2, BV_CMD_EXPANS},
    {"Expansion Feat",              0x07, 0x01,         3,   6, BV_CMD_EXPANS_FEAT_E},
    {"Expansion Setup",             0x07, 0x02,         3,   2, BV_CMD_EXPANS_ID_OPT},
    {"Expansion Rec Setup",         0x07, 0x03,         3,   2, BV_CMD_EXPANS_RECYCLER_SETUP},
    {"Expansion Rec Enable",        0x07, 0x04,         3,  20, BV_CMD_EXPANS_RECYCLER_ENABLE},
    {"Expansion Dispense S",        0x07, 0x05,         3,   2, BV_CMD_EXPANS_BILL_DISPENSE_S},
    {"Expansion Dispense Type",     0x07, 0x06,         3,   5, BV_CMD_EXPANS_BILL_DISP_TYPE},
    {"Expansion Dispense Value",    0x07, 0x07,         3,   4, BV_CMD_EXPANS_BILL_DISP_VALUE},
    {"Expansion Dispense State",    0x07, 0x08,         3,   2, BV_CMD_EXPANS_BILL_DISP_STATE},
    {"Expansion Dispense State Val",0x07, 0x09,         2,   2, BV_CMD_EXPANS_BILL_DISP_S_VAL},
    {"Expansion Dispense Cancel",   0x07, 0x0A,         2,   2, BV_CMD_EXPANS_BILL_DISP_CANCL},
    {"EOL",                         0x00, 0x00,         0,   0, BV_CMD_NOT_IMPLEMENTED}
};

typedef union __attribute__((packed)) mdbDataRespBV_t
{
        uint8_t raw[37];

        struct __attribute__((packed))
        {
                uint8_t featureLvl;
                uint8_t countryCodeHigh;
                uint8_t countryCodeLow;
                uint8_t scaleFactorHigh;
                uint8_t scaleFactorLow;
                uint8_t decimalPlaces;
                uint8_t stackCapacityHigh;
                uint8_t stackCapacityLow;
                uint8_t billSecurityLevel00 : 1;
                uint8_t billSecurityLevel01 : 1;
                uint8_t billSecurityLevel02 : 1;
                uint8_t billSecurityLevel03 : 1;
                uint8_t billSecurityLevel04 : 1;
                uint8_t billSecurityLevel05 : 1;
                uint8_t billSecurityLevel06 : 1;
                uint8_t billSecurityLevel07 : 1;
                uint8_t billSecurityLevel08 : 1;
                uint8_t billSecurityLevel09 : 1;
                uint8_t billSecurityLevel10 : 1;
                uint8_t billSecurityLevel11 : 1;
                uint8_t billSecurityLevel12 : 1;
                uint8_t billSecurityLevel13 : 1;
                uint8_t billSecurityLevel14 : 1;
                uint8_t billSecurityLevel15 : 1;
                uint8_t escrowCapable;
                uint8_t billValue00;
                uint8_t billValue01;
                uint8_t billValue02;
                uint8_t billValue03;
                uint8_t billValue04;
                uint8_t billValue05;
                uint8_t billValue06;
                uint8_t billValue07;
                uint8_t billValue08;
                uint8_t billValue09;
                uint8_t billValue10;
                uint8_t billValue11;
                uint8_t billValue12;
                uint8_t billValue13;
                uint8_t billValue14;
                uint8_t billValue15;
        } billValidatorSetup;

        struct __attribute__((packed))
        {
                uint8_t BillSecurity;
                uint8_t billSecurityLevel00 : 1;
                uint8_t billSecurityLevel01 : 1;
                uint8_t billSecurityLevel02 : 1;
                uint8_t billSecurityLevel03 : 1;
                uint8_t billSecurityLevel04 : 1;
                uint8_t billSecurityLevel05 : 1;
                uint8_t billSecurityLevel06 : 1;
                uint8_t billSecurityLevel07 : 1;
                uint8_t billSecurityLevel08 : 1;
                uint8_t billSecurityLevel09 : 1;
                uint8_t billSecurityLevel10 : 1;
                uint8_t billSecurityLevel11 : 1;
                uint8_t billSecurityLevel12 : 1;
                uint8_t billSecurityLevel13 : 1;
                uint8_t billSecurityLevel14 : 1;
                uint8_t billSecurityLevel15 : 1;
        } billValidatorSecurity;

        struct __attribute__((packed))
        {
                uint8_t BillType;
                uint8_t billEnable00 : 1;
                uint8_t billEnable01 : 1;
                uint8_t billEnable02 : 1;
                uint8_t billEnable03 : 1;
                uint8_t billEnable04 : 1;
                uint8_t billEnable05 : 1;
                uint8_t billEnable06 : 1;
                uint8_t billEnable07 : 1;
                uint8_t billEnable08 : 1;
                uint8_t billEnable09 : 1;
                uint8_t billEnable10 : 1;
                uint8_t billEnable11 : 1;
                uint8_t billEnable12 : 1;
                uint8_t billEnable13 : 1;
                uint8_t billEnable14 : 1;
                uint8_t billEnable15 : 1;
                uint8_t billEscrowEnable00 : 1;
                uint8_t billEscrowEnable01 : 1;
                uint8_t billEscrowEnable02 : 1;
                uint8_t billEscrowEnable03 : 1;
                uint8_t billEscrowEnable04 : 1;
                uint8_t billEscrowEnable05 : 1;
                uint8_t billEscrowEnable06 : 1;
                uint8_t billEscrowEnable07 : 1;
                uint8_t billEscrowEnable08 : 1;
                uint8_t billEscrowEnable09 : 1;
                uint8_t billEscrowEnable10 : 1;
                uint8_t billEscrowEnable11 : 1;
                uint8_t billEscrowEnable12 : 1;
                uint8_t billEscrowEnable13 : 1;
                uint8_t billEscrowEnable14 : 1;
                uint8_t billEscrowEnable15 : 1;
        } billType;

        struct __attribute__((packed))
        {
                uint16_t billStackerFull;
        } billStackerStatus;

        struct __attribute__((packed))
        {
                uint8_t BillEscrow;
                uint8_t escrowCmnd;
        } escrow;

        struct __attribute__((packed))
        {
                uint8_t BillExpansion;
                char manufacturer[3];
                char serialNumber[12];
                char model[12];
                uint8_t softwareVersionHighByte;
                uint8_t softwareVersionLowByte;
        } billExpansionId;

        struct __attribute__((packed))
        {
                uint8_t BillFeatureEnable;
                uint8_t featureEnable1;
                uint8_t featureEnable2;
                uint8_t featureEnable3;
                uint8_t featureEnable4;
        } billFeatureEnable;
        
        struct __attribute__((packed))
        {
                uint8_t BillExpansionIdLvl3;
                char manufacturer[3];
                char serialNumber[12];
                char model[12];
                uint8_t softwareVersionHighByte;
                uint8_t optFeature24to30;
                uint8_t optFeature16to23;
                uint8_t optFeature8to15;
                uint8_t optFeature7    : 1;
                uint8_t optFeature6    : 1;
                uint8_t optFeature5    : 1;
                uint8_t optFeature4    : 1;
                uint8_t optFeature3    : 1;
                uint8_t optFeature2    : 1;
                uint8_t recycleSupport : 1;
                uint8_t ftlSupported   : 1;
        } billExpansionIdLvl3;

        struct __attribute__((packed))
        {
                uint8_t BillRecyclerRouter;
                uint8_t billRouteRecycler00 : 1;
                uint8_t billRouteRecycler01 : 1;
                uint8_t billRouteRecycler02 : 1;
                uint8_t billRouteRecycler03 : 1;
                uint8_t billRouteRecycler04 : 1;
                uint8_t billRouteRecycler05 : 1;
                uint8_t billRouteRecycler06 : 1;
                uint8_t billRouteRecycler07 : 1;
                uint8_t billRouteRecycler08 : 1;
                uint8_t billRouteRecycler09 : 1;
                uint8_t billRouteRecycler10 : 1;
                uint8_t billRouteRecycler11 : 1;
                uint8_t billRouteRecycler12 : 1;
                uint8_t billRouteRecycler13 : 1;
                uint8_t billRouteRecycler14 : 1;
                uint8_t billRouteRecycler15 : 1;
        } billRecyclerSetup;

        struct __attribute__((packed))
        {
                uint8_t BillDispenseEnable;
                uint8_t billDispenseEnable00 : 1;
                uint8_t billDispenseEnable01 : 1;
                uint8_t billDispenseEnable02 : 1;
                uint8_t billDispenseEnable03 : 1;
                uint8_t billDispenseEnable04 : 1;
                uint8_t billDispenseEnable05 : 1;
                uint8_t billDispenseEnable06 : 1;
                uint8_t billDispenseEnable07 : 1;
                uint8_t billDispenseEnable08 : 1;
                uint8_t billDispenseEnable09 : 1;
                uint8_t billDispenseEnable10 : 1;
                uint8_t billDispenseEnable11 : 1;
                uint8_t billDispenseEnable12 : 1;
                uint8_t billDispenseEnable13 : 1;
                uint8_t billDispenseEnable14 : 1;
                uint8_t billDispenseEnable15 : 1;
                uint8_t billRecyclerEnabled00;
                uint8_t billRecyclerEnabled01;
                uint8_t billRecyclerEnabled02;
                uint8_t billRecyclerEnabled03;
                uint8_t billRecyclerEnabled04;
                uint8_t billRecyclerEnabled05;
                uint8_t billRecyclerEnabled06;
                uint8_t billRecyclerEnabled07;
                uint8_t billRecyclerEnabled08;
                uint8_t billRecyclerEnabled09;
                uint8_t billRecyclerEnabled10;
                uint8_t billRecyclerEnabled11;
                uint8_t billRecyclerEnabled12;
                uint8_t billRecyclerEnabled13;
                uint8_t billRecyclerEnabled14;
                uint8_t billRecyclerEnabled15;
        } billRecyclerEnable;

        struct __attribute__((packed))
        {
                uint8_t BillDispenseStatus;
                uint16_t billDispenserFull;
                uint8_t billDispenseEnable00 : 1;
                uint8_t billDispenseEnable01 : 1;
                uint8_t billDispenseEnable02 : 1;
                uint8_t billDispenseEnable03 : 1;
                uint8_t billDispenseEnable04 : 1;
                uint8_t billDispenseEnable05 : 1;
                uint8_t billDispenseEnable06 : 1;
                uint8_t billDispenseEnable07 : 1;
                uint8_t billDispenseEnable08 : 1;
                uint8_t billDispenseEnable09 : 1;
                uint8_t billDispenseEnable10 : 1;
                uint8_t billDispenseEnable11 : 1;
                uint8_t billDispenseEnable12 : 1;
                uint8_t billDispenseEnable13 : 1;
                uint8_t billDispenseEnable14 : 1;
                uint8_t billDispenseEnable15 : 1;
                uint16_t billCount00;
                uint16_t billCount01;
                uint16_t billCount02;
                uint16_t billCount03;
                uint16_t billCount04;
                uint16_t billCount05;
                uint16_t billCount06;
                uint16_t billCount07;
                uint16_t billCount08;
                uint16_t billCount09;
                uint16_t billCount10;
                uint16_t billCount11;
                uint16_t billCount12;
                uint16_t billCount13;
                uint16_t billCount14;
                uint16_t billCount15;
        } billDispenseStatus;

        struct __attribute__((packed))
        {
                uint8_t BillDispenseBillType;
                uint8_t billTypeToDispense; //b4-b7 be zero
                uint16_t billAmount;
        } billDispenseBillType;

        struct __attribute__((packed))
        {
                uint8_t BillDispenseBillValue;
                uint16_t billValue;
        } billDispenseBillValue;

        struct __attribute__((packed))
        {
                uint8_t BillPayOutStatus;
                uint8_t billPayOutCount00;
                uint8_t billPayOutCount01;
                uint8_t billPayOutCount02;
                uint8_t billPayOutCount03;
                uint8_t billPayOutCount04;
                uint8_t billPayOutCount05;
                uint8_t billPayOutCount06;
                uint8_t billPayOutCount07;
                uint8_t billPayOutCount08;
                uint8_t billPayOutCount09;
                uint8_t billPayOutCount10;
                uint8_t billPayOutCount11;
                uint8_t billPayOutCount12;
                uint8_t billPayOutCount13;
                uint8_t billPayOutCount14;
                uint8_t billPayOutCount15;
        } billPayOutStatus;

        struct __attribute__((packed))
        {
                uint8_t BillAmountCMD;
                uint16_t billAmount;
        } billDispensedValue;

        struct __attribute__((packed))
        {
                uint8_t activity[16];
        } billValidatorPollResult;
} bilValidatorDataResponse;
#endif