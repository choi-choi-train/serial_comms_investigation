#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <DMAChannel.h>
#include "imxrt.h"

#define BUFFER_SIZE 4
#define DATA_REGISTER *(volatile uint8_t*)IMXRT_LPUART3_ADDRESS

using namespace TeensyTimerTool;
using namespace std;

PeriodicTimer t1(TCK_RTC);

// ======= DMA HOUSEKEEPING =======
DMAChannel rx_dma;
DMAChannel tx_dma;
uint32_t rx_buffer; // word size of 4 bytes sent between DMA buffers
uint32_t tx_buffer; // word size of 4 bytes sent between DMA buffers

void ISR() {
  ;
}

void polling() {
  // prepare RX DMA
  rx_dma.destinationBuffer(&rx_buffer, BUFFER_SIZE);
  rx_dma.transferCount(1);
  rx_dma.enable();

  while (!rx_dma.complete());
  rx_dma.clearComplete();

  // begin TX DMA
  tx_dma.sourceBuffer(&tx_buffer, BUFFER_SIZE); // ECHO directly from DMA buffer

  // uint32_t tx_data = *(uint32_t*) rx_buffer; 
  // tx_dma.sourceBuffer(&tx_data, BUFFER_SIZE); // ECHO by reading from RX buffer and writing to TX buffer

  tx_dma.transferCount(1);
  tx_dma.enable();
}

void setup() {
  Serial.begin(115200);

  // UART TX DMA
  tx_dma.sourceBuffer(&tx_buffer, BUFFER_SIZE); // allocate a buffer in Ram
  tx_dma.destination(DATA_REGISTER); // set UART TX data register as destination
  tx_dma.transferCount(BUFFER_SIZE); // send 4 bytes total
  tx_dma.transferSize(4); //bytes to be sent per transfer

  // UART RX DMA
  rx_dma.triggerAtHardwareEvent(DMAMUX_SOURCE_LPUART3_RX);
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