// definisi Blynk ID
#define BLYNK_TEMPLATE_ID "TMPLPOHPg5FZ"
#define BLYNK_DEVICE_NAME "SmartKumbung IoT"
#define BLYNK_AUTH_TOKEN "1mPa2OkBTkMPrFMjzWSl9zSi96z_FjGC"

// definisi Blynk Serial untuk kepentingan debugging
#define BLYNK_PRINT Serial
#define APP_DEBUG

// inisialisasi library yang digunakan
#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

// info login jaringan dan server blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "vivo1820";
char password[] = "sinta123";

// definisi pin wemos yang digunakan
#define pinDHT 13 // D7
#define pinTMPT6000 A0
#define pinSwFan 16        // D2
#define pinSwFanMist 2     // D9
#define pinSwMist 14       // D5
#define pinSwFanPeltier 12 // D6
#define pinSwPeltier 12    // D12
#define pinLED 0           // D8
#define pinBuzzer 15       // D10

// definisi virtual pin blynk yang digunakan
#define vPinSuhu V0
#define vPinKelembapan V1
#define vPinLedStatsFan V2
#define vPinLedStatMist V3
#define vPinBSuhu V4
#define vPinBKelembapan V5
#define vPinBBIntensCahayaMin V6
#define vPinIntensitasCahaya V7
#define vPinPeltierStat V8

// inisialisasi beberapa variabel yang digunakan
BlynkTimer timer;
DHT dht(pinDHT, DHT11);
LiquidCrystal_I2C lcd(0x27, 20, 4);

WidgetLED fanStats(vPinLedStatsFan);
WidgetLED mistStats(vPinLedStatMist);
WidgetLED peltStats(vPinPeltierStat);

int suhu, kelembapan, intensitasCahaya, kualitasSinyal;
int batasSuhu = 27;
int batasKelembapan = 80;
int batasBawahNilaiCahaya = 200;
int batasAtasNilaiCahaya = 600;
int nilaiPWM = 0;
int kecerahan;
int timingSuhu = 0;
int timingKelembapan = 0;
int timingSK = 0;
unsigned long waktuBerjalan, waktuBerjalanBlynk;
unsigned long waktuSebelum, waktuSebelumBlynk;
const unsigned long waktuJeda = 250;
const unsigned long waktuJedaBlynk = 50;

const int numReadings = 20;

int readings[numReadings]; // the readings from the analog input
int readIndex = 0;         // the index of the current reading
int total = 0;             // the running total

int inputPin = pinTMPT6000;

// fungsi kipas nyala
void fanOn()
{
  digitalWrite(pinSwFan, LOW);
  fanStats.on();
  Serial.println("Kipas On");
}

// fungsi kipas mati
void fanOff()
{
  digitalWrite(pinSwFan, HIGH);
  fanStats.off();
  Serial.println("Kipas Off");
}

// fungsi mist maker nyala
void mistOn()
{
  digitalWrite(pinSwFanMist, LOW);
  digitalWrite(pinSwMist, LOW);
  mistStats.on();
  Serial.println("Mist Maker On");
}

// fungsi mist maker mati
void mistOff()
{
  digitalWrite(pinSwFanMist, HIGH);
  digitalWrite(pinSwMist, HIGH);
  Serial.println("Mist Maker Off");
  mistStats.off();
}

// fungsi peltier nyala
void peltOn()
{
  peltStats.on();
  digitalWrite(pinSwFanPeltier, LOW);
  digitalWrite(pinSwPeltier, LOW);
  Serial.println("Peltier On");
}

// fungsi peltier mati
void peltOff()
{
  peltStats.off();
  digitalWrite(pinSwPeltier, HIGH);
  digitalWrite(pinSwFanPeltier, HIGH);
  Serial.println("Peltier Off");
}

// fungsi kirim data ke server blynk
void sendSensorData()
{
  // dapatkan data suhu dan kelembapan
  suhu = dht.readTemperature();
  kelembapan = dht.readHumidity();
  Blynk.virtualWrite(vPinSuhu, suhu);
  Blynk.virtualWrite(vPinKelembapan, kelembapan);
  Serial.print("% Temperature: ");
  Serial.print(suhu);
  Serial.println(" C");
  Serial.print("% Kelembaban: ");
  Serial.print(kelembapan);
  Serial.println(" %");
  lcd.setCursor(0, 0);
  lcd.print("Suhu/Lembab: ");
  lcd.print(suhu);
  lcd.print("C ");
  lcd.setCursor(17, 0);
  lcd.print(kelembapan);
  lcd.print("%");
  delay(250);

  // dapatkan data kualitas sinyal
  kualitasSinyal = WiFi.RSSI();
  lcd.setCursor(0, 3);
  lcd.print("Sinyal : ");
  lcd.print(kualitasSinyal);
  lcd.print(" RSSI");

  // tampilkan nilai intensitas cahaya
  Blynk.virtualWrite(vPinIntensitasCahaya, intensitasCahaya);
  Serial.print("% Intensitas Cahaya: ");
  Serial.print(intensitasCahaya);
  Serial.println(" lux");
  lcd.setCursor(0, 1);
  lcd.print("Cahaya : ");
  lcd.print(intensitasCahaya);
  lcd.print(" lux");
  Serial.print("% Nilai PWM: ");
  Serial.println(nilaiPWM);
  lcd.setCursor(0, 2);
  lcd.print("Lampu : ");
  lcd.print(kecerahan);
  lcd.print(" %");
  delay(250);

  // tampilkan nilai variabel lain ke serial monitor untuk debugging
  Serial.print("+ Batas Temperature: ");
  Serial.print(batasSuhu);
  Serial.println(" C");
  delay(250);
  Serial.print("+ Batas Kelembaban: ");
  Serial.print(batasKelembapan);
  Serial.println(" %");

  // desicion making ketika suhu diatas batas
  if (suhu > batasSuhu && kelembapan > batasKelembapan)
  {
    peltOn();
    fanOff();
    mistOff();
  }
  else if (suhu < batasSuhu && kelembapan < batasKelembapan)
  {
    fanOn();
    mistOn();
    peltOff();
  }
  else if (suhu == batasSuhu && kelembapan == batasKelembapan)
  {
    fanOff();
    mistOff();
    peltOff();
  }
  else if (suhu > batasSuhu)
  {
    peltOn();
    fanOff();
    mistOff();
  }
  else if (suhu < batasSuhu)
  {
    fanOn();
    peltOff();
    mistOff();
  }
  else if (kelembapan > batasKelembapan)
  {
    peltOn();
    fanOff();
    mistOff();
  }
  else if (kelembapan < batasKelembapan)
  {
    mistOn();
    peltOff();
    fanOff();
  }
}

// dapatkan input batas kelembapan dari blynk
BLYNK_WRITE(vPinBKelembapan)
{
  batasKelembapan = param.asInt();
}
// dapatkan input batas suhu dari blynk
BLYNK_WRITE(vPinBSuhu)
{
  batasSuhu = param.asInt();
}
// dapatkan input batas bawah intensitas cahaya dari blynk
BLYNK_WRITE(vPinBBIntensCahayaMin)
{
  batasBawahNilaiCahaya = param.asInt();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  Blynk.begin(auth, ssid, password);

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensorData);
  pinMode(pinSwFan, OUTPUT);
  pinMode(pinSwFanMist, OUTPUT);
  pinMode(pinSwMist, OUTPUT);
  pinMode(pinSwPeltier, OUTPUT);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinSwFan, HIGH);
  digitalWrite(pinSwFanMist, HIGH);
  digitalWrite(pinSwMist, HIGH);
  digitalWrite(pinSwPeltier, HIGH);

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  waktuBerjalan = millis();
  waktuBerjalanBlynk = millis();

  if (waktuBerjalanBlynk - waktuSebelumBlynk >= waktuJedaBlynk)
  {
    Blynk.run();
    timer.run();
    waktuSebelumBlynk = waktuBerjalanBlynk;
  }

  if (waktuBerjalan - waktuSebelum >= waktuJeda)
  {
    // dapatkan data intensitas cahaya
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = analogRead(inputPin);
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings)
    {
      // ...wrap around to the beginning:
      readIndex = 0;
    }

    // calculate the average:
    intensitasCahaya = total / numReadings;
    waktuSebelum = waktuBerjalan;
  }

  // desicion making nilai cahaya
  if (intensitasCahaya < batasBawahNilaiCahaya)
  {
    nilaiPWM++;
    if (nilaiPWM >= 255)
    {
      nilaiPWM = 255;
    }
  }
  else if (intensitasCahaya > batasBawahNilaiCahaya)
  {
    nilaiPWM--;
    if (nilaiPWM <= 0)
    {
      nilaiPWM = 0;
    }
  }

  // tulis nilai PWM ke LED
  analogWrite(pinLED, nilaiPWM);
  kecerahan = (nilaiPWM / 255) * 100;
}