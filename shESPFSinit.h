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
#define ESP_FS_NAME "SPIFFS"
#elif defined(SH_USE_LITTLEFS)
#include <LittleFS.h>
#define ESP_FILESYSTEM LittleFS
#define ESP_FS_NAME "LittleFS"
#elif defined(SH_USE_FFAT) && defined(ARDUINO_ARCH_ESP32)
#include <FFat.h>
#define ESP_FILESYSTEM FFat
#define ESP_FS_NAME "FFat"
#else
#error "The method for storing files has not been defined"
#endif

// ==== SERIAL =======================================
// Макрос SH_LOG_OFF виден только в том TU, где он определён (обычно - в файле
// скетча), поэтому весь код вывода отладочных сообщений вынесен в этот
// заголовок: только так его можно исключить из компиляции.
// Реализация в .cpp не выводит ничего.
#if defined(SH_LOG_OFF)

#define EFI_SET_LOG_ON_STATE(x) (void)(x)
#define EFI_GET_LOG_ON_STATE false
#define EFI_SET_SERIAL_OBJECT(x) (void)(x)
#define EFI_PRINT(x)
#define EFI_PRINTLN(x)
#define EFI_PRINT_RESULT(x)

#else

#define EFI_SET_LOG_ON_STATE(x) logOnState = x
#define EFI_GET_LOG_ON_STATE logOnState
#define EFI_SET_SERIAL_OBJECT(x) \
  do                             \
  {                              \
    if (x)                       \
      serial = x;                \
  } while (0)
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
#define EFI_PRINT_RESULT(x)                       \
  {                                               \
    if (logOnState && serial)                     \
      serial->print((x) ? F("OK") : F("failed")); \
  }
#endif
// ==== shESPFSinit class ============================

class shESPFSinit
{
private:
  String labelFileName = "/fs";
  String fsName = ESP_FS_NAME;

  bool logOnState = true;
  Print *serial = &Serial;

  // статус записи/чтения метки файловой системы
  enum LabelStatus : uint8_t
  {
    LABEL_OK = 0,   // метка записана или прочитана успешно
    LABEL_NO_FILE,  // файл метки не удалось открыть
    LABEL_NO_WRITE  // файл открыт, но записан не полностью
  };

  // немые примитивы: только действия с файловой системой, без вывода
  LabelStatus writeLabel();
  LabelStatus readLabel(String &label);
  bool fs_format_silent();

  // вывод формируется в заголовке, где видно SH_LOG_OFF
  bool checkLabel();

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

  /**
   * @brief установка интерфейса для вывода отладочных сообщений
   *
   * @param _serial указатель на интерфейс
   */
  void setSerialObject(Print *_serial) { EFI_SET_SERIAL_OBJECT(_serial); }

  /**
   * @brief включение/отключение вывода информации о работе модуля через Serial
   *
   * @param _on
   */
  void setLogOnState(bool _on) { EFI_SET_LOG_ON_STATE(_on); }

  /**
   * @brief включен или отключен вывод информации о работе модуля через Serial
   *
   * @return true
   * @return false
   */
  bool getLogOnState() { return EFI_GET_LOG_ON_STATE; }
};

// ==== inline definitions ===========================
// Определение этих методов здесь, а не в .cpp, принципиально: именно они
// формируют вывод, и только в TU скетча видно SH_LOG_OFF.

inline bool shESPFSinit::checkLabel()
{
  if (!ESP_FILESYSTEM.exists(labelFileName))
  {
    // метки нет - создаем
    EFI_PRINT(F("Write label file..."));
    LabelStatus status = writeLabel();
    if (status == LABEL_NO_FILE)
    {
      EFI_PRINT(labelFileName);
      EFI_PRINT(F(" - failed to open file for writing"));
      EFI_PRINTLN();
    }
    EFI_PRINT_RESULT(status == LABEL_OK);
    return (status == LABEL_OK);
  }

  EFI_PRINT(F("Checking file system label..."));
  String label;
  LabelStatus status = readLabel(label);
  if (status != LABEL_OK)
  {
    EFI_PRINT(labelFileName);
    EFI_PRINT(F(" - failed to open file for reading"));
    EFI_PRINTLN();
    EFI_PRINT_RESULT(false);
    return (false);
  }

  bool result = (label == fsName);
  EFI_PRINT_RESULT(result);

  return (result);
}

inline bool shESPFSinit::init()
{
  EFI_PRINT(F("File system initialization..."));
  bool result = ESP_FILESYSTEM.begin();

  EFI_PRINT_RESULT(result);

  if (result)
  {
    EFI_PRINT(F("File system - "));
    EFI_PRINTLN(fsName);

    result = checkLabel();
  }

  if (!result)
  {
    EFI_PRINT(F("Formatting the file system..."));
    result = fs_format_silent();
    EFI_PRINT_RESULT(result);
#if defined(ARDUINO_ARCH_ESP32)
    if (result)
    {
      EFI_PRINTLN(F("The module will restart now"));
      EFI_PRINTLN();
      ESP.restart();
    }
#else
    if (result)
    {
      // примонтировать и записать метку
      result = ESP_FILESYSTEM.begin() && checkLabel();
    }
#endif
  }

  return (result);
}
