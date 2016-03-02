
#include "LoRaCoAP.h"
#include <SPI.h>
#include "DHT.h"

#define DEEP_DEBUG 1
LoraShield lora;

CoAPServer CS;
CoAPToken* tk = new CoAPToken(); 


#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only

//  lora.setFreq(FREQ_8680);
  Serial.println("I'm here"); 

  CS.begin(lora, "dzeta.t.eu.org"); 
    lora.setContikiDebug(true);
  lora.setFreq(FREQ_8681);
  lora.setBandwidth(BW_MIN);

    dht.begin();

}

uint16_t mid = random(0xFFFF); 


void loop()
{
  float t1 = dht.readTemperature();
  String temp = String((int) t1); 
  
  Serial.print ("temp = "); Serial.println (temp);
  
  CS.setHeader(COAP_TYPE_NON, COAP_METHOD_POST, mid, tk); 
  CS.addOption(COAP_OPTION_URI_PATH, "test");
  CS.addOption(COAP_END_OPTION);
  CS.addValue(temp, COAP_FORMAT_TEXT); 
  CS.endMessage(); 
  
  mid++; 
  
  delay (3000); 
  
  Serial.println("Response");
  while (lora.dataAvailable()) {
    byte b; 
    
    digitalWrite(SS_PIN,LOW);
    //  READ BYTE CMD
    int previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    int shield_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    b = SPI.transfer(ARDUINO_CMD_AVAILABLE);
    delayMicroseconds(WAIT_TIME_BETWEEN_BYTES_SPI);
    digitalWrite(SS_PIN,HIGH);
    delayMicroseconds(WAIT_TIME_BETWEEN_SPI_MSG);
    
    Serial.print (b, HEX);
    Serial.print ("-");
  }
  Serial.println("fini");
    
  delay(200000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}
