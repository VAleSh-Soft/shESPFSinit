#include <Arduino.h>
#include "shESPFSinit.h"

// ===================================================
// Здесь только действия с файловой системой.
// Вывод отладочных сообщений формируется в shESPFSinit.h,
// чтобы макрос SH_LOG_OFF из скетча реально исключал код и строки.
// ==== public =======================================

shESPFSinit::shESPFSinit() {}

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

// ==== private ======================================
// Немые примитивы: только действия с файловой системой, без вывода.
// Вывод (если включён) формируется в заголовке, где видно SH_LOG_OFF.

shESPFSinit::LabelStatus shESPFSinit::writeLabel()
{
#if defined(ARDUINO_ARCH_ESP32)
  File file = ESP_FILESYSTEM.open(labelFileName, "w", true);
#else
  File file = ESP_FILESYSTEM.open(labelFileName, "w");
#endif
  if (!file) return LABEL_NO_FILE;

  bool ok = (file.write((const uint8_t *)fsName.c_str(),
                        fsName.length()) == fsName.length());

  file.close();

  return ok ? LABEL_OK : LABEL_NO_WRITE;
}

shESPFSinit::LabelStatus shESPFSinit::readLabel(String &label)
{
#if defined(ARDUINO_ARCH_ESP32)
  File file = ESP_FILESYSTEM.open(labelFileName, "r", false);
#else
  File file = ESP_FILESYSTEM.open(labelFileName, "r");
#endif
  if (!file || file.isDirectory())
  {
    if (file)
    {
      file.close();
    }
    return LABEL_NO_FILE;
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

  label = String(buf);

  return LABEL_OK;
}

bool shESPFSinit::fs_format_silent()
{
  ESP_FILESYSTEM.end();
  return ESP_FILESYSTEM.format();
}
