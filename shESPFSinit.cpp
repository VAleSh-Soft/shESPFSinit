#include <Arduino.h>
#include "shESPFSinit.h"

// ===================================================

#ifdef ESP_FS_INIT_LOG_OFF

#define EFI_PRINT(x)
#define EFI_PRINTLN(x)

#else

#define EFI_PRINT(x)          \
  {                           \
    if (logOnState && serial) \
      serial->print(x);       \
  }
#define EFI_PRINTLN(x)        \
  {                           \
    if (logOnState && serial) \
      serial->println(x);     \
  }
#endif
// ===================================================

static const char FORMATTING_FS[] PROGMEM = "Formatting the file system...";
static const char WRITE_LABEL[] PROGMEM = "Write label file...";
static const char CHECKING_FS_LAB[] PROGMEM = "Checking file system label...";
static const char FS_INIT[] PROGMEM = "File system initialization...";
static const char FS_NAME[] PROGMEM = "File system - ";
static const char UNKN_FS[] PROGMEM = "Unsupported file system specified";
static const char NO_WRITE[] PROGMEM = " - failed to open file for writing";
static const char NO_READ[] PROGMEM = " - failed to open file for reading";
static const char RESTART[] PROGMEM = "The module will restart now";
static const char FAILED_STR[] PROGMEM = "failed";
static const char OK_STR[] PROGMEM = "OK";

// ==== public =======================================

shESPFSinit::shESPFSinit() {}

bool shESPFSinit::init()
{
#if defined(SH_USE_SPIFFS)
  fsName = "SPIFFS";
#elif defined(SH_USE_LITTLEFS)
  fsName = "LittleFS";
#elif defined(SH_USE_FFAT) && defined(ARDUINO_ARCH_ESP32)
  fsName = "FFat";
#else
  EFI_PRINTLN(FPSTR(UNKN_FS));
  return (false);
#endif

  EFI_PRINT(FPSTR(FS_INIT));
  bool result = ESP_FILESYSTEM.begin();

  printResult(result);

  if (result)
  {
    EFI_PRINT(FPSTR(FS_NAME));
    EFI_PRINTLN(fsName);

    result = fs_check();
  }

  if (!result)
  {
    result = fs_format();
#if !defined(ARDUINO_ARCH_ESP32)
    if (result)
    {
      // примонтировать и записать метку
      result = ESP_FILESYSTEM.begin() && fs_check();
    }
#endif
  }

  return (result);
}

void shESPFSinit::setLabelFileName(const char *_path)
{
  // копируем строку во избежание висячего указателя
  labelFileName = String(_path ? _path : "/fs");
  if (labelFileName[0] != '/')
  {
    labelFileName = "/" + labelFileName;
  }
}

String shESPFSinit::getLabelFileName() { return labelFileName; }

#ifndef ESP_FS_INIT_LOG_OFF
void shESPFSinit::setSerialObject(Print *_serial)
{
  if (_serial)
  {
    serial = _serial;
  }
}

void shESPFSinit::setLogOnState(bool _on)
{
  logOnState = _on;
}

bool shESPFSinit::getLogOnState() { return logOnState; }
#endif
// ==== private ======================================

void shESPFSinit::printResult(bool result)
{
#ifndef ESP_FS_INIT_LOG_OFF
  String res = (result) ? FPSTR(OK_STR) : FPSTR(FAILED_STR);
  EFI_PRINTLN(res);
#endif
}

bool shESPFSinit::fs_check()
{
  bool result = ESP_FILESYSTEM.exists(labelFileName);
  if (!result)
  {
    EFI_PRINT(FPSTR(WRITE_LABEL));
    result = writeLabel(labelFileName.c_str());
    printResult(result);
  }
  else
  {
    EFI_PRINT(FPSTR(CHECKING_FS_LAB));
    String x = readLabel(labelFileName.c_str());
    result = (x == fsName);
    printResult(result);
  }

  return (result);
}

bool shESPFSinit::fs_format()
{
  EFI_PRINT(FPSTR(FORMATTING_FS));
  ESP_FILESYSTEM.end();
  bool result = ESP_FILESYSTEM.format();
  printResult(result);
#if defined(ARDUINO_ARCH_ESP32)
  if (result)
  {
    EFI_PRINTLN(FPSTR(RESTART));
    EFI_PRINTLN();
    ESP.restart();
  }
#endif

  return (result);
}

bool shESPFSinit::writeLabel(const char *_path)
{
#if defined(ARDUINO_ARCH_ESP32)
  File file = ESP_FILESYSTEM.open(_path, "w", true);
#else
  File file = ESP_FILESYSTEM.open(_path, "w");
#endif
  if (!file)
  {
    EFI_PRINT(_path);
    EFI_PRINTLN(FPSTR(NO_WRITE));
    return (false);
  }

  bool result = (file.write((const uint8_t *)fsName.c_str(),
                            fsName.length()) == fsName.length());

  file.close();

  return (result);
}

String shESPFSinit::readLabel(const char *path)
{
#if defined(ARDUINO_ARCH_ESP32)
  File file = ESP_FILESYSTEM.open(path, "r", false);
#else
  File file = ESP_FILESYSTEM.open(path, "r");
#endif
  if (!file || file.isDirectory())
  {
    EFI_PRINT(path);
    EFI_PRINTLN(FPSTR(NO_READ));
    if (file)
    {
      file.close();
    }
    return (String());
  }

  // метка короткая - читаем не более 16 байт
  char buf[17];
  int len = file.readBytes(buf, sizeof(buf) - 1);
  if (len < 0)
  {
    len = 0;
  }
  buf[len] = '\0';
  file.close();

  // обрезаем по первому пробельному символу
  char *p = strpbrk(buf, " \n\r\t");
  if (p)
  {
    *p = '\0';
  }

  return (String(buf));
}
