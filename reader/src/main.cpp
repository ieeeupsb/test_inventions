#include <SPI.h>
#include <MFRC522.h>
#include <sha256.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN 9   // Configurable, see typical pin layout above
#define SS_1_PIN 10 // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN 8  // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1

#define wifiSSID "ssid"
#define wifiPassword "password"

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522; // Create MFRC522 instance.

void setup()
{
  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  SPI.begin(); // Init SPI bus

  mfrc522.PCD_Init(ssPins[0], RST_PIN); // Init each MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    String uid = "";
    byte *buffer = mfrc522.uid.uidByte;
    byte bufferSize = mfrc522.uid.size;
    for (byte i = 0; i < bufferSize; i++)
    {
      uid += buffer[i] < 0x10 ? " 0" : " ";
      uid += buffer[i], HEX;
    }
    HTTPClient http;
    http.begin("http://haxor.fe.up.pt:3000/entry");
    http.addHeader("Content-Type", "text/plain");

    int httpCode = http.POST(uid);
    String payload = http.getString();

    http.end();
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  }
}
