#pragma once

#include <FS.h>
#include <Arduino.h>

#if !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_ESP8266)
#error "The library is designed for use in the ESP8266 or ESP32 environment"
#endif

// ==== ESP_FILESYSTEM================================

#if !defined(SH_USE_SPIFFS) && !defined(SH_USE_LITTLEFS) && !defined(SH_USE_FFAT)
#if defined(ARDUINO_ARCH_ESP32)
#define SH_USE_FFAT // доступна только для esp32
#else
#define SH_USE_LITTLEFS
#endif
#endif

#if defined(SH_USE_SPIFFS)
#if defined(ARDUINO_ARCH_ESP32)
#include <SPIFFS.h>
#endif
#define ESP_FILESYSTEM SPIFFS
#elif defined(SH_USE_LITTLEFS)
#include <LittleFS.h>
#define ESP_FILESYSTEM LittleFS
#elif defined(SH_USE_FFAT) && defined(ARDUINO_ARCH_ESP32)
#include <FFat.h>
#define ESP_FILESYSTEM FFat
#else
#error "The method for storing files has not been defined"
#endif

// =====================================================

#ifdef SH_LOG_OFF
#define ESP_FS_INIT_LOG_OFF
#endif

// ==== shESPFSinit class ============================

class shESPFSinit
{
private:
  String labelFileName = "/fs";
  String fsName;
#ifndef ESP_FS_INIT_LOG_OFF
  bool logOnState = true;
  Print *serial = &Serial;
#endif

  void printResult(bool result);
  bool fs_check();
  bool fs_format();
  bool writeLabel(const char *_path);
  String readLabel(const char *path);

public:
  /**
   * @brief конструктор
   *
   */
  shESPFSinit();

  /**
   * @brief инициализация файловой системы
   *
   * @return true
   * @return false
   */
  bool init();

  /**
   * @brief установка нового имени файловой метки (по умолчанию "/fs")
   *
   * @param _path новое имя файла
   */
  void setLabelFileName(const char *_path);

  /**
   * @brief получение текущего имени файловой метки
   *
   * @return String
   */
  String getLabelFileName();

#ifndef ESP_FS_INIT_LOG_OFF
/**
 * @brief установка интерфейса для вывода отладочных сообщений
 * 
 * @param _serial указатель на интерфейс
 */
  void setSerialObject(Print *_serial);

  /**
   * @brief включение/отключение вывода информации о работе модуля через Serial
   *
   * @param _on
   */
  void setLogOnState(bool _on);

  /**
   * @brief включен или отключен вывод информации о работе модуля через Serial
   *
   * @return true
   * @return false
   */
  bool getLogOnState();
#endif
};
