// TEENSY B
#include <Arduino.h>
#include <TeensyTimerTool.h>
using namespace TeensyTimerTool;

int counter = 0;
int last_recorded_time = 0;

PeriodicTimer t1(TCK_RTC);

void callback() {
  if (Serial3.available() >= 4) {
    int response = 1234;
    Serial3.write((char*)&response, 4);
    Serial3.flush();
  }
}

void setup() {
  Serial3.begin(115200);
  t1.begin(callback, 100);
}

void loop() {
  ;
}