#include "sensors.h"

#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>
#include <Wire.h>

#include "config.h"

namespace {
Adafruit_MPU6050 mpu;
Adafruit_BMP085 bmp;
TinyGPSPlus gps;
}  // namespace

bool Sensors::begin() {
  Wire.setSDA(cfg::PIN_I2C_SDA);
  Wire.setSCL(cfg::PIN_I2C_SCL);
  Wire.begin();

  // Settings carried over from the original bench script.
  imuOk_ = mpu.begin(0x68, &Wire);
  if (imuOk_) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  baroOk_ = bmp.begin(BMP085_STANDARD, &Wire);

  Serial1.setRX(cfg::PIN_GPS_RX);
  Serial1.setTX(cfg::PIN_GPS_TX);
  Serial1.begin(cfg::GPS_BAUD);

  return imuOk_ && baroOk_;
}

void Sensors::pollGps() {
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }
}

void Sensors::read(Sample &s) {
  s.ms = millis();

  if (imuOk_) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    s.ax = a.acceleration.x;
    s.ay = a.acceleration.y;
    s.az = a.acceleration.z;
    s.gx = g.gyro.x;
    s.gy = g.gyro.y;
    s.gz = g.gyro.z;
    s.imuTempC = t.temperature;
  }

  if (baroOk_) {
    s.baroTempC = bmp.readTemperature();
    s.pressurePa = static_cast<float>(bmp.readPressure());
    s.altM = bmp.readAltitude(cfg::SEA_LEVEL_HPA * 100.0f);
  }

  s.gpsFix = gps.location.isValid();
  if (s.gpsFix) {
    s.lat = gps.location.lat();
    s.lon = gps.location.lng();
  }
  if (gps.altitude.isValid()) s.gpsAltM = gps.altitude.meters();
  if (gps.speed.isValid()) s.speedKmph = gps.speed.kmph();
  if (gps.course.isValid()) s.courseDeg = gps.course.deg();
  if (gps.satellites.isValid()) s.sats = gps.satellites.value();

  s.utcValid = gps.date.isValid() && gps.time.isValid();
  if (s.utcValid) {
    s.utcH = gps.time.hour();
    s.utcM = gps.time.minute();
    s.utcS = gps.time.second();
  }
}
