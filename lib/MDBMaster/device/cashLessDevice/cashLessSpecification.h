/*

*/
#ifndef CASHLESS_SPEC_H
#define CASHLESS_SPEC_H


// Cashless  Command set

#define CL_RESP_ACK 0x00
#define CL_RESP_NACK 0xFF

typedef union __attribute__((packed)) mdbDataRespCL_t
{
    uint8_t raw[37];

    struct __attribute__((packed))
    {
        uint8_t cmd;
        union __attribute__((packed))
        {
            #define CL_RESP_JUST_RESET 0x00

            #define CL_RESP_READER_CONFIG 0x01
            struct __attribute__((packed))
            {        
                uint8_t ReaderFeatureLevel;
                uint8_t CountryCodeHigh;
                uint8_t CountryCodeLow;
                uint8_t ScaleFactor;
                uint8_t DecimalPlaces;
                uint8_t AppMaxResponseTime;
                
                uint8_t Option0_CanRestoreFunds : 1;                
                uint8_t Option1_CanMultivend : 1;
                uint8_t Option2_HasOwnDisplay : 1;
                uint8_t Option3_CanCashsale : 1;
                uint8_t Option4_7_Unused : 4;

            } ReaderConfigData;

            #define CL_RESP_DISPLAY_REQ 0x02
            struct __attribute__((packed))
            {
                uint8_t DisplayTime;
                char DisplayData[32];
            } DisplayRequest;

            #define CL_RESP_BEGIN_SESSION 0x03
            #define CL_RESP_BEGIN_SESSION_LEN_LVL1 3
            #define CL_RESP_BEGIN_SESSION_LEN_LVL23 10
            #define CL_RESP_BEGIN_SESSION_LEN_LVL3_EXP 17
            struct __attribute__((packed))
            {
                uint16_t FundsAvailable;
                uint32_t PaymentMediaID;
                uint8_t PaymentType;
                uint16_t PaymentData;
            } BeginSession;

            #define CL_RESP_SESSION_CANCEL_REQ 0x04

            #define CL_RESP_VEND_APPROVED 0x05
            #define CL_RESP_VEND_APPROVED_LEN 3
            #define CL_RESP_VEND_APPROVED_LEN_LVL3_EXP 5
            struct __attribute__((packed))
            {
                uint16_t VendAmount;
            } VendApproved;
            #define CL_RESP_VEND_DENIED 0x06
            #define CL_RESP_END_SESSION 0x07


            #define CL_RESP_PERIPHERAL_ID 0x09
            struct __attribute__((packed))
            {
                char ManufacturerCode[3];
                char SerialNumber[12];
                char ModelNumber[12];
                uint16_t SoftwareVersion;
                uint32_t OptionalFeatureBitsPad : 24;                   //[0]::0 - [2]::7
                uint8_t OptionalFeatureBits_CanFTL : 1;                 //[3]::0
                uint8_t OptionalFeatureBits_MonetaryFormat : 1;         //[3]::1
                uint8_t OptionalFeatureBits_CanMultiCurrency : 1;       //[3]::2
                uint8_t OptionalFeatureBits_CanNegativeVend : 1;        //[3]::3
                uint8_t OptionalFeatureBits_CanDataEntry : 1;           //[3]::4
                uint8_t OptionalFeatureBits_CanAlwaysIdle : 1;          //[3]::5

            } PeripheralID;

            #define CL_RESP_REVALUE_APPROVED 0x0D
            #define CL_RESP_REVALUE_DENIED   0x0E
            #define CL_RESP_REVALUE_LIMIT    0x0F
            #define CL_RESP_REVALUE_LIMIT_AMOUNT_LEN 3
            struct __attribute__((packed))
            {
                uint16_t RevalueAmount;
            } RevalueLimit;

        };
    };

} CashLessDataResponse;

typedef union __attribute__((packed)) mdbDataRequCL_t
{
    uint8_t raw[36];

    struct __attribute__((packed))
    {
        uint8_t cmd;
        uint8_t sub;

        union __attribute__((packed))
        {
            #define CL_REQU_CMD_RESET           0x00
            #define CL_REQU_CMD_RESET_LEN       1

            #define CL_REQU_CMD_SETUP           0x01
            #define CL_REQU_SUB_SETUP_CFG       0x00
            #define CL_REQU_CMD_SETUP_CFG_LEN   6
            struct __attribute__((packed))
            {
                uint8_t VMCFeatureLevel;
                uint8_t ColumnsOnDisplay;
                uint8_t RowsOnDisplay;
                uint8_t DisplayInformation;
            } SetupConfigData;

            #define CL_REQU_SUB_SETUP_MAXMIN       0x01
            #define CL_REQU_CMD_SETUP_MAXMIN_LEN   6
            struct __attribute__((packed))
            {
                uint16_t MaximumPrice;
                uint16_t MinimumPrice;
            } SetupConfigMaxMinPrices;




            #define CL_REQU_CMD_EXPANSION           0x07
            #define CL_REQU_SUB_EXPANSION_ID        0x00
            #define CL_REQU_CMD_EXPANSION_ID_LEN    31
            struct __attribute__((packed))
            {
                char ManufacturerCode[3];
                char SerialNumber[12];
                char ModelNumber[12];
                uint16_t SoftwareVersion;
            } ExpansionID;



            #define CL_REQU_CMD_POLL                0x02
            #define CL_REQU_CMD_POLL_LEN            1

            #define CL_REQU_CMD_VEND                0x03
            #define CL_REQU_SUB_VEND_REQUEST        0x00
            #define CL_REQU_CMD_VEND_REQUEST_LEN    6
            struct __attribute__((packed))
            {
                uint16_t ItemPrice;
                uint16_t ItemNumber;
            } VendRequest;

            #define CL_REQU_SUB_VEND_CANCEL        0x01
            #define CL_REQU_CMD_VEND_CANCEL_LEN    2

            #define CL_REQU_SUB_VEND_SUCCESS        0x02
            #define CL_REQU_CMD_VEND_SUCCESS_LEN    4
            struct __attribute__((packed))
            {
                uint16_t ItemNumber;
            } VendSuccess;

            #define CL_REQU_SUB_VEND_FAILURE        0x03
            #define CL_REQU_CMD_VEND_FAILURE_LEN    2

            #define CL_REQU_SUB_VEND_SESSION_COMPLETE        0x04
            #define CL_REQU_CMD_VEND_SESSION_COMPLETE_LEN    2

            #define CL_REQU_CMD_READER              0x04
            #define CL_REQU_SUB_READER_DISABLE      0x00
            #define CL_REQU_CMD_READER_DISABLE_LEN  2
            #define CL_REQU_SUB_READER_ENABLE       0x01
            #define CL_REQU_CMD_READER_ENABLE_LEN   2

            #define CL_REQU_CMD_REVALUE               0x05
            #define CL_REQU_SUB_REVALUE_REQUEST       0x00
            #define CL_REQU_CMD_REVALUE_REQUEST_LEN   4
            struct __attribute__((packed))
            {
                uint16_t RevalueAmount;
            } RevalueRequest;

            #define CL_REQU_SUB_REVALUE_LIMIT         0x01
            #define CL_REQU_CMD_REVALUE_LIMIT_LEN     2

        };
    };

    

} CashLessDataRequest;


#endif