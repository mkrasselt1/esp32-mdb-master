#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
// #include <RingBuf.h>
#include <functional>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPUpdate.h>
#include "mdbMaster.h"
#include "time.h"
#include "mbedtls/md.h"

#define ST(A) #A
#define STR(A) ST(A)

WiFiManager wm;
LiquidCrystal_I2C lcd(0x27, 16, 2);  //16x2 LCD Display on I2C Adress 0x27

const char *ntpServer = "de.pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

bool freeVend = 0;
uint16_t prod = 0;
uint32_t funds = 0;


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

void lcdWrite(uint8_t col, uint8_t row, const char *text)
{
  char temp[(17 - col)];
  int8_t textLen = strlen(text);
  if (!textLen || textLen > 16)
  {
    textLen = 16;
  }
  strcpy(temp, text);
  for (uint8_t x = textLen; x < sizeof(temp) - 1; x++)
  {
    temp[x] = ' ';
  }
  temp[sizeof(temp) - 1] = '\0';
  lcd.setCursor(col, row);
  lcd.print(temp);
}

void saveParamsCallback();

void setup()
{
  delay(3000);
  // Pins and Ports
  Serial.begin(115200);
  Wire.setPins(13, 14);

  // Screen and Debug Output
  Serial.println("Booting");
  Serial.print("ESP-MDB-Master Interface vers.:");
  Serial.println(STR(CODE_VERSION));

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.clear();

  lcdWrite(0, 0, "VMC-Master");
  lcdWrite(0, 1, "Booting");

  IPAddress IP;

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP-MDB-Master");

  // wm.resetSettings(); // reset the settings for testing
  
  wm.setConfigPortalBlocking(false);
  wm.setSaveParamsCallback(saveParamsCallback);

  bool res;
  Serial.print("Verbinde WLAN");
  lcdWrite(0, 0, "Verbinde WLAN");

  res = wm.autoConnect("ESP-MDB-Master", WiFi.macAddress().c_str());
  
  if (!res)
  {
    Serial.println("Konfigurationsportal läuft");
    lcdWrite(0, 0, "Konfigurations-");
    lcdWrite(0, 1, "Oberfläche aktiv");
    // ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
    Serial.println("verbunden");
    IP = WiFi.localIP();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  }

  Serial.print("IP address: ");
  Serial.println(IP);
  lcdWrite(0, 0, "IP-Adresse:");
  lcdWrite(0, 1, IP.toString().c_str());
  xTaskCreatePinnedToCore(MDBRUN, "MDB Master Runner", 2048, NULL, 1, NULL, 1);
}


void loop()
{
  wm.process(); // for wifi manager
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
      prod = 1;
      break;
    case '2':
      prod = 2;
      break;
    }
  }
  
  if (mdbMaster::isNewText()) //show mdb text to screen 
  {
    char tempBuff[21];

    lcdWrite(0, 0, "");
    lcdWrite(0, 1, "");

    memcpy(tempBuff, mdbMaster::displayText, 16);
    lcdWrite(0, 0, tempBuff);

    memcpy(tempBuff, &mdbMaster::displayText[16], 16);
    lcdWrite(0, 1, tempBuff);
  }

  // only if product request is pending
  if (prod)
  {
    uint16_t price = 100; //default price is independent from product selection
    Serial.printf("Default Price is %d\r\n", price);


    // only if there is no approval ongoing
    if (mdbMaster::expiredApproval())
    {
      Serial.printf("MDB->Approve Prod. %d / Price %d\r\n", prod, price);
      mdbMaster::approve(prod, price);
    }
    bool result = false;
    // master has approved ?
    if (mdbMaster::hasApproved(&result))
    {
      prod = 0; // delete manual vend request
      if (result)
      {
        Serial.println("MDB->has approved");
        Serial.println("Kauf erfolgreich");
        //Todo: do something for the money now
      }
      else
      {
        Serial.println("MDB->has denied");
        char temp[32];
        sprintf(temp, "Produkt: %2d Preis: %.2f Euro", prod, ((float)price) / 100);
        Serial.println(temp);
      }
    }
  }

  // check for end of vend session end - not yet signaling vend errors :/
  if (true)
  {
    mdbMaster::finishedProduct();
  }

  // example for handling changed funds
  if (funds != mdbMaster::getFunds())
  {
    funds = mdbMaster::getFunds();
    char temp[17];
    sprintf(temp, "Kredit: %.2f Euro", ((float)funds) / 100);
    Serial.println(temp);
  }

  delay(20);
}

void saveParamsCallback()
{
  Serial.println("Get Params:");
  //Todo: save optional parameters
}
