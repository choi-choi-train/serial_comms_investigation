#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <DMAChannel.h>
#include "imxrt.h"

#define BUFFER_SIZE 4
#define DATA_REGISTER (*(uint8_t*)IMXRT_LPUART6_ADDRESS)

using namespace std;
using namespace TeensyTimerTool;

PeriodicTimer t1(TCK_RTC); 

// ======= DMA HOUSEKEEPING =======
DMAChannel rx_dma;
DMAChannel tx_dma;
uint8_t rx_buffer[BUFFER_SIZE]; // word size of 4 bytes sent between DMA buffers
uint8_t tx_buffer[BUFFER_SIZE]; // word size of 4 bytes sent between DMA buffers

// ======= DATA COLLECTION =======
int ping_counter = 0;
uint32_t recorded_times[10000];
uint32_t tx_data = 123456789;

void polling();
void ISR();

void analyze_results() {
    uint32_t total_time = 0;
    for (int t=0; t<10000; t++) {
        total_time += recorded_times[t];
    }
    float mean = static_cast<float>(total_time/10000.f);

    float sigma = 0;
    for (int t=0; t<10000; t++) {
        sigma += ((recorded_times[t]-mean)*(recorded_times[t]-mean));
    }

    float standard_dev = sqrt(sigma/9999.f);

    Serial.println("\n==== 10k PING SEND RESULTS ====");
    Serial.printf("Total Time: %.6fsec\n", static_cast<float>(total_time/1000000.0));
    Serial.printf("Mean Time/Exchange: %f\n", mean);
    Serial.printf("Standard Deviation: %f\n", standard_dev);

    delay(1000);
    ping_counter = 0;
    t1.begin(polling, 100);
}

void polling() {
    uint32_t initial_time = micros();

    // prepare RX DMA
    rx_dma.destinationBuffer(rx_buffer, BUFFER_SIZE);
    rx_dma.transferCount(BUFFER_SIZE);
    rx_dma.enable();
    Serial.println("Ready to receive data from RX register...");

    // begin TX DMA
    memcpy(tx_buffer, &tx_data, BUFFER_SIZE);
    tx_dma.sourceBuffer(tx_buffer, BUFFER_SIZE);
    tx_dma.transferCount(BUFFER_SIZE);
    tx_dma.enable();
    Serial.println("Sending DMA to TX register...");

    while (!rx_dma.complete());
    rx_dma.clearComplete();

    // READ DATA FROM DMA
    uint32_t rx_data = *(uint32_t*) rx_buffer;
    Serial.printf("DATA RECEIVED from RX DMA: %d\n", rx_data);

    uint32_t elapsed_time = micros() - initial_time;
    
    recorded_times[ping_counter] = elapsed_time;
    ping_counter++;

    if (ping_counter >= 10000) {
        t1.stop();
        analyze_results();
    }
}

void ISR() {
  ;
}

void setup() {
  Serial.begin(115200);
  Serial3.begin(115200);

  // UART RX DMA
  rx_dma.source(DATA_REGISTER);
  rx_dma.destinationBuffer(rx_buffer, BUFFER_SIZE);
  rx_dma.transferSize(1);

// UART TX DMA
  tx_dma.sourceBuffer(&tx_data, BUFFER_SIZE);
  tx_dma.destination(DATA_REGISTER); 
  tx_dma.transferSize(1);

  while (!Serial);
  Serial.println("TEST START");
  t1.begin(polling, 100);
  attachErrFunc(ErrorHandler(Serial));
}

void loop() {
  ;
}