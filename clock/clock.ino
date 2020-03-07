#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)

TM1637Display display(CLK, DIO);

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;           // interval at which to blink (milliseconds)
long time = 0;

int mode = 0; //modes 0=clock, 1=set clock, 2=set wake time, 3=set night light?
int setting = 0; //1=hour, 2=minutes

void setup() {
  Serial.begin(9600);

  display.clear();
  display.setBrightness(0); //0-7
}

void loop() {
  counting();
}

void counting() {
     
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int minutes = numberOfMinutes(time);
    int hours = numberOfHours(time);

    int hours12 = hours > 12 ? hours - 12 : hours;  // for 12 hour clock
    hours12 = hours12 == 0 ? 12 : hours12; //set 0 to 12

    data[0] = hours12 / 10 > 0 ? display.encodeDigit(hours12 / 10) : 0x00;
    data[1] = display.encodeDigit(hours12 % 10) | (time % 2 ? 0x80 : 0x00);
    data[2] = display.encodeDigit(minutes / 10);
    data[3] = display.encodeDigit(minutes % 10); //TODO am/pm
 
    display.setSegments(data);

    //display.showNumberDecEx(199, 0b11100000);
    
    time++;

    if (time > SECS_PER_DAY) {
      time = 0;
    }
  }
}
