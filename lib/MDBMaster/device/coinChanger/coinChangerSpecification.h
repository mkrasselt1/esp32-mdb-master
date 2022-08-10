#ifndef COINCHANGER_SPEC_H
#define COINCHANGER_SPEC_H

#include "../../mdbStructure.h"

// Coin Changer MDB States

#define CC_STATE_MISSING 0
#define CC_STATE_RESET 1
#define CC_STATE_SETUP 2
#define CC_STATE_FEATURE_ID 3
#define CC_STATE_FEATURE_ENABLE 4
#define CC_STATE_TUBE_STATUS 5
#define CC_STATE_COIN_ENABLE 6
#define CC_STATE_ACTIVE 7
#define CC_STATE_INACTIVE 8
#define CC_STATE_ESCROW 9

// Coin Changer Command set
#define CC_CMD_RESET 0x08

#define CC_CMD_SETUP_CFG 0x09

#define CC_CMD_TUBE_STATUS 0x0A

#define CC_CMD_POLL 0x0B

#define CC_CMD_COIN_ACC 0x0C

#define CC_CMD_DISPENSE 0x0D

#define CC_CMD_EXPANS 0x0F
#define CC_CMD_EXPANS_ID 0x00
#define CC_CMD_EXPANS_FEAT_E 0x01
#define CC_CMD_EXPANS_DEBUG 0x02

#define CC_CMD_NOT_IMPLEMENTED 0xFF

// Coin Changer Response set
#define CC_RESP_JUST_RESET 0x00

#define CC_MDB_ACK 0x00
#define CC_MDB_NACK 0xFF

const mdbCommandDescriptor coinChangerCommands[]{
    //---CMD Name-------------------CMD---SubCMD------Lvl--Len--Action
    {"Reset", 0x00, NO_SUBCMD, 1, 1, CC_CMD_RESET},
    {"Setup", 0x01, NO_SUBCMD, 2, 23, CC_CMD_SETUP_CFG},
    {"Tube Status", 0x02, NO_SUBCMD, 2, 18, CC_CMD_TUBE_STATUS},
    {"Poll", 0x03, NO_SUBCMD, 2, 16, CC_CMD_POLL},
    {"Coin Type", 0x04, NO_SUBCMD, 2, 4, CC_CMD_COIN_ACC},
    {"Dispense", 0x05, NO_SUBCMD, 2, 1, CC_CMD_DISPENSE},
    {"Expansion Id", 0x06, 0x00, 3, 1, CC_CMD_EXPANS_ID},
    {"Expansion Feat", 0x06, 0x01, 3, 1, CC_CMD_EXPANS_FEAT_E},
    {"Expansion Debug", 0x06, 0x02, 3, 1, CC_CMD_EXPANS_DEBUG},
    {"EOL", 0x00, 0x00, 0, 0, CC_CMD_NOT_IMPLEMENTED}};

typedef union __attribute__((packed)) mdbDataRespCC_t
{
        uint8_t raw[37];

        struct __attribute__((packed))
        {
                uint8_t CoinChangerFeatureLvl;
                uint8_t countryCodeHigh;
                uint8_t countryCodeLow;
                uint8_t scaleFactor;
                uint8_t decimalPlaces;
                uint8_t CoinB15RouteToTube : 1;
                uint8_t CoinB14RouteToTube : 1;
                uint8_t CoinB13RouteToTube : 1;
                uint8_t CoinB12RouteToTube : 1;
                uint8_t CoinB11RouteToTube : 1;
                uint8_t CoinB10RouteToTube : 1;
                uint8_t CoinB09RouteToTube : 1;
                uint8_t CoinB08RouteToTube : 1;
                uint8_t CoinB07RouteToTube : 1;
                uint8_t CoinB06RouteToTube : 1;
                uint8_t CoinB05RouteToTube : 1;
                uint8_t CoinB04RouteToTube : 1;
                uint8_t CoinB03RouteToTube : 1;
                uint8_t CoinB02RouteToTube : 1;
                uint8_t CoinB01RouteToTube : 1;
                uint8_t CoinB00RouteToTube : 1;
                uint8_t coinValueScaledB00;
                uint8_t coinValueScaledB01;
                uint8_t coinValueScaledB02;
                uint8_t coinValueScaledB03;
                uint8_t coinValueScaledB04;
                uint8_t coinValueScaledB05;
                uint8_t coinValueScaledB06;
                uint8_t coinValueScaledB07;
                uint8_t coinValueScaledB08;
                uint8_t coinValueScaledB09;
                uint8_t coinValueScaledB10;
                uint8_t coinValueScaledB11;
                uint8_t coinValueScaledB12;
                uint8_t coinValueScaledB13;
                uint8_t coinValueScaledB14;
                uint8_t coinValueScaledB15;
                uint8_t appMaxResponseTime;
        } CoinChangerSetup;

        struct __attribute__((packed))
        {
                uint8_t CoinB15TubeFull : 1;
                uint8_t CoinB14TubeFull : 1;
                uint8_t CoinB13TubeFull : 1;
                uint8_t CoinB12TubeFull : 1;
                uint8_t CoinB11TubeFull : 1;
                uint8_t CoinB10TubeFull : 1;
                uint8_t CoinB09TubeFull : 1;
                uint8_t CoinB08TubeFull : 1;
                uint8_t CoinB07TubeFull : 1;
                uint8_t CoinB06TubeFull : 1;
                uint8_t CoinB05TubeFull : 1;
                uint8_t CoinB04TubeFull : 1;
                uint8_t CoinB03TubeFull : 1;
                uint8_t CoinB02TubeFull : 1;
                uint8_t CoinB01TubeFull : 1;
                uint8_t CoinB00TubeFull : 1;
                uint8_t CoinB00Level;
                uint8_t CoinB01Level;
                uint8_t CoinB02Level;
                uint8_t CoinB03Level;
                uint8_t CoinB04Level;
                uint8_t CoinB05Level;
                uint8_t CoinB06Level;
                uint8_t CoinB07Level;
                uint8_t CoinB08Level;
                uint8_t CoinB09Level;
                uint8_t CoinB10Level;
                uint8_t CoinB11Level;
                uint8_t CoinB12Level;
                uint8_t CoinB13Level;
                uint8_t CoinB14Level;
                uint8_t CoinB15Level;
                uint8_t appMaxResponseTime;
        } CoinChangerTubeStatus;

        struct __attribute__((packed))
        {
                char manufacturer[4];
                char serialNumber[12];
                char model[12];
                uint8_t softwareVersionHighByte;
                uint8_t softwareVersionLowByte;
                uint8_t OptFeature31: 1;
                uint8_t OptFeature30: 1;
                uint8_t OptFeature29: 1;
                uint8_t OptFeature28: 1;
                uint8_t OptFeature27: 1;
                uint8_t OptFeature26: 1;
                uint8_t OptFeature25: 1;
                uint8_t OptFeature24: 1;
                uint8_t OptFeature23: 1;
                uint8_t OptFeature22: 1;
                uint8_t OptFeature21: 1;
                uint8_t OptFeature20: 1;
                uint8_t OptFeature19: 1;
                uint8_t OptFeature18: 1;
                uint8_t OptFeature17: 1;
                uint8_t OptFeature16: 1;
                uint8_t OptFeature15: 1;
                uint8_t OptFeature14: 1;
                uint8_t OptFeature13: 1;
                uint8_t OptFeature12: 1;
                uint8_t OptFeature11: 1;
                uint8_t OptFeature10: 1;
                uint8_t OptFeature09: 1;
                uint8_t OptFeature08: 1;
                uint8_t OptFeature07: 1;
                uint8_t OptFeature06: 1;
                uint8_t OptFeature05: 1;
                uint8_t OptFeature04: 1;
                uint8_t OptFeatureFileTransport : 1;
                uint8_t OptFeatureManualPayoutIn : 1;
                uint8_t OptFeatureExtendedDiagnose : 1;
                uint8_t OptFeatureAlternativePayout : 1;
        } CoinChangerFeatureId;

        struct __attribute__((packed))
        {
                uint8_t activity[16];
        } CoinChangerPollResult;
} CoinChangerDataResponse;
#endif