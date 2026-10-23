#pragma once
#include <Arduino.h>

#include "sensors.h"

// Writes one CSV file per boot (flight_000.csv, flight_001.csv, ...) to the SD card (SdFat on SPI1).
class Logger {
 public:
  bool begin();
  void log(const Sample &s);
  bool ok() const { return ok_; }

  // Bench helpers, driven by serial commands.
  void listFiles();   // print every file on the card with its size
  void dumpCurrent(); // print the current log file over serial

 private:
  bool ok_ = false;
  uint8_t rowsSinceFlush_ = 0;
  char name_[24] = "";
};