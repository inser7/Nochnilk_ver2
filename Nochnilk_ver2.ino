/*
  Скетч создан на основе FASTSPI2 EFFECTS EXAMPLES автора teldredge (www.funkboxing.com)
  А также вот этой статьи https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#cylon
  Доработан, переведён и разбит на файлы 2017 AlexGyver
  Смена выбранных режимов в случайном порядке через случайное время
*/

#include "FastLED.h"          // библиотека для работы с лентой
#include "OneButton.h"

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
const uint8_t VAR_alarm_MIN   = 30;           // будильник минуты
      uint8_t VAR_alarm_HOUR  = 7;           // будильник часы
      bool    VAR_alarm_FLAG1 = false;        // будильник разрешение работы
      bool    VAR_alarm_FLAG2 = false;       // будильник совершение действий (сигнализация)
      

#define LED_COUNT 30          // число светодиодов в кольце/ленте
#define LED_DT 13             // пин, куда подключен DIN ленты

int max_bright = 150;          // максимальная яркость (0 - 255)
boolean adapt_light = 0;       // адаптивная подсветка (1 - включить, 0 - выключить)

byte fav_modes[] = {2, 3, 4};  // список "любимых" режимов
byte num_modes = sizeof(fav_modes);         // получить количество "любимых" режимов (они все по 1 байту..)
unsigned long change_time, last_change, last_bright, startMillis;
int new_bright;

int ledMode = 4;
/*
  Стартовый режим
  0 - все выключены
  1 - все включены
  3 - кольцевая радуга
  888 - демо-режим
*/


OneButton button(3, true);

// цвета мячиков для режима
byte ballColors[3][3] = {
  {0xff, 0, 0},
  {0xff, 0xff, 0xff},
  {0   , 0   , 0xff}
};

// ---------------СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ-----------------
int BOTTOM_INDEX = 0;        // светодиод начала отсчёта
int TOP_INDEX = int(LED_COUNT / 2);
int EVENODD = LED_COUNT % 2;
struct CRGB leds[LED_COUNT];
int ledsX[LED_COUNT][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)

int thisdelay = 20;          //-FX LOOPS DELAY VAR
int thisstep = 10;           //-FX LOOPS DELAY VAR
int thishue = 0;             //-FX LOOPS DELAY VAR
int thissat = 255;           //-FX LOOPS DELAY VAR

int thisindex = 0;
int thisRED = 0;
int thisGRN = 0;
int thisBLU = 0;

int idex = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue = 0;                //-HUE (0-255)
int ibright = 0;             //-BRIGHTNESS (0-255)
int isat = 0;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
float tcount = 0.0;          //-INC VAR FOR SIN LOOPS
int lcount = 0;              //-ANOTHER COUNTING VAR
int x = 0;
// ---------------СЛУЖЕБНЫЕ ПЕРЕМЕННЫЕ-----------------

void setup()
{
  button.attachClick(randomChange);
  
  Serial.begin(9600);              // открыть порт для связи
  LEDS.setBrightness(max_bright);  // ограничить максимальную яркость

  LEDS.addLeds<WS2811, LED_DT, GRB>(leds, LED_COUNT);  // настрйоки для нашей ленты (ленты на WS2811, WS2812, WS2812B)
  one_color_all(0, 0, 0);          // погасить все светодиоды
  LEDS.show();                     // отослать команду
  randomSeed(analogRead(0));
}

void loop() {
  button.tick();
  tmElements_t tm;
  
  if (adapt_light) {                        // если включена адаптивная яркость
    if (millis() - last_bright > 500) {     // каждые полсекунды
      last_bright = millis();               // сброить таймер
      new_bright = map(analogRead(6), 1, 1000, 3, max_bright);   // считать показания с фоторезистора, перевести диапазон
      LEDS.setBrightness(new_bright);        // установить новую яркость
    }
  }

 if (millis() - last_change > 120000){ 
  if (RTC.read(tm)) {   
    if(tm.Hour >= 18 && tm.Hour < 20){
      LEDS.setBrightness(max_bright);
      ledMode = fav_modes[random(0, num_modes - 1)];
    }
    
    if(tm.Hour >= 20 && tm.Hour < 21){
       ledMode = 4;
       LEDS.setBrightness(100);
    }
    
    if(tm.Hour >= 21){
      ledMode = 110; // warm whtie
      LEDS.setBrightness(40);
      
      if(tm.Minute >= 30 ){
        ledMode = 110; // warm whtie
        LEDS.setBrightness(25);
      }
    }
    
    if(tm.Hour >= 22 && tm.Hour < 0 ){
       ledMode = 110; // warm whtie
       LEDS.setBrightness(20);
    }
    
    if(tm.Hour >= 0 && tm.Hour < 7 ){
        ledMode = 110; // warm white
        LEDS.setBrightness(10);
        Serial.println("Night");
    }

    if(tm.Hour >= 7 && tm.Hour < 8 ){
        LEDS.setBrightness(max_bright);
        if(tm.Minute < 30){
          ledMode = 104; //temp=104 // white = 1
        }
        else{
          ledMode = 102; //green
        }
        Serial.println("Morning");
    }

    if(tm.Hour >= 8 && tm.Hour < 9 ){
        LEDS.setBrightness(max_bright);
        ledMode = 101; //red
        Serial.println("Go to childrengarden");
    }
    
    if(tm.Hour >= 9 && tm.Hour < 18 ){
        LEDS.setBrightness(0);
        ledMode = 0;
        Serial.println("Do nothing");
    }
  }
  last_change = millis();
 }

//  if (millis() - last_change > change_time) {
//    change_time = random(5000, 20000);                // получаем новое случайное время до следующей смены режима
//    ledMode = fav_modes[random(0, num_modes - 1)];    // получаем новый случайный номер следующего режима
//    change_mode(ledMode);                             // меняем режим через change_mode (там для каждого режима стоят цвета и задержки)
//    last_change = millis();
//  }
 
    if (Serial.available() > 0) {     // если что то прислали
      ledMode = Serial.parseInt();    // парсим в тип данных int
      LEDS.setBrightness(max_bright);
      Serial.println(ledMode);
    }

    changeMode(ledMode);
 }

 void changeMode(int mode){
    switch (ledMode) {
      case 999: break;                           // пазуа
      case 0: one_color_all(0, 0, 0); LEDS.show(); break; //---ALL OFF
      case 1: one_color_all(255, 255, 255); LEDS.show(); break; //---ALL ON
      
      case 2: rainbowCycle(thisdelay); break;            // плавная смена цветов всей ленты
      case 3: rainbow_fade(60); break;            // крутящаяся радуга
      case 4: new_rainbow_loop(thisdelay);break;
  
      case 101: one_color_all(255, 0, 0); LEDS.show(); break; //---ALL RED
      case 102: one_color_all(0, 255, 0); LEDS.show(); break; //---ALL GREEN
      case 103: one_color_all(0, 0, 255); LEDS.show(); break; //---ALL BLUE
      case 104: one_color_all(255, 255, 0); LEDS.show(); break; //---ALL COLOR X
      case 105: one_color_all(0, 255, 255); LEDS.show(); break; //---ALL COLOR Y
      case 106: one_color_all(255, 0, 255); LEDS.show(); break; //---ALL COLOR Z
  
      case 107: one_color_all(255, 244, 229); LEDS.show(); break; //Warm Fluorescent
  
      case 108: one_color_all(255, 197, 143); LEDS.show(); break; //40W Tungsten
  
      case 109: one_color_all(255, 183, 76); LEDS.show(); break; //High Pressure Sodium
  
      case 110: one_color_all(255, 147, 41); LEDS.show(); break; // Candle
  }
}

void randomChange(){
    ledMode = fav_modes[random(0, num_modes - 1)]; 
    LEDS.setBrightness(max_bright);
    Serial.println(ledMode);
    changeMode(ledMode); // 2, 3, 4
}


///effects
void one_color_all(int cred, int cgrn, int cblu) {       //-SET ALL LEDS TO ONE COLOR
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGB( cred, cgrn, cblu);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}
void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}


//
void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;
  if (millis() - startMillis > SpeedDelay) {
    for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
      for (i = 0; i < LED_COUNT; i++) {
        c = Wheel(((i * 256 / LED_COUNT) + j) & 255);
        setPixel(i, *c, *(c + 1), *(c + 2));
      }
      FastLED.show();
    }
    startMillis = millis();
  }        
}

void rainbow_fade(int SpeedDelay) {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
  if (millis() - startMillis > SpeedDelay) {
    ihue++;
    if (ihue > 255) {
      ihue = 0;
    }
    for (int idex = 0 ; idex < LED_COUNT; idex++ ) {
      leds[idex] = CHSV(ihue, thissat, 255);
    }
    LEDS.show();
    startMillis = millis();
  }
}


void new_rainbow_loop(int SpeedDelay) {                      //-m88-RAINBOW FADE FROM FAST_SPI2
  if (millis() - startMillis > SpeedDelay) {
    ihue -= 1;
    fill_rainbow( leds, LED_COUNT, ihue );
    LEDS.show();
    startMillis = millis();
  }
}
