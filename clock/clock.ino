#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

#define SET_BTN 4
#define HOUR_BTN 5
#define MIN_BTN 6

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)

TM1637Display display(CLK, DIO);

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

unsigned long debounce = 200UL;   // the debounce time, increase if the output flickers
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000;//1000           // interval at which to blink (milliseconds)
long time = 0;
long setCounter = 0;
unsigned long mil = 0;           // the last time the output pin was toggled

int mode = 0; //modes 0=clock, 1=set clock, 2=set wake time, 3=set night light?
int settingState = 0; //0=hour, 1=minutes

int modeButtonState = 0;
int modeButtonPrev = 0;
int hrButtonState = 0;
int hrButtonPrev = 0;
int minButtonState = 0;
int minButtonPrev = 0;

void setup() {
  Serial.begin(9600);

  pinMode(SET_BTN, INPUT);
  pinMode(HOUR_BTN, INPUT);
  pinMode(MIN_BTN, INPUT);

  display.clear();
  display.setBrightness(0); //0-7
}

void loop() {

  if (buttonHandler(SET_BTN, modeButtonState, modeButtonPrev)) {
    mode = mode + 1 > 3 ? 0 : mode + 1;
    Serial.println(mode);
  }

  if (mode == 0) {
    counting();
  } else if (mode == 1) {
    setting();
  }
}

void setting() {
  if (buttonHandler(MIN_BTN, minButtonState, minButtonPrev)) {
    if (settingState == 0) {
      settingState = 1;
    } else if (settingState == 1) {
      settingState = 0;
      mode = 0;
    }
  }
  
  if (settingState == 0) {
    if (buttonHandler(HOUR_BTN, hrButtonState, hrButtonPrev)) {
      time += SECS_PER_HOUR;
    }
    showSettingTimeHR();
  } else if (settingState == 1) {
    if (buttonHandler(HOUR_BTN, hrButtonState, hrButtonPrev)) {
      time += SECS_PER_MIN;
    }
    showSettingTimeMIN();
  }

}

void showSettingTimeHR() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval / 4) {
    previousMillis = currentMillis;

    int minutes = numberOfMinutes(time);
    int hours = numberOfHours(time);
    bool pm = hours > 12;
    int hours12 = pm ? hours - 12 : hours;  // for 12 hour clock
    hours12 = hours12 == 0 ? 12 : hours12; //set 0 to 12

    if (setCounter % 2 == 0) {
      data[0] = hours12 / 10 > 0 ? display.encodeDigit(hours12 / 10) : 0x00;
      data[1] = display.encodeDigit(hours12 % 10) | 0x80;
      data[2] = display.encodeDigit(minutes / 10);
      data[3] = display.encodeDigit(minutes % 10);
      display.setSegments(data);
    } else {
      data[0] = 0x00;
      data[1] = 0x80;
      data[2] = display.encodeDigit(minutes / 10);
      data[3] = display.encodeDigit(minutes % 10);
      display.setSegments(data);
    }

    setCounter++;
  }
}

void showSettingTimeMIN() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval / 4) {
    previousMillis = currentMillis;

    int minutes = numberOfMinutes(time);
    int hours = numberOfHours(time);
    bool pm = hours > 12;
    int hours12 = pm ? hours - 12 : hours;  // for 12 hour clock
    hours12 = hours12 == 0 ? 12 : hours12; //set 0 to 12

    if (setCounter % 2 == 0) {
      data[0] = hours12 / 10 > 0 ? display.encodeDigit(hours12 / 10) : 0x00;
      data[1] = display.encodeDigit(hours12 % 10) | 0x80;
      data[2] = display.encodeDigit(minutes / 10);
      data[3] = display.encodeDigit(minutes % 10);
      display.setSegments(data);
    } else {
      data[0] = hours12 / 10 > 0 ? display.encodeDigit(hours12 / 10) : 0x00;
      data[1] = display.encodeDigit(hours12 % 10) | 0x80;
      data[2] = 0x00;
      data[3] = 0x00;
      display.setSegments(data);
    }

    setCounter++;
  }
}

void counting() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int minutes = numberOfMinutes(time);
    int hours = numberOfHours(time);
    bool pm = hours > 12;
    int hours12 = pm ? hours - 12 : hours;  // for 12 hour clock
    hours12 = hours12 == 0 ? 12 : hours12; //set 0 to 12

    data[0] = hours12 / 10 > 0 ? display.encodeDigit(hours12 / 10) : 0x00;
    data[1] = display.encodeDigit(hours12 % 10) | (time % 2 ? 0x80 : 0x00);
    data[2] = display.encodeDigit(minutes / 10);
    data[3] = display.encodeDigit(minutes % 10); //TODO am/pm
 
    display.setSegments(data);
    
    time++;

    if (time > SECS_PER_DAY) {
      time = 0;
    }
  }
}

bool buttonHandler(int pin, int state, int prev) {
  state = digitalRead(pin);

  if (state == HIGH && prev == LOW && millis() - mil > debounce)
  {
    mil = millis();
    return true;
  } else {
    return false;
  }

  prev = state;
}
