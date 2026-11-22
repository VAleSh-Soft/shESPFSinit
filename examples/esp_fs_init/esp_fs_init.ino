#include <shESPFSinit.h>

shESPFSinit esp_fs;

void setup()
{
  Serial.begin(115200);

  Serial.print(F("File system initialization..."));
  if (esp_fs.init())
  {
    Serial.println(F("OK"));
    Serial.println();

    String labelFileName = esp_fs.getLabelFileName();
    Serial.print(F("File label name: "));
    Serial.println(labelFileName);

    File file = ESP_FILESYSTEM.open(labelFileName, "r");
    if (file)
    {
      Serial.print(F("The file label contains: "));
      Serial.println(file.readString());
      file.close();
    }
    else
    {
      Serial.println(F("The file label could not be opened"));
    }
  }
  else
  {
    Serial.println(F("FAIL"));
  }
}

void loop() {}