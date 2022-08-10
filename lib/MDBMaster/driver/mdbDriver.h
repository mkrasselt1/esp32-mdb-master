#ifndef MDB_DRIVER_H
#define MDB_DRIVER_H

#include <Arduino.h>
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"
#include "../mdbMaster.h"

#define MDB_ACK 0x00
#define MDB_NACK 0xFF

class mdbDriver{
    public:
        static void init();
        static void start();
        static void send(uint16_t data);
        static void sendPacket(uint16_t* toSend, uint16_t len);
        static void disable();
        static void enable();
    
    private:
        static rmt_config_t rmt_rx;
        static rmt_config_t rmt_tx;
        static RingbufHandle_t rb_tx;
        static void swSerialTask(void* args);
        static bool disabled;
        
};


#endif
