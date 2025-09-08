#include <Arduino.h>
#include <TeensyTimerTool.h>
#include <cstring>
#include <SPI.h>
#include <SPISlave_T4.h>
// class SPISlave_T4: takes 2 parameters (port, bits/frame)
// void begin(); Initialize as SPI slave
// void onReceive(_SPI_ptr handler); takes function pointer type void(*)[]
   // interrupt for when receiving data from master
// void pushr(uint32_t data); writes data into TX buffer
// uint32_t popr(); reads received data from RX buffer
#define CS_PIN 10

volatile int idx = 0;

uint32_t my_data = 123456789; 
uint8_t tx_bytes[4] = {0xAB, 0xCD, 0xEF, 0xFF};
// uint8_t *tx_bytes = (uint8_t*)&my_data; //my_data in an array of bytes

SPISlave_T4<&SPI, SPI_8_BITS> tsyBSPI; //frame size of 8 bits/transfer

void response();

void setup() {
  tsyBSPI.begin();
  tsyBSPI.pushr(0x00);
  tsyBSPI.pushr(0x00);
  //For some reason SPISlave_T4 needs to preload 2 bytes in order to send data correctly
  tsyBSPI.onReceive(response);
}

void response() {
  if (tsyBSPI.available()) {
      uint8_t input = tsyBSPI.popr(); // reading data input from master
      tsyBSPI.pushr(tx_bytes[idx]); // refresh TX buffer with my_data once master pushes in 0x00's
      idx++;
      if (idx > 3) {idx = 0;}
  }
}

void loop() {
  ;
}

// Preloading just the first byte of bytes[4]
// Preloading all elements of bytes[4]
// 'Refilling' Slave TX array with each FIFO transmission to Master RX