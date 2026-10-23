#pragma once
#include <Arduino.h>

// One row of telemetry. Fields stay 0/false if their sensor is missing.
struct Sample {
  uint32_t ms = 0;
  float ax = 0, ay = 0, az = 0;     // m/s^2
  float gx = 0, gy = 0, gz = 0;     // rad/s
  float imuTempC = 0;
  float pressurePa = 0;
  float baroTempC = 0;
  float altM = 0;                   // barometric, from cfg::SEA_LEVEL_HPA
  bool gpsFix = false;
  double lat = 0, lon = 0;
  float gpsAltM = 0;
  float speedKmph = 0;
  float courseDeg = 0;
  uint8_t sats = 0;
  bool utcValid = false;
  uint8_t utcH = 0, utcM = 0, utcS = 0;
};

class Sensors {
 public:
  // Returns true if every sensor came up; individual flags below tell which.
  bool begin();

  // Call every loop iteration so GPS NMEA bytes are drained promptly.
  void pollGps();

  // Take a fresh reading from IMU, barometer and latest GPS data.
  void read(Sample &out);

  bool imuOk() const { return imuOk_; }
  bool baroOk() const { return baroOk_; }

 private:
  bool imuOk_ = false;
  bool baroOk_ = false;
};
