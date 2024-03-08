/*

*/
#ifndef USD_SPEC_H
#define USD_SPEC_H


// USD  Command set

#define USD_RESP_ACK 0x00
#define USD_RESP_NACK 0xFF

typedef union __attribute__((packed)) mdbDataRespUSD_t
{
    uint8_t raw[37];

    struct __attribute__((packed))
    {
        uint8_t cmd;
        union __attribute__((packed))
        {
            #define USD_RESP_JUST_RESET 0x00

            #define USD_RESP_VEND_REQUEST 0x01
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
                uint16_t ItemPrice;
            } VendRequest;
            
            #define USD_RESP_VEND_HOME_SUCCESS 0x02

            #define USD_RESP_VEND_HOME_FAIL 0x03
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
                uint16_t ReasonBitsPad : 10;
                uint8_t ReasonBits_SoldOut : 1;
                uint8_t ReasonBits_Jam : 1;
                uint8_t ReasonBits_NonExistend : 1;
                uint8_t ReasonBits_InvalidRange : 1;
                uint8_t ReasonBits_HealthSafety : 1;
            } VendHomeFail;

            #define USD_RESP_SETUP_DATA 0x04
            struct __attribute__((packed))
            {
                uint8_t FeatureLevel;
                uint16_t MaximumPrice;
                uint16_t ItemNumber;
                uint8_t MaximumResponseTime;
            } SetupData;

            #define USD_RESP_ITEM_PRICE_REQUEST 0x05
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
            } PriceRequest;

            #define USD_RESP_ERROR 0x06
            struct __attribute__((packed))
            {
                uint16_t ReasonBitsPad : 13;
                uint8_t ReasonBits_HealthSafety : 1;
                uint8_t ReasonBits_HomeOrChuteSensor : 1;
                uint8_t ReasonBits_KeypadOrSelectionSwitch : 1;
            } Error;

            #define USD_RESP_PERIPHERAL_ID 0x07
            struct __attribute__((packed))
            {
                char ManufacturerIDCode[3];
                char SerialNumber[12];
                char ModelNumber[12];
                uint16_t SoftwareVersion;
                uint32_t OptionalFeatureBitsPad : 29;
                uint8_t OptionalFeatureBits_CanControlPricing : 1;
                uint8_t OptionalFeatureBits_CanSelectItems : 1;
                uint8_t OptionalFeatureBits_CanFTL : 1;
            } PeripheralID;

            #define USD_RESP_STATUS_RESPONSE 0x08
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
                uint16_t ReasonBitsPad : 10;
                uint8_t ReasonBits_SoldOut : 1;
                uint8_t ReasonBits_Jam : 1;
                uint8_t ReasonBits_NonExistend : 1;
                uint8_t ReasonBits_InvalidRange : 1;
                uint8_t ReasonBits_HealthSafety : 1;
            } StatusResponse;

            #define USD_RESP_MULTI_BLOCK_TRANSFER_RESPONSE 0x09

        };
    };

} USDDataResponse;

typedef union __attribute__((packed)) mdbDataRequUSD_t
{
    uint8_t raw[36];

    struct __attribute__((packed))
    {
        uint8_t cmd;
        uint8_t sub; //has to be VMC featureLvl when sending SetupConfig

        union __attribute__((packed))
        {
            #define USD_REQU_CMD_RESET           0x00
            #define USD_REQU_CMD_RESET_LEN       1

            #define USD_REQU_CMD_SETUP           0x01
            #define USD_REQU_CMD_SETUP_CFG_LEN   6
            struct __attribute__((packed))
            {
                uint16_t ScalingFactor;
                uint8_t DecimalPlaces;
                uint8_t VMCMaxApproveDenyTime;
            } SetupConfigData;

            #define USD_REQU_CMD_POLL                0x02
            #define USD_REQU_CMD_POLL_LEN            1

            #define USD_REQU_CMD_VEND                           0x03
            #define USD_REQU_SUB_VEND_REQUESTED_APPROVED        0x00
            #define USD_REQU_CMD_VEND_REQUESTED_APPROVED_LEN    2

            #define USD_REQU_SUB_VEND_REQUESTED_DISAPPROVED        0x01
            #define USD_REQU_CMD_VEND_REQUESTED_DISAPPROVED_LEN    2

            #define USD_REQU_SUB_VEND_VEND_ITEM        0x02
            #define USD_REQU_CMD_VEND_VEND_ITEM_LEN    4
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
            } VendItem;

            #define USD_REQU_SUB_VEND_HOME_ITEM        0x03
            #define USD_REQU_CMD_VEND_HOME_ITEM_LEN    4
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
            } HomeItem;

            #define USD_REQU_SUB_VEND_REQUEST_ITEM_STATUS        0x04
            #define USD_REQU_CMD_VEND_REQUEST_ITEM_STATUS_LEN    4
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
            } RequestItemStatus;

            #define USD_REQU_CMD_FUNDS                  0x04
            #define USD_REQU_SUB_FUNDS_AVAILABLE        0x00
            #define USD_REQU_CMD_FUNDS_AVAILABLE_LEN    4
            struct __attribute__((packed))
            {
                uint16_t FundsAvailable;
            } Funds;

            #define USD_REQU_SUB_FUNDS_ITEM_PRICE       0x01
            #define USD_REQU_CMD_FUNDS_ITEM_PRICE_LEN    8
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
                uint16_t SelectionPrice;
                char AlphanumericIdentifier[2];
            } ItemPrice;

            #define USD_REQU_CMD_CONTROL                0x05
            #define USD_REQU_SUB_CONTROL_DISABLE        0x00
            #define USD_REQU_CMD_CONTROL_DISABLE_LEN    2

            #define USD_REQU_SUB_CONTROL_ENABLE         0x01
            #define USD_REQU_CMD_CONTROL_ENABLE_LEN     2

            #define USD_REQU_CMD_EXPANSION                  0x07
            #define USD_REQU_SUB_EXPANSION_REQUEST_ID       0x00
            #define USD_REQU_CMD_EXPANSION_REQUEST_ID_LEN   2

            #define USD_REQU_SUB_EXPANSION_FEATURE_ENABLE       0x01
            #define USD_REQU_CMD_EXPANSION_FEATURE_ENABLE_LEN   6
            struct __attribute__((packed))
            {
                uint32_t OptionalFeatureBitsPad : 24;                   //[0]::0 - [2]::7
                uint8_t OptionalFeatureBits_CanControlPricing : 1;      //[3]::0
                uint8_t OptionalFeatureBits_CanSelectItems : 1;         //[3]::1
                uint8_t OptionalFeatureBits_CanFTL : 1;                 //[3]::2
            } EnableOptionalFeatureBits;

            #define USD_REQU_SUB_EXPANSION_NUMBER_OF_DATA_BLOCKS        0x02
            #define USD_REQU_CMD_EXPANSION_NUMBER_OF_DATA_BLOCKS_LEN    3
            struct __attribute__((packed))
            {
                uint8_t NumberOfDataBlocks;
            } BlocksToSend;

            #define USD_REQU_SUB_EXPANSION_SEND_BLOCK           0x03
            #define USD_REQU_CMD_EXPANSION_SEND_BLOCK_LEN_MAX   36
            #define USD_REQU_CMD_EXPANSION_SEND_BLOCK_LEN_MAX_DATA   33
            struct __attribute__((packed))
            {
                uint8_t BlockNumber;
                uint8_t Data[33];
            } SendBlock;

            #define USD_REQU_SUB_EXPANSION_REQUEST_BLOCK        0x04
            #define USD_REQU_CMD_EXPANSION_REQUEST_BLOCK_LEN    3
            struct __attribute__((packed))
            {
                uint8_t BlockNumber;
                uint8_t Data[33];
            } RequestBlock;

        };
    };

    

} USDDataRequest;


#endif