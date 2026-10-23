#pragma once
#include <Arduino.h>

namespace cfg {

// Serial / timing
constexpr uint32_t SERIAL_BAUD        = 115200;
constexpr uint32_t LOG_INTERVAL_MS    = 50;    // 20 Hz
constexpr uint32_t PRINT_INTERVAL_MS  = 1000;
constexpr uint8_t  FLUSH_EVERY_N_ROWS = 20;    // about once per second at 20 Hz

// I2C (MPU6050 + BMP085)
constexpr uint8_t PIN_I2C_SDA = 4;
constexpr uint8_t PIN_I2C_SCL = 5;

// GPS (NEO-6M on UART0)
constexpr uint8_t  PIN_GPS_RX = 1;   // Pico RX <- GPS TX
constexpr uint8_t  PIN_GPS_TX = 0;   // Pico TX -> GPS RX
constexpr uint32_t GPS_BAUD   = 9600;

// SD card on SPI1
constexpr uint8_t PIN_SD_SCK  = 10;
constexpr uint8_t PIN_SD_MOSI = 11;
constexpr uint8_t PIN_SD_MISO = 12;
constexpr uint8_t PIN_SD_CS   = 13;

// High-current switch (kept LOW). Change if you wire this to another pin.
constexpr uint8_t PIN_MOSFET = 22;

// Barometric altitude reference (hPa). Set to local sea-level pressure.
constexpr float SEA_LEVEL_HPA = 1013.25f;

}  // namespace cfg