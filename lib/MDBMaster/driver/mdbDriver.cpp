#include "mdbDriver.h"

volatile const uint8_t bitTime = 104;
volatile const uint8_t bitTimeSnd = 82;
volatile const uint8_t bitNum = 11;
rmt_config_t mdbDriver::rmt_rx;
rmt_config_t mdbDriver::rmt_tx;
RingbufHandle_t mdbDriver::rb_tx;
bool mdbDriver::disabled;

static void IRAM_ATTR u16_to_rmt(const void *src, rmt_item32_t *dest, size_t src_size,
                                 size_t wanted_num, size_t *translated_size, size_t *item_num)
{
  if (src == NULL || dest == NULL)
  {
    *translated_size = 0;
    *item_num = 0;
    return;
  }

  size_t size = 0;
  size_t num = 0;
  uint16_t *psrc = (uint16_t *)src;
  rmt_item32_t *pdest = dest;
  uint8_t lastState = 1;
  /*
  pdest->level0 = 0;
  pdest->duration0 = 0;
  pdest->level1 = 1;
  pdest->duration1 = 0;*/
  pdest--;
  while (size < (src_size / sizeof(uint16_t)) && num < wanted_num)
  {
    for (int i = 0; i < bitNum; i++)
    {
      if (*psrc & (0x1 << i))
      {
        pdest->duration1 += bitTimeSnd;
        lastState = 1;
      }
      else
      {
        if (lastState)
        {
          num++;
          pdest++;
          lastState = 0;
          pdest->level0 = 1; // New Hardware has inverter
          pdest->duration0 = 0;
          pdest->level1 = 0;
          pdest->duration1 = 0;
        }
        pdest->duration0 += bitTimeSnd;
      }
    }
    size++;
    psrc++;
  }
  *translated_size = size;
  *item_num = num;
}

void mdbDriver::init()
{
  rmt_tx.rmt_mode = RMT_MODE_TX;
  rmt_tx.channel = RMT_CHANNEL_6;
  rmt_tx.gpio_num = GPIO_NUM_32;
  rmt_tx.mem_block_num = 1;
  rmt_tx.tx_config.loop_en = 0;
  rmt_tx.tx_config.carrier_en = 0;
  rmt_tx.tx_config.idle_output_en = 1;
  rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW; // New Hardware has inverter
  rmt_tx.clk_div = 100;

  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
  rmt_translator_init(rmt_tx.channel, u16_to_rmt);

  rmt_rx.channel = RMT_CHANNEL_0;
  rmt_rx.gpio_num = GPIO_NUM_35;
  rmt_rx.clk_div = 200;
  rmt_rx.mem_block_num = 5;
  rmt_rx.rmt_mode = RMT_MODE_RX;
  rmt_rx.rx_config.filter_en = true;
  rmt_rx.rx_config.filter_ticks_thresh = 5;
  rmt_rx.rx_config.idle_threshold = 500;

  rmt_config(&rmt_rx);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
  disabled = false;
}

void mdbDriver::disable()
{
  rmt_driver_uninstall(rmt_rx.channel);
  rmt_driver_uninstall(rmt_tx.channel);
  disabled = true;
}

void mdbDriver::enable()
{
  rmt_driver_install(rmt_tx.channel, 0, 0);
  rmt_translator_init(rmt_tx.channel, u16_to_rmt);
  rmt_driver_install(rmt_rx.channel, 1000, 0);
  disabled = false;
}

void mdbDriver::start()
{
  init();
  xTaskCreatePinnedToCore(swSerialTask, "mdbDriver", 2048, NULL, 1, NULL, 0);
}

void mdbDriver::send(uint16_t data)
{
  uint16_t tmp[1];
  tmp[0] = (data << 1) | (1 << 10);
  rmt_wait_tx_done(rmt_tx.channel, portMAX_DELAY);
  rmt_write_sample(rmt_tx.channel, (uint8_t *)tmp, sizeof(tmp), false);
}

void mdbDriver::sendPacket(uint16_t *toSend, uint16_t len)
{
  for (int i = 0; i < len; i++)
  {
    mdbDriver::send(toSend[i]);
  }
}

void mdbDriver::swSerialTask(void *args)
{
  RingbufHandle_t rb = NULL;
  rmt_get_ringbuf_handle(rmt_rx.channel, &rb);
  rmt_rx_start(rmt_rx.channel, 1);
  uint16_t currentByte = 0;
  int16_t currBit = 0;
  int16_t currTime = 0;
  bool framingError = false;

  while (rb)
  {
    if (!disabled)
    {
      size_t rx_size = 0;
      rmt_item32_t *item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, portMAX_DELAY);
      rmt_item32_t *cItem;
      if (item)
      {
        for (int i = 0; i < rx_size >> 2; i++)
        {
          // printf("%d:%dus %d:%dus\n", (item+i)->level0, (((item+i)->duration0)*25)/10, (item+i)->level1, (((item+i)->duration1)*25)/10);

          cItem = (item + i);
          currTime = ((cItem->duration0) * 25) / 10;
          while ((currTime > (bitTime >> 1)) && (!framingError))
          {
            currTime -= bitTime;
            currBit++;
            if (currBit >= bitNum)
            {
              // error, stop bit mismatch
              framingError = true;
              break;
            }
          }

          currTime = ((cItem->duration1) * 25) / 10;
          while ((currTime > (bitTime >> 1)) && (!framingError))
          {
            currentByte |= (1 << currBit);
            currTime -= bitTime;
            currBit++;
            if (currBit >= bitNum)
            {
              // byte completed
              currentByte = (currentByte >> 1) & 0x1FF;

              // Serial.printf("REC: %03X\n", currentByte);

              mdbMaster::receive(currentByte);
              currBit = 0;
              currentByte = 0;
              break;
            }
          }

          // Bedingungen für end of byte->anzahl bits oder duration = 0
          if (cItem->duration1 == 0)
          {
            if (!framingError)
            {
              while (currBit < bitNum)
              {
                currentByte |= (1 << currBit);
                currBit++;
              }
              currentByte = (currentByte >> 1) & 0x1FF;
              // Serial.printf("REC: %03X\n", currentByte);

              mdbMaster::receive(currentByte);
            }

            currBit = 0;
            currentByte = 0;
            framingError = false;
          }
        }
        vRingbufferReturnItem(rb, (void *)item);
      }
      else
      {
        break;
      }
    }
  }
}