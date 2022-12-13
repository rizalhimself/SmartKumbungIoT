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
#define pinDHT D7
#define pinTMPT6000 A0
#define pinSwFan D2
#define pinSwFanMist D9
#define pinSwMist D5
#define pinSwPeltier D6
#define pinLED D8

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

int suhu, kelembapan, intensitasCahaya;
int batasSuhu = 27;
int batasKelembapan = 80;
int batasBawahNilaiCahaya = 200;
int batasAtasNilaiCahaya = 600;
int nilaiPWM = 0;
unsigned long waktuBerjalan;
unsigned long waktuSebelum;
const unsigned long waktuJeda = 250;

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
  digitalWrite(pinSwPeltier, LOW);
  Serial.println("Peltier On");
}

// fungsi peltier mati
void peltOff()
{
  peltStats.off();
  digitalWrite(pinSwPeltier, HIGH);
  Serial.println("Peltier Off");
}

// fungsi kirim data ke server blynk
void sendSensorData()
{
  // dapatkan data suhu
  suhu = dht.readTemperature();
  Blynk.virtualWrite(vPinSuhu, suhu);
  Serial.print("% Temperature: ");
  Serial.print(suhu);
  Serial.println(" C");
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(suhu);
  lcd.print(" C");
  delay(250);

  // dapatkan data kelembapan
  kelembapan = dht.readHumidity();
  Blynk.virtualWrite(vPinKelembapan, kelembapan);
  Serial.print("% Kelembaban: ");
  Serial.print(kelembapan);
  Serial.println(" %");
  lcd.setCursor(0, 1);
  lcd.print("Lembab : ");
  lcd.print(kelembapan);
  lcd.print(" %");
  delay(250);

  // tampilkan nilai intensitas cahaya
  Blynk.virtualWrite(vPinIntensitasCahaya, intensitasCahaya);
  Serial.print("% Intensitas Cahaya: ");
  Serial.print(intensitasCahaya);
  Serial.println(" lux");
  lcd.setCursor(0, 2);
  lcd.print("Cahaya : ");
  lcd.print(intensitasCahaya);
  lcd.print(" lux");
  Serial.print("% Nilai PWM: ");
  Serial.println(nilaiPWM);
  lcd.setCursor(0, 3);
  lcd.print("Lampu : ");
  int kecerahan = (nilaiPWM / 255) * 100;
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
  if (suhu > batasSuhu)
  {
    fanOn();
    waktuBerjalan++;
    if (waktuBerjalan > 15)
    {
      peltOn();
    }
  }
  // desicion making ketika kelembapan diatas batas
  else if (suhu == batasSuhu && kelembapan > batasKelembapan)
  {
    fanOff();
    peltOn();
    waktuBerjalan = 0;
  }
  // desicion making ketika tidak dalam kondisi kedua diatas
  else if (suhu == batasSuhu && kelembapan == batasKelembapan)
  {
    fanOff();
    peltOff();
    waktuBerjalan = 0;
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
}

void loop()
{
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();

  waktuBerjalan = millis();
  if (waktuBerjalan - waktuSebelum >= waktuJeda)
  {
    // dapatkan data intensitas cahaya
    intensitasCahaya = analogRead(pinTMPT6000);
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
  else if (intensitasCahaya > batasAtasNilaiCahaya)
  {
    nilaiPWM--;
    if (nilaiPWM <= 0)
    {
      nilaiPWM = 0;
    }
  }

  // tulis nilai PWM ke LED
  analogWrite(pinLED, nilaiPWM);
}