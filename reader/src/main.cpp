#include <SPI.h>
#include <MFRC522.h>
#include <sha256.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN 9   // Configurable, see typical pin layout above
#define SS_1_PIN 10 // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN 8  // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 1
#define RED_LED_PIN 1
#define YELLOW_LED_PIN 2
#define GREEN_LED_PIN 3

#define wifiSSID "ssid"
#define wifiPassword "password"

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522; // Create MFRC522 instance.

uint8_t hmacKey[]={
  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
};

#define HMAC_KEY_LENGHT 20

String generateHash(String data, const uint8_t* key, int key_length){
  Sha256.initHmac(key, key_length); // key, and length of key in bytes
  Sha256.print(data);
  return Sha256.resultHmac();
}

void red(bool state, int pin = RED_LED_PIN){
  digitalWrite(pin, state);
}
void yellow(bool state, int pin = YELLOW_LED_PIN){
  digitalWrite(pin, state);
}
void green(bool state, int pin = GREEN_LED_PIN){
  digitalWrite(pin, state);
}

void setup()
{
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  red(true);
  yellow(true);
  green(true);

  WiFi.begin(wifiSSID, wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  SPI.begin(); // Init SPI bus

  mfrc522.PCD_Init(ssPins[0], RST_PIN); // Init each MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();

  red(false);
  yellow(false);
  green(false);

}

void loop()
{
  if (WiFi.status() == WL_CONNECTED && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    yellow(true);
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

    int httpCode = http.POST(generateHash(uid, hmacKey, HMAC_KEY_LENGTH));
    String payload_s = http.getString();

    int payload_i = payload_s.toInt();

    if(String(payload_i) == payload_s && !payload_i){ //this insures that the conversion went well, because when it fails it returns 0, which is a valid input.
      yellow(false);
      green(true);
      delay(2000);
      green(false);
    }
    else{
      yellow(false);
      red(true);
      delay(2000);
      red(false);
    }

    http.end();
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
  }
}
