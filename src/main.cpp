#include <Arduino.h>
// #include <RingBuf.h>
#include <functional>
#include <Wire.h>
#include "mdbMaster.h"
#include "time.h"
#include "mbedtls/md.h"

#define ST(A) #A
#define STR(A) ST(A)

bool freeVend = 0;
uint16_t prod = 0;
uint32_t funds = 0;
bool inhibit = false;

void MDBRUN(void *args)
{
  // char tmp[30];
  mdbMaster::init();
  while (1)
  {
    mdbMaster::pollAll();
    vTaskDelay(10);
  }
}

void activateCoinChannel(uint8_t channelId)
{
  switch (channelId)
  {
  case 0:
    digitalWrite(21, false);
    delay(100);
    digitalWrite(21, true);
    break;
  case 1:
    digitalWrite(33, false);
    delay(100);
    digitalWrite(33, true);
    break;
  case 2:
    digitalWrite(22, false);
    delay(100);
    digitalWrite(22, true);
    break;
  case 3:
    // digitalWrite(34, true);
    // delay(100);
    // digitalWrite(34, false);
    break;
  }
}

void setup()
{
  pinMode(21, OUTPUT);
  digitalWrite(21, true);
  pinMode(22, OUTPUT);
  digitalWrite(22, true);
  pinMode(33, OUTPUT);
  digitalWrite(33, true);
  pinMode(34, INPUT_PULLUP);
  delay(3000);

  // Pins and Ports
  Serial.begin(115200);
  Serial.println("Booting");
  Serial.print("Bill2Coin Translator:");
  Serial.println(STR(CODE_VERSION));

  xTaskCreatePinnedToCore(MDBRUN, "MDB Master Runner", 2048, NULL, 1, NULL, 1);
}

void loop()
{
  if (Serial.available())
  {
    switch (Serial.read())
    {
    case 'r':
      ESP.restart();
      break;
    case 'e':
      mdbCashLess::reqSessionComplete();
      break;
    case '1':
      activateCoinChannel(0);
      break;
    case '2':
      activateCoinChannel(1);
      break;
    case '3':
      activateCoinChannel(2);
      break;
    case '4':
      activateCoinChannel(3);
      break;
    }
  }

  if (digitalRead(34) != inhibit)
  {
    if (inhibit)
    {
      Serial.println("inhibit bills");
      mdbBillValidator::setInhibit(1);
    }
    else
    {
      Serial.println("enable bills");
      mdbBillValidator::setInhibit(2);
    }
    inhibit = !inhibit;
  }

  if (funds != mdbMaster::getFunds())
  {
    funds = mdbMaster::getFunds();
    switch (funds)
    {
    case 500:
      activateCoinChannel(0);
      break;
    case 1000:
      activateCoinChannel(1);
      break;
    case 2000:
      activateCoinChannel(2);
      break;
    case 5000:
      activateCoinChannel(3);
      break;
    }
  }
  delay(20);
}