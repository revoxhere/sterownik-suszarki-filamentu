// Sterownik suszarki filamentu
// 05.2023 piotrowsky.dev
// wersja 1.0

#include <Arduino.h>
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8X8_SSD1306_64X48_ER_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);

#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;

#include "ESPRotary.h"
#define ROTARY_PIN1  D0
#define ROTARY_PIN2 D5
#define BUTTON_PIN  A0
#define CLICKS_PER_STEP   4

#define GRZALKA D7
#define WENTYLATOR D6

#include "Ticker.h"
Ticker t;
ESPRotary r;

#include <Preferences.h>
Preferences prefs;

byte wilgotnosc = 0;
byte temperatura = 0;
byte old_wilgotnosc = 0;
byte old_temperatura = 0;

int zadana_wilgotnosc = 0;
int zadana_temperatura = 0;
int zadana_wilgotnosc_2 = 0;
int zadana_temperatura_2 = 0;

byte edycja = 0;

void setup()   {
  Serial.begin(115200);

  pinMode(GRZALKA, OUTPUT);
  pinMode(WENTYLATOR, OUTPUT);
  digitalWrite(GRZALKA, LOW);
  digitalWrite(WENTYLATOR, LOW);

  prefs.begin("suszarka");
  zadana_temperatura = prefs.getInt("zadana_temperatura", 0);
  zadana_wilgotnosc = prefs.getInt("zadana_wilgotnosc", 0);
  zadana_temperatura_2 = prefs.getInt("zadana_temperatura_2", 0);
  zadana_wilgotnosc_2 = prefs.getInt("zadana_wilgotnosc_2", 0);

  u8x8.begin();
  while (! aht.begin()) {
    Serial.println("Nie znaleziono czujnika AHT");

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f  );
    u8x8.setCursor(0, 0);
    u8x8.print("Brak");
    u8x8.setCursor(0, 18);
    u8x8.print("czujnika");

    delay(1000);
  }
  Serial.println("Czujnik AHT znaleziony");

  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  t.attach_ms(6, handleLoop);
}

void rotate(ESPRotary& r) {
  if (edycja == 1) {
    zadana_wilgotnosc = r.getPosition();
  } else if (edycja == 2) {
    zadana_temperatura = r.getPosition();
  } else if (edycja == 3) {
    zadana_wilgotnosc_2 = r.getPosition();
  } else if (edycja == 4) {
    zadana_temperatura_2 = r.getPosition();
  }
}

void showDirection(ESPRotary& r) {
  Serial.println(r.directionToString(r.getDirection()));
}

void click() {
  edycja += 1;

  if (edycja == 1 || edycja == 3) {
    r.setUpperBound(100);
    r.setLowerBound(0);
  } else if (edycja == 2 || edycja == 4) {
    r.setUpperBound(80);
    r.setLowerBound(-40);
  }

  if (edycja == 1) {
    r.resetPosition(zadana_wilgotnosc);
  } else if (edycja == 2) {
    r.resetPosition(zadana_temperatura);
  } else if (edycja == 3) {
    r.resetPosition(zadana_wilgotnosc_2);
  } else if (edycja == 4) {
    r.resetPosition(zadana_temperatura_2);
  }
}

long lastClick = 0;
void handleLoop() {
  r.loop();
  if (analogRead(A0) < 512 && millis() - lastClick > 500) {
    click();
    lastClick = millis();
  }
}

void loop() {
  if (edycja == 5) {

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f );
    u8x8.setCursor(0, 0);
    u8x8.print("ZAPIS");
    u8x8.setCursor(0, 2);
    u8x8.print("USTAWIEN");
    u8x8.setCursor(0, 4);
    delay(400);
    prefs.putInt("zadana_wilgotnosc", zadana_wilgotnosc);
    prefs.putInt("zadana_temperatura", zadana_temperatura);
    prefs.putInt("zadana_wilgotnosc_2", zadana_wilgotnosc_2);
    prefs.putInt("zadana_temperatura_2", zadana_temperatura_2);
    u8x8.print("OK");
    delay(300);
    edycja = 0;

  } else if (edycja == 1) {

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f );
    u8x8.setCursor(0, 0);
    u8x8.print("WYJSCIE1");
    u8x8.setFont(u8x8_font_7x14_1x2_f );
    u8x8.setCursor(0, 2);
    u8x8.print("Wilgotn.");
    u8x8.setCursor(0, 4);
    u8x8.print(">");
    u8x8.print(zadana_wilgotnosc);
    u8x8.print("%");

  } else if (edycja == 2) {

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f );
    u8x8.setCursor(0, 0);
    u8x8.print("WYJSCIE1");
    u8x8.setCursor(0, 2);
    u8x8.setFont(u8x8_font_7x14_1x2_f );
    u8x8.print("Temp.");
    u8x8.setCursor(0, 4);
    u8x8.print(">");
    u8x8.print(zadana_temperatura);
    u8x8.print("*C");

  } else if (edycja == 3) {

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f );
    u8x8.setCursor(0, 0);
    u8x8.print("WYJSCIE2");
    u8x8.setCursor(0, 2);
    u8x8.setFont(u8x8_font_7x14_1x2_f );
    u8x8.print("Wilgotn.");
    u8x8.setCursor(0, 4);
    u8x8.print(">");
    u8x8.print(zadana_wilgotnosc_2);
    u8x8.print("%");

  } else if (edycja == 4) {

    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_f );
    u8x8.setCursor(0, 0);
    u8x8.print("WYJSCIE2");
    u8x8.setCursor(0, 2);
    u8x8.setFont(u8x8_font_7x14_1x2_f );
    u8x8.print("Temp.");
    u8x8.setCursor(0, 4);
    u8x8.print(">");
    u8x8.print(zadana_temperatura_2);
    u8x8.print("*C");

  } else {

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    temperatura = temp.temperature;
    wilgotnosc = humidity.relative_humidity;

    if (temperatura != old_temperatura || wilgotnosc != old_wilgotnosc) {
      u8x8.clear();
      u8x8.setFont(u8x8_font_courB18_2x3_f  );
      u8x8.setCursor(0, 0);
      u8x8.print(temperatura);
      u8x8.print("*C");
      u8x8.setCursor(0, 3);
      u8x8.print(wilgotnosc);
      u8x8.print("%");

      old_wilgotnosc = wilgotnosc;
      old_temperatura = temperatura;
    }

    if (wilgotnosc > zadana_wilgotnosc && temperatura > zadana_temperatura) {
      digitalWrite(GRZALKA, HIGH);
    } else {
      digitalWrite(GRZALKA, LOW);
    }

    if (wilgotnosc > zadana_wilgotnosc_2 && temperatura > zadana_temperatura_2) {
      digitalWrite(WENTYLATOR, HIGH);
    } else {
      digitalWrite(WENTYLATOR, LOW);
    }

  }

  delay(100);
}
