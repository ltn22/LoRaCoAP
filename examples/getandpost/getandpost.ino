
#include <LoRaCoAP.h>
#include <SPI.h>

LoraShield lora;

CoAPServer CS;
CoAPResource* resLum;
CoAPResource* resMsg;

String answer_get (CoAPResource* res, uint8_t format)
{
      Serial.println("GET requested"); 
      
     return String(666);
}

uint8_t answer_msg  (CoAPResource* res, uint8_t format, uint8_t method, String input)
{
      Serial.println("PUT requested"); 
      
      Serial.println (input);
      
      return ERROR_CODE(2,04);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

  CS.begin(lora, "dzeta.t.eu.org"); 
  
   resLum = CS.addRes(String("/lum"), answer_get);
   resMsg = CS.addRes(String("/msg"), answer_msg);
#ifdef DUMP_COAP
   CS.listRes();
#endif 
}

bool led = false;

void loop()
{
  CS.incoming();
  delay(100);
}
