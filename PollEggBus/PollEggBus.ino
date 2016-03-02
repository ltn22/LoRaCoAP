#include <stdint.h>
#include <SPI.h>
#include "LoRaCoAP.h"


#include "Wire.h"
#include "EggBus.h"

EggBus eggBus;
LoraShield lora;
CoAPServer CS; 

float Results[10];
int   idxRes = 0;

String answer_co2 (CoAPResource* res, uint8_t format)
{
      Serial.println("GET requested"); 
      
     return String((long)Results[0]) + " " + String((long)Results[1]);
}

void setup(){
  Serial.begin(9600);
  
  CS.begin (lora, "theta.t.eu.org");
  CS.addRes(String("/co2"), answer_co2); 
  
}

void loop(){
  uint8_t   egg_bus_address;
  float x_scaler = 0.0;
  float y_scaler = 0.0;
  float sensor_resistance;
  uint32_t r0 = 0;
  
  eggBus.init();
  while((egg_bus_address = eggBus.next())){
    Serial.println("===========================");
    Serial.print("Egg Bus Address: 0x");
    Serial.println(egg_bus_address, HEX);

    Serial.print("Firmware Version: ");
    Serial.println(eggBus.getFirmwareVersion(), DEC);

    Serial.print("  Sensor Address: ");
    printAddress(eggBus.getSensorAddress());             
    
    uint8_t numSensors = eggBus.getNumSensors();
    idxRes=0;
    for(uint8_t ii = 0; ii < numSensors; ii++){
    
      Serial.println("---------------------------");
      Serial.print("  Sensor Index: ");
      Serial.println(ii, DEC);       
      
      Serial.print("    Sensor Type: ");
      Serial.println(eggBus.getSensorType(ii));
     
      Serial.print("  Table X Scaler: ");
      x_scaler = eggBus.getTableXScaler(ii);
      Serial.println(x_scaler, 8);     
     
      Serial.print("  Table Y Scaler: ");
      y_scaler = eggBus.getTableYScaler(ii);
      Serial.println(y_scaler, 8);         

      Serial.print("              R0: ");
      r0 = eggBus.getSensorR0(ii);
      Serial.println(r0);
      
      sensor_resistance = eggBus.getSensorResistance(ii);
      
      Serial.print("      Resistance: ");
      Serial.println(sensor_resistance, 3);
      
      Serial.print("            R/R0: ");
      Serial.println(sensor_resistance/r0, 3);
      
      uint8_t xval, yval, row = 0;
      while(eggBus.getTableRow(ii, row++, &xval, &yval)){
        Serial.print("     Table Row ");
        Serial.print(row);
        Serial.print(": [");
        Serial.print(xval, DEC);
        Serial.print(", ");        
        Serial.print(yval, DEC);        
        Serial.print("] => [");
        Serial.print(x_scaler * xval, 8);
        Serial.print(", ");
        Serial.print(y_scaler * yval, 8);
        Serial.println("]");
      }
      
      Serial.print("    Sensor Value: ");
      Serial.println(eggBus.getSensorValue(ii), DEC);    
      Results[idxRes++] = eggBus.getSensorValue(ii);
      
      Serial.print("    Sensor Units: ");
      Serial.println(eggBus.getSensorUnits(ii));            
    }
  }
  CS.incoming();
  
  Serial.print ("Results "); 
  for (int i = 0; i <idxRes; i++) {
    Serial.print (Results[i]);
    Serial.print (" ");
  }
  Serial.println();
  
  delay(5000);
}

void printAddress(uint8_t * address){
  for(uint8_t jj = 0; jj < 6; jj++){
    if(address[jj] < 16) Serial.print("0");
    Serial.print(address[jj], HEX);
    if(jj != 5 ) Serial.print(":");
  }
  Serial.println();
}
