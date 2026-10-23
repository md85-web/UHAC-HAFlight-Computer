#include "logger.h"

#include <SPI.h>
#include <SdFat.h>

#include "config.h"

namespace {
SdFat sd;
FsFile logFile;

constexpr const char *kHeader =
    "ms,ax,ay,az,gx,gy,gz,imu_c,press_pa,baro_c,alt_m,gps_fix,lat,lon,gps_alt_m,speed_kmh,course_deg,sats,utc";
}  // namespace

bool Logger::begin() {
  SPI1.setSCK(cfg::PIN_SD_SCK);
  SPI1.setTX(cfg::PIN_SD_MOSI);
  SPI1.setRX(cfg::PIN_SD_MISO);

  if (!sd.begin(SdSpiConfig(cfg::PIN_SD_CS, SHARED_SPI, SD_SCK_MHZ(4), &SPI1))) {
    sd.initErrorPrint(&Serial);  // explains why the card failed to start
    ok_ = false;
    return false;
  }

  // Pick the first unused filename so a reboot never overwrites data.
  bool found = false;
  for (int i = 0; i < 1000; ++i) {
    snprintf(name_, sizeof(name_), "flight_%03d.csv", i);
    if (!sd.exists(name_)) {
      found = true;
      break;
    }
  }
  if (!found) {
    ok_ = false;
    return false;
  }

  if (!logFile.open(name_, O_WRONLY | O_CREAT | O_AT_END)) {
    ok_ = false;
    return false;
  }

  logFile.println(kHeader);
  logFile.sync();
  Serial.printf("Logging to %s\n", name_);
  ok_ = true;
  return true;
}

void Logger::log(const Sample &s) {
  if (!ok_) return;

  // Print floats with print(value, decimals); printf("%f") is not supported on this core.
  logFile.print(static_cast<unsigned long>(s.ms));
  logFile.print(',');
  logFile.print(s.ax, 3);  logFile.print(',');
  logFile.print(s.ay, 3);  logFile.print(',');
  logFile.print(s.az, 3);  logFile.print(',');
  logFile.print(s.gx, 4);  logFile.print(',');
  logFile.print(s.gy, 4);  logFile.print(',');
  logFile.print(s.gz, 4);  logFile.print(',');
  logFile.print(s.imuTempC, 2);    logFile.print(',');
  logFile.print(s.pressurePa, 1);  logFile.print(',');
  logFile.print(s.baroTempC, 2);   logFile.print(',');
  logFile.print(s.altM, 2);        logFile.print(',');
  logFile.print(s.gpsFix ? 1 : 0); logFile.print(',');
  logFile.print(s.lat, 6);         logFile.print(',');
  logFile.print(s.lon, 6);         logFile.print(',');
  logFile.print(s.gpsAltM, 1);     logFile.print(',');
  logFile.print(s.speedKmph, 2);   logFile.print(',');
  logFile.print(s.courseDeg, 1);   logFile.print(',');
  logFile.print(s.sats);           logFile.print(',');
  if (s.utcValid) {
    char utc[10];
    snprintf(utc, sizeof(utc), "%02u:%02u:%02u", s.utcH, s.utcM, s.utcS);
    logFile.print(utc);
  }
  logFile.println();
  if (++rowsSinceFlush_ >= cfg::FLUSH_EVERY_N_ROWS) {
    logFile.sync();
    rowsSinceFlush_ = 0;
  }
}

void Logger::listFiles() {
  if (!ok_) {
    Serial.println("SD not available");
    return;
  }
  logFile.sync();  // make the current file's size accurate

  FsFile root;
  if (!root.open("/")) {
    Serial.println("Cannot open root");
    return;
  }
  FsFile f;
  while (f.openNext(&root, O_RDONLY)) {
    if (!f.isDir()) {
      char n[40];
      f.getName(n, sizeof(n));
      Serial.printf("%s  %lu bytes\n", n, static_cast<unsigned long>(f.fileSize()));
    }
    f.close();
  }
  root.close();
}

void Logger::dumpCurrent() {
  if (!ok_) {
    Serial.println("SD not available");
    return;
  }
  logFile.sync();

  FsFile r;
  if (!r.open(name_, O_RDONLY)) {
    Serial.println("Cannot open log file for reading");
    return;
  }
  Serial.printf("--- %s ---\n", name_);
  uint8_t buf[256];
  int n;
  while ((n = r.read(buf, sizeof(buf))) > 0) {
    Serial.write(buf, n);
  }
  r.close();
  Serial.println("--- end ---");
}