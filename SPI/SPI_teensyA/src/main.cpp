#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <SPI.h>
#define CS_PIN 7
// beginTransaction(): initializes SPI bus after SPI.begin() is called, with settings applied
// endTransaction(): stops using the SPI bus, basically CS_PIN low
// end(): disables SPI bus
// transfer(): 
//     transfer16(): transfer() but for two bytes at a time
//     transfer(buffer, size): bulk transfer, sending a data array
//     with [size] elements. But must be an array of bytes.
//     After call finishes, buffer is overwritten with received
//     data.

using namespace TeensyTimerTool;
using namespace std;

PeriodicTimer ttt_loop(TCK_RTC);
SPISettings s(14000000, MSBFIRST, SPI_MODE0);

int counter = 0;
vector<int> recorded_times(10000);

void callBack();

void loop() {
  ;
}

void analyze_results() {
  int total_time = 0;
  for (int t : recorded_times) {
    total_time += t;
  }
  float mean = static_cast<float>(total_time/10000.f);
  
  float sigma = 0;
  for (int t : recorded_times) {
    sigma += ((t-mean)*(t-mean));
  }
  
  float standard_dev = sqrt(sigma/9999.f);
  
  Serial.println("\n==== 10k PING SEND RESULTS ====");
  Serial.printf("Total Time: %.6fsec\n", static_cast<float>(total_time/1000000.0));
  Serial.printf("Mean Time/Exchange: %f\n", mean);
  Serial.printf("Standard Deviation: %f\n", standard_dev);
  
  delay(1000);
  counter = 0;
  ttt_loop.begin(callBack, 100);
}

void setup() {
  pinMode(CS_PIN, OUTPUT);
  Serial.begin(115200);
  while(!Serial);

  SPI.begin();
  ttt_loop.begin(callBack, 100);
}

void callBack() {
  uint8_t rx_buffer[4];
  //Duplex exchanging 4 bytes
  int start_time = micros();
  
  digitalWriteFast(CS_PIN, LOW);
  SPI.beginTransaction(s);
  SPI.transfer(nullptr, rx_buffer, 4);
  SPI.endTransaction();
  digitalWriteFast(CS_PIN, HIGH);

  int elapsed_time = micros() - start_time;
  Serial.printf("%lu || %lu || %lu || %lu\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3]);
  uint32_t res = (uint32_t(rx_buffer[0]) << 24 |
            uint32_t(rx_buffer[1]) << 16 |
            uint32_t(rx_buffer[2]) << 8 |
            uint32_t(rx_buffer[3]));
  // Serial.printf("%lu\n", res);

  recorded_times.at(counter) = elapsed_time;
  counter++;

  if (counter >= 10000) {
    ttt_loop.stop();
    analyze_results();
  }
}

// CS_PIN toggle per byte
// CS_PIN toggle once over all 4 bytes in buffer

// Not working...Theories:
// Needs more delay between transfers to allow ISR to refill its data
// Issue when running with TeensyTimerTool
// Issue with 