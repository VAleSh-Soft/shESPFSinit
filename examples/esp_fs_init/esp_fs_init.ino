/**
 * @file esp_fs_init.ino
 * @author Vladimir Shatalov (valesh-soft@yandex.ru)
 * 
 * @brief Пример использования библиотеки - инициализация и последующее 
 *        использование файловой системы.
 * 
 *        Экземпляр файловой системы устанавливается библиотекой - ESP_FILESYSTEM.
 *        После инициализации его можно использовать в обычном порядке:
 *        для открытия, создания, поиска, удаления файлов и т.д.
 * 
 * @version 1.0
 * @date 22.11.2026
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <shESPFSinit.h>

shESPFSinit esp_fs;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // инициализация файловой системы
  if (esp_fs.init())
  {
    String labelFileName = esp_fs.getLabelFileName();
    Serial.print(F("File label name: "));
    Serial.println(labelFileName);

    // открываем файл с меткой файловой системы (по умолчанию "/fs")
    File file = ESP_FILESYSTEM.open(labelFileName, "r");
    if (file)
    {
      // и считываем его содержимое
      Serial.print(F("The file label contains: "));
      Serial.println(file.readString());
      file.close();
    }
    else
    {
      Serial.println(F("The file label could not be opened"));
    }
  }
}

void loop() {}