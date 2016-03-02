#include "LoRaCoAP.h"
#include <SPI.h>

#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

LoraShield lora;

CoAPServer CS;
CoAPResource* resTmp;

String answer_get (CoAPResource* res, uint8_t format)
{
      Serial.println("GET requested");
     
     return String(666);
}


void setup()
{
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

  CS.begin(lora, "dzeta.t.eu.org");
  lora.setContikiDebug(true);
  lora.setFreq(FREQ_8681);
  lora.setBandwidth(BW_MIN);
  resTmp = CS.addRes(String("/tmp"), answer_get);
}

bool led = false;

void loop()
{
  CS.incoming();
  delay(100);
  
}

