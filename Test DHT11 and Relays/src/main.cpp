#include <Arduino.h>
#include "DHT.h"

#define pinDHT D7
#define pinSw1 D3
#define pinSw2 D4
#define pinSw3 D5
#define pinSw4 D6

DHT dht(pinDHT, DHT11);
float suhu, kelembapan;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  pinMode(pinSw1, OUTPUT);
  pinMode(pinSw2, OUTPUT);
  pinMode(pinSw3, OUTPUT);
  pinMode(pinSw4, OUTPUT);
  digitalWrite(pinSw1, HIGH);
  digitalWrite(pinSw2, HIGH);
  digitalWrite(pinSw3, HIGH);
  digitalWrite(pinSw4, HIGH);
}

void loop()
{
  // put your main code here, to run repeatedly:
  suhu = dht.readTemperature();
  kelembapan = dht.readHumidity();
  Serial.print("Suhu = ");
  Serial.print(suhu);
  Serial.println(" C");
  Serial.print("Kelembapan = ");
  Serial.print(kelembapan);
  Serial.println(" %");

  if (suhu > 27.50)
  {
    digitalWrite(pinSw1, LOW);
    digitalWrite(pinSw2, LOW);
    digitalWrite(pinSw3, LOW);
    Serial.println("Kipas On");
    Serial.println("Mist Maker On");
  }
  else
  {
    digitalWrite(pinSw1, HIGH);
    digitalWrite(pinSw2, HIGH);
    digitalWrite(pinSw3, HIGH);
    Serial.println("Kipas Off");
    Serial.println("Mist Maker Off");
  }

  delay(1000);
}