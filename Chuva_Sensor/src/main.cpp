#include <Arduino.h>

int Led = 13;
int sensor_chuva = 34;
int chuva = 0;

void setup() {
  pinMode(Led, OUTPUT);
}

void loop() {
  chuva = analogRead(sensor_chuva);        
  if(chuva<750)
  {                
    digitalWrite(Led,LOW);
  }
  else
  {                    
    digitalWrite(Led,HIGH);
  }
  delay(10);  
}