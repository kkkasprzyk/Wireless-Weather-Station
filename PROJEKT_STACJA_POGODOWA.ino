 //// PROJEKT STACJI POGODOWEJ ///
//Autorzy :
//Krzysztof Kasprzyk
//Rafał Piwowarczyk 


// Biblioteki których używamy do naszych czujników oraz ogólnego działania kodu projektu 
#include <IRremote.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
#include <IRremote.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <MQ135.h>
#include "DHT.h"
///czujnik deszczu wartosci skrajne 
// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum
const int buzzer=10; //PWM
int zmienna = 0;
// Piny zdefiniowane nazwami 
#define DHT11_PIN 7 // czujnik wilgotności i temperatury 

// Zdefiniowane Piny do obsługi I2C pod wyświetlacz i czujnik BMP280
#define BMP_SCK 13  
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 8 
#define PIN_MQ135 A2  // czujnik ciśnienia i temperatury
int RECV_PIN = 5; // Pilot IR

// Inicjalizacja czujników specjalnymi dyrektywami zawartymi w manualach do bibliotek
DHT dht;
LiquidCrystal_PCF8574 lcd(0x27);
IRrecv irrecv(RECV_PIN);
decode_results results;
MQ135 mq135_sensor(PIN_MQ135);
//Adafruit_BMP280 bme; // I2C
Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
int show = -1;
int co2=0;  
/// Wartosci potrzebne do kalibracji czujników
float temperature = 21.0; // Assume current temperature. Recommended to measure with DHT22
float humidity = 25.0; // Assume current humidity. Recommended to measure with DHT22//analog feed from MQ135
  
  
void setup()
{
  //buzer 

  analogWrite(buzzer,0); 
pinMode(buzzer,OUTPUT);
 Serial.begin(9600);  // Komunikacja UART z komputerem , głównie wykorzystwana do testów czujników 

  // Inicjalizacja czujników 
 dht.setup(DHT11_PIN);
  bme.begin();
 irrecv.enableIRIn(); 
  display_init();
  lcd.setBacklight(255);
  lcd.print("Stacja Pogodowa");
  }

void loop() {   //// petla glowna 
  // Kalibracja czujnika bmp280
  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
  float resistance = mq135_sensor.getResistance();
  float ppm = mq135_sensor.getPPM();
  float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

  //Pobranie informacji o wilgotnosci
  int wilgotnosc = dht.getHumidity();
  int wilg_ok,temp_ok;
  //Pobranie informacji o temperaturze
  int temperatura = dht.getTemperature();
  
  if (dht.getStatusString() == "OK") {
    wilg_ok=wilgotnosc;
    Serial.print(wilgotnosc);
   Serial.print("%RH | ");
    //temp_ok=temperatura;
  //  Serial.print(temperatura);
  //  Serial.println("*C");
  }
  
  //Odczekanie wymaganego czasu dla czujnika DHT 11
  delay(dht.getMinimumSamplingPeriod());

  ////////Deszcz ALERT  ////// 

int dsc= analogRead(A0); //dsc -> 0 do 1023
//Serial.println(dsc);
    if(dsc<300){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Schowaj Stacje");
    lcd.setCursor(1,1);
    lcd.print("Pada Deszcz !");
    analogWrite(buzzer,75);
    delay(1500);
}
else{
    analogWrite(buzzer,0); 
}
  delay(200);  // delay between reads

  /////

irrecv.decode(&results);
 
 //Serial.println(results.value, HEX);  // wyświetlanie wartości HEX aby sprawdzić poszczególne przyciski naszego pilota 
 
    /// Całość obsługi za pomocą pilota IR oparliśmy na "switch case " aby wszytsko miało w miare przejrzysty wyglad i proste dodawanie funkcji dodatkowych
     //////// konczy sie switch case /////////////////////////////////////////////////////////////////////

    
      if(results.value==0xFFA25D){ // klawisz 1 
        digitalWrite(9, LOW); 
      lcd.clear(); 
      lcd.print("Jakosc powietrza");
            if(correctedPPM < 500){
              lcd.setCursor(5,1);
    lcd.print("Dobra");
    } else if(correctedPPM > 500 && correctedPPM <= 1000){
      lcd.setCursor(6,1);
      lcd.print("Zla");
    } else if(correctedPPM > 1000){
      lcd.setCursor(2,1);
      lcd.print("Bardzo zla"); 
      }
      }
      if(results.value==0xFF629D){

        lcd.clear();
        lcd.setCursor(3,0);
      lcd.print("Cisnienie:");
      //Serial.print(bme.readPressure()/100);
      lcd.setCursor(3,1);
      lcd.print(bme.readPressure()/100);
      lcd.print("hPa");
      }
      
      if(results.value==0xFFE21D){
        lcd.clear();
        lcd.setCursor(2,0);
      lcd.print("Temperatura:");
      lcd.setCursor(4,1);
      lcd.print(bme.readTemperature());
       lcd.print((char)223);
      lcd.print("C");
      }
      
      if(results.value==0xFF22DD){
        lcd.clear();
      lcd.setBacklight(255);
      lcd.print("Wilgotnosc");
      lcd.setCursor(0,1);
      lcd.print("Powietrza ");
      lcd.print(wilg_ok);
      lcd.print("%RH");
      }
  if(results.value==0xFF02FD){  // klawisz 5
    lcd.clear();
       int war= analogRead(A3);
       lcd.setCursor(3,0);
      lcd.print("Pora Dnia:");
      lcd.setCursor(1,0);
      if(war<50){
         lcd.setCursor(5,1);
        lcd.print("Noc");
        
        }
      else {
        lcd.setCursor(5,1);
        lcd.print("Dzien");   
        }
      

    }

if(results.value==0xFFC23D){
    lcd.clear();
       int glb = analogRead(A1);
       Serial.println(glb);
       glb = map(glb, 200, 742, 100, 0);
       lcd.setCursor(0,0);
      lcd.print("Wilgotnosc Gleby");
      lcd.setCursor(6,1);
      if(glb<0){
        lcd.print("0");
        }
        else{\
      lcd.print(glb);
        }
      lcd.print("%");
}
        
    //////// konczy sie switch case /////////////////////////////////////////////////////////////////////
  irrecv.resume();  // kontynuuuj odbieranie sygnałów IR z pilota


}


/// Funkcja uruchamiająca wyświetlacz i sprawdzająca czy napewno działa prawidłwo
void display_init(){
   int error;

  Serial.begin(9600);
  //Serial.println("LCD...");

  // wait on Serial to be available 
  while (!Serial)
    ;

  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
 // Serial.print("Error: ");
  //Serial.print(error);

  if (error == 0) {
    show = 0;
    lcd.begin(16, 2); // initialize the lcd

  } else {
  } // if

  }
