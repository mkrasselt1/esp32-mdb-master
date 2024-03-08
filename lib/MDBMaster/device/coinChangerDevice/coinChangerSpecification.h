/*

*/
#ifndef COIN_SPEC_H
#define COIN_SPEC_H


// Coin Changer  Command set

#define COIN_RESP_ACK 0x00
#define COIN_RESP_NACK 0xFF

typedef union __attribute__((packed)) mdbDataRespCOIN_t
{
    uint8_t raw[37];

    struct __attribute__((packed))
    {
        union __attribute__((packed))
        {
            #define COIN_RESP_STATUS                            0b00000000
            #define COIN_RESP_STATUS_MASK                       0b11100000

            #define COIN_RESP_STATUS_ESCROW_REQU                0x01
            #define COIN_RESP_STATUS_PAYOUT_BUSY                0x02
            #define COIN_RESP_STATUS_NOCREDIT                   0x03
            #define COIN_RESP_STATUS_DEFECTIVE_TUBE_SENSOR      0x04
            #define COIN_RESP_STATUS_DOUBLE_ARRIVAL             0x05
            #define COIN_RESP_STATUS_ACCEPTOR_UNPLUGGED         0x06
            #define COIN_RESP_STATUS_TUBE_JAM                   0x07
            #define COIN_RESP_STATUS_ROM_CHKSUM_ERROR           0x08
            #define COIN_RESP_STATUS_COIN_ROUTING_ERROR         0x09
            #define COIN_RESP_STATUS_CHANGER_BUSY               0x0A
            #define COIN_RESP_STATUS_JUST_RESET                 0x0B
            #define COIN_RESP_STATUS_COIN_JAM                   0x0C
            #define COIN_RESP_STATUS_POSSIBLE_CREDIT_REMOVAL    0x0D

            #define COIN_RESP_COINS_DISPENSED                   0b10000000
            #define COIN_RESP_COINS_DISPENSED_MASK              0b10000000
            
            #define COIN_RESP_COINS_DEPOSITED                   0b01000000
            #define COIN_RESP_COINS_DEPOSITED_MASK              0b11000000

            #define COIN_RESP_SLUG                              0b00100000
            #define COIN_RESP_SLUG_MASK                         0b11100000

            struct __attribute__((packed))
            {
                uint8_t data[16];
            } activity;

            struct __attribute__((packed))
            {        
                uint8_t ChangerFeatureLevel;
                uint16_t CurrencyCode;
                uint8_t CoinScalingFactor;
                uint8_t DecimalPlaces;
                uint16_t CoinTypeRouting;
                uint8_t CoinTypeCredit[16];
            } setupData;

            struct __attribute__((packed))
            {        
                uint16_t FullStatus;
                uint8_t TubeStatus[16];
            } tubeStatus;

            struct __attribute__((packed))
            {        
                char ManufacturerCode[3];
                char SerialNumber[12];
                char ModelNumber[12];
                uint16_t SoftwareVersion;
                uint32_t OptionalFeatureBitsPad : 24;                   //[0]::0 - [2]::7
                uint8_t OptionalFeatureBits_AlternatePayout : 1;        //[3]::0
                uint8_t OptionalFeatureBits_ExtendedDiagnostics : 1;    //[3]::1
                uint8_t OptionalFeatureBits_ControlledManualFill : 1;   //[3]::2
                uint8_t OptionalFeatureBits_FTLSupport : 1;             //[3]::3
            } expansionID;
        };
    };

} COINDataResponse;

typedef union __attribute__((packed)) mdbDataRequCOIN_t
{
    uint8_t raw[36];

    struct __attribute__((packed))
    {
        uint8_t cmd;

        union __attribute__((packed))
        {
            #define COIN_REQU_CMD_RESET             0x00
            #define COIN_REQU_CMD_RESET_LEN         1

            #define COIN_REQU_CMD_SETUP             0x01
            #define COIN_REQU_CMD_SETUP_LEN         1

            #define COIN_REQU_CMD_TUBE_STATUS       0x02
            #define COIN_REQU_CMD_TUBE_STATUS_LEN   1

            #define COIN_REQU_CMD_POLL              0x03
            #define COIN_REQU_CMD_POLL_LEN          1

            #define COIN_REQU_CMD_COIN_TYPE         0x04
            #define COIN_REQU_CMD_COIN_TYPE_LEN     5
            struct __attribute__((packed))
            {
                uint16_t coinEnable;
                uint16_t manualDispenseEnable;
            } coinType;


            #define COIN_REQU_CMD_DISPENSE          0x05
            #define COIN_REQU_CMD_DISPENSE_LEN      2
            struct __attribute__((packed))
            {
                uint8_t amountCoinType;              
            } dispense;

            #define COIN_REQU_CMD_EXPANSION                  0x07
            struct __attribute__((packed))
            {
                uint8_t subCmd;
                union __attribute__((packed))
                {
                    #define COIN_REQU_SUB_EXPANSION_REQUEST_ID       0x00
                    #define COIN_REQU_CMD_EXPANSION_REQUEST_ID_LEN   2

                    #define COIN_REQU_SUB_EXPANSION_FEATURE_ENABLE       0x01
                    #define COIN_REQU_CMD_EXPANSION_FEATURE_ENABLE_LEN   6
                    struct __attribute__((packed))
                    {
                        uint32_t features;        
                    } featureEnable;
                };
            } expansion;
        };
    };

    

} COINDataRequest;


#endif