#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <DMAChannel.h>
#include "imxrt.h"

#define BUFFER_SIZE 4
#define DATA_REGISTER *(volatile uint8_t*)IMXRT_LPUART3_ADDRESS //dereferenced first 8 bits of TX REGISTER, located at IMXRT_LPUART6

using namespace std;
using namespace TeensyTimerTool;

PeriodicTimer t1(TCK_RTC); 

// ======= DMA HOUSEKEEPING =======
DMAChannel rx_dma;
DMAChannel tx_dma;
uint32_t rx_buffer; // word size of 4 bytes sent between DMA buffers
uint32_t tx_buffer; // word size of 4 bytes sent between DMA buffers

// ======= DATA COLLECTION =======
int ping_counter = 0;
uint32_t recorded_times[10000];

void polling();
void ISR();

void analyze_results() {
    uint32_t total_time = 0;
    for (int t=0; 10000; t++) {
        total_time += recorded_times[t];
    }
    float mean = static_cast<float>(total_time/10000.f);

    float sigma = 0;
    for (int t=0; t<sizeof(recorded_times); t++) {
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
    uint32_t tx_data = 123456789;

    int initial_time = micros();

    // prepare RX DMA
    rx_dma.destinationBuffer(&rx_buffer, BUFFER_SIZE);
    rx_dma.transferCount(1);
    rx_dma.enable();

    // begin TX DMA
    tx_dma.sourceBuffer(&tx_buffer, BUFFER_SIZE);
    tx_dma.transferCount(1);
    tx_dma.enable();

    while (!rx_dma.complete()); // WAIT FOR RESPONSE
    rx_dma.clearComplete();

    // READ DATA FROM DMA
    uint32_t rx_data = *(uint32_t*) rx_buffer;

    float elapsed_time = micros() - initial_time;
    
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

// UART TX DMA
  tx_dma.sourceBuffer(&tx_buffer, BUFFER_SIZE); // allocate a buffer in RAM
  tx_dma.destination(DATA_REGISTER); // set UART TX data register as the destination.
  tx_dma.transferCount(BUFFER_SIZE); // send 4 bytes total
  tx_dma.transferSize(4); // bytes to be sent per transfer

// UART RX DMA
  rx_dma.triggerAtHardwareEvent(DMAMUX_SOURCE_LPUART3_RX); // handles response, DMA stuffs data into RX register
  rx_dma.source(DATA_REGISTER);
  rx_dma.transferSize(4);

  while (!Serial);
  t1.begin(polling, 100);
  attachErrFunc(ErrorHandler(Serial));

  rx_dma.enable();
  tx_dma.enable();
}

void loop() {
  ;
}