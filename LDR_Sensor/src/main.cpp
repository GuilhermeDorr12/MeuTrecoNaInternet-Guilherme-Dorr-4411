#include <Arduino.h>

int pinoLed = 13; 
int pinoSensorLuz = 34;               
int valorLuz = 0;

void setup() {
  pinMode(pinoLed,OUTPUT); 
}

void loop() {
  valorLuz = analogRead(pinoSensorLuz);        
  if(valorLuz<750)
  {                
    digitalWrite(pinoLed,LOW);
  }
  else
  {                    
    digitalWrite(pinoLed,HIGH);
  }
  delay(10);
}