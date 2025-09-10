#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <DMAChannel.h>
#include "imxrt.h"

#define BUFFER_SIZE 4
#define DATA_REGISTER (*(uint8_t*)IMXRT_LPUART6_ADDRESS)

using namespace TeensyTimerTool;
using namespace std;

PeriodicTimer t1(TCK_RTC);

// ======= DMA HOUSEKEEPING =======
DMAChannel rx_dma;
DMAChannel tx_dma;
uint8_t rx_buffer[BUFFER_SIZE]; // word size of 4 bytes sent between DMA buffers
uint8_t tx_buffer[BUFFER_SIZE]; // word size of 4 bytes sent between DMA buffers

void polling() {
  // prepare RX DMA
  rx_dma.destinationBuffer(rx_buffer, BUFFER_SIZE);
  rx_dma.transferCount(BUFFER_SIZE);
  rx_dma.enable();

  while (!rx_dma.complete());
  Serial.println("Received a ping!");
  rx_dma.clearComplete();
  
  // begin TX DMA
  tx_dma.transferCount(BUFFER_SIZE);
  tx_dma.enable();
}

void ISR() {
  ;
}

void setup() {
  Serial3.begin(115200);
  
  // UART RX DMA
  rx_dma.source(DATA_REGISTER);
  rx_dma.destinationBuffer(rx_buffer, BUFFER_SIZE);
  rx_dma.transferSize(1);

  // UART TX DMA
  tx_dma.sourceBuffer(rx_buffer, BUFFER_SIZE); //TX reads directly from DMA and makes ping echo
  tx_dma.destination(DATA_REGISTER); 
  tx_dma.transferSize(1);

  while (!Serial);
  t1.begin(polling, 100);
  attachErrFunc(ErrorHandler(Serial));
}

void loop() {
  ;
}