#include <Arduino.h>
#include <LoRa.h>
#include <Sleep_n0m1.h>
#include <SPI.h>
#include <elapsedMillis.h>

////////////////////////////////////////////
Sleep slp;
elapsedMillis Mtime;
elapsedMillis Btime;
////////////////////////////////////////////
#define SPIN 0
const int PROGMEM LCS = 4;
const int PROGMEM LRST = 3;
const int PROGMEM LDIO = 2;
////////////////////////////////////////////
/// Device ID - 8bit integer value ///
byte tag = 108;
byte devtype = 110;
////////////////////////////////////////////
bool state = false;
bool mode = false; // false = magnet mode/ true = repeater mode
int SleepDelay = 5; //in seconds
////////////////////////////////////////////

void initialPing(){
  struct ping{
    byte tag;
    byte request;
  }px1;

  struct resp{
    byte tag;
    byte resp;
  }rs1;

  int x;

  px1.tag = tag;
  px1.request = (byte)73;


  LoRa.idle();
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&px1, sizeof(px1));
  LoRa.endPacket();

  Btime = 0;
  while (Btime < 180*1000)
  {
    Serial.println(F("Waiting for mode input"));
    x = LoRa.parsePacket();
    if (x == 2)
    {
      while (LoRa.available())
      {
        LoRa.readBytes((uint8_t*)&rs1, sizeof(rs1));
      }
      break;      
    }    
  } 
  LoRa.sleep();

  if (rs1.tag == tag && rs1.resp == 77)
  {
    mode = false;
  }

  if (rs1.tag == tag && rs1.resp == 114)
  {
    mode = true;
  }
}

void Ping(byte a, bool s, byte d){
  struct ping{
    uint8_t ta;
    uint8_t devtyp;
    bool st;
  }p;
  p.devtyp = d;
  p.ta = a;
  p.st = s;

  Serial.print(F("len")); Serial.println((int)sizeof(p));
  LoRa.idle();
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&p, sizeof(p));
  LoRa.endPacket();
  LoRa.sleep();
}

void setup() {
  // put your setup code here, to run once:

  //// Begin Serials ////
  Serial.begin(9600);
  Serial1.begin(9600);

   //// Begin LoRa Radio ////
  LoRa.setPins(4, 3, 2);
  LoRa.begin(433E6);
  LoRa.setTxPower(18);
  LoRa.sleep();

  initialPing();
  //// pinMode Define ////
  pinMode(SPIN, INPUT);
  slp.pwrDownMode();
  slp.sleepDelay(1000);
  Serial.println(F("System Ready"));

}

void loop() {
  if (mode == false)
  {
    Serial.println(F("Magnet mode"));
    enablePower(POWER_ADC);
    enablePower(POWER_SERIAL0);
    enablePower(POWER_SERIAL1);
    enablePower(POWER_SPI);
    enablePower(POWER_WIRE);

    if(digitalRead(0) == 0){
      Ping(tag, false, devtype);
    }else
    {
      Ping(tag, true, devtype);
    }

    disablePower(POWER_ADC);
    disablePower(POWER_SERIAL0);
    disablePower(POWER_SERIAL1);
    disablePower(POWER_SPI);
    disablePower(POWER_WIRE);

    slp.pwrDownMode();
    slp.sleepDelay(SleepDelay*1000);
  }
  if (mode == true)
  {
    Serial.println(F("Repeater mode"));
    Btime = 0;
    while (Btime <= 1800*1000)
    {
      int x  = LoRa.parsePacket();
      if (x)
      {
       struct ping{
       uint8_t ta;
       uint8_t devtyp;
       bool st;
      }p; 

        LoRa.readBytes((uint8_t*)&p, sizeof(p));
        Serial.println(p.ta);
        Serial.println(p.devtyp);
        Serial.println(p.st);
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&p, sizeof(p));
        LoRa.endPacket();
      }      
    }    
  } 
}