#include <Arduino.h>

#include "config.h"
#include "logger.h"
#include "sensors.h"

static Sensors sensors;
static Logger logger;
static uint32_t lastLogMs = 0;
static uint32_t lastPrintMs = 0;

// Human-readable once-per-second report.
static void printReport(const Sample &s) {
  Serial.println("========================================");
  Serial.printf("Time since boot: %lu ms   SD: %s\n", static_cast<unsigned long>(s.ms),
                logger.ok() ? "logging" : "NOT LOGGING");
  Serial.print("MPU6050  accel ");
  Serial.print(s.ax, 3);  Serial.print(' ');
  Serial.print(s.ay, 3);  Serial.print(' ');
  Serial.print(s.az, 3);  Serial.print(" m/s^2  gyro ");
  Serial.print(s.gx, 3);  Serial.print(' ');
  Serial.print(s.gy, 3);  Serial.print(' ');
  Serial.print(s.gz, 3);  Serial.print(" rad/s  temp ");
  Serial.print(s.imuTempC, 2);
  Serial.println(" C");

  Serial.print("BMP085   temp ");
  Serial.print(s.baroTempC, 2);
  Serial.print(" C  pressure ");
  Serial.print(s.pressurePa, 0);
  Serial.print(" Pa  altitude ");
  Serial.print(s.altM, 2);
  Serial.println(" m");

  if (s.gpsFix) {
    Serial.printf("GPS      sats %u  ", s.sats);
    Serial.print("lat ");   Serial.print(s.lat, 6);
    Serial.print("  lon "); Serial.print(s.lon, 6);
    Serial.print("  alt "); Serial.print(s.gpsAltM, 2);
    Serial.print(" m  speed "); Serial.print(s.speedKmph, 2);
    Serial.print(" km/h  course "); Serial.print(s.courseDeg, 2);
    Serial.println(" deg");
  } else {
    Serial.printf("GPS      NO FIX (sats %u)\n", s.sats);
  }
  if (s.utcValid) Serial.printf("UTC      %02u:%02u:%02u\n", s.utcH, s.utcM, s.utcS);
}

// Bench-only serial commands:  l = list files,  d = dump current log.
// Dumping blocks logging while it runs, so don't use it in flight.
static void handleSerialCommands() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'l') {
      logger.listFiles();
    } else if (c == 'd') {
      logger.dumpCurrent();
    }
  }
}

void setup() {
  Serial.begin(cfg::SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(cfg::PIN_MOSFET, OUTPUT);
  digitalWrite(cfg::PIN_MOSFET, LOW);  // keep the high-current switch off

  // Wait briefly for a USB serial monitor on the bench, but never block a real flight.
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {
  }

  Serial.println("Flight computer booting");

  sensors.begin();
  Serial.printf("IMU: %s  Baro: %s\n", sensors.imuOk() ? "ok" : "MISSING",
                sensors.baroOk() ? "ok" : "MISSING");

  logger.begin();
  Serial.printf("SD:  %s\n", logger.ok() ? "ok" : "MISSING");
  Serial.println("Commands: l = list files, d = dump current log");
}

void loop() {
  handleSerialCommands();
  sensors.pollGps();

  uint32_t now = millis();
  if (now - lastLogMs < cfg::LOG_INTERVAL_MS) return;
  lastLogMs = now;

  Sample s;
  sensors.read(s);
  logger.log(s);

  if (now - lastPrintMs >= cfg::PRINT_INTERVAL_MS) {
    lastPrintMs = now;
    printReport(s);
  }

  digitalWrite(LED_BUILTIN, (now / 500) % 2);  // heartbeat
}