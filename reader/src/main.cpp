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

#define HMAC_KEY_LENGTH 20

char* generateHash(String data, const uint8_t* key, int key_length){
  Sha256.initHmac(key, key_length); // key, and length of key in bytes
  Sha256.print(data);
  return (char*)Sha256.resultHmac();
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

bool networkDetected(String ssid, int &n){
  Serial.println("Scanning for networks...");
  n = WiFi.scanNetworks();
  Serial.println("Scan complete.");

  for(int i = 0; i < n; i++){
    if(!strcmp(ssid.c_str(), WiFi.SSID(i).c_str()))
      return true;
  }

  return false;
}
bool atemptConnection(String ssid, String pass){
  Serial.println("Using credentials:\n\tssid: "+ssid+"\n\tpass: " + pass);


  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    return false;
  }

  //  if(FIXED_IP)
  //    WiFi.config(ip, gateway, subnet);
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void setup(){
  pinMode(RED_LED_PIN, OUTPUT);     //
  pinMode(YELLOW_LED_PIN, OUTPUT);  //Set up LED pins
  pinMode(GREEN_LED_PIN, OUTPUT);   //
  red(true);    //
  yellow(true); //Turn on all LEDs to indicate that the device is booting and to test those same LEDs.
  green(true);  //


  int n = 0;
  while(1){
    if(networkDetected(wifiSSID, n) && atemptConnection(wifiSSID, wifiPassword))
      break;
    delay(3000);
  }

  SPI.begin(); //Init SPI bus

  mfrc522.PCD_Init(ssPins[0], RST_PIN); //Init each MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();

  red(false);     //
  yellow(false);  //Turn off all LEDs
  green(false);   //

}

void loop(){
  if (WiFi.status() == WL_CONNECTED && mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
    yellow(true); //Indicate that the reading has started.

    String uid = "";
    byte *buffer = mfrc522.uid.uidByte;
    byte bufferSize = mfrc522.uid.size;
    for (byte i = 0; i < bufferSize; i++){ //Populate the 'uid' string
      uid += buffer[i] < 0x10 ? " 0" : " ";
      uid += buffer[i], HEX;
    }

    HTTPClient http;
    http.begin("http://haxor.fe.up.pt:3000/entry");
    http.addHeader("Content-Type", "text/plain");

    int httpCode = http.POST(generateHash(uid, hmacKey, HMAC_KEY_LENGTH)); //HTTP POST and http response code to such POST
    String payload_s = http.getString(); //Get response string to the POST.

    int payload_i = payload_s.toInt(); //Converts the receieved string to an integer.

    /*
     *
     *Do Something with the http code. Nedd to do some research first.
     *
    */

    if(String(payload_i) == payload_s && !payload_i){ //This insures that the conversion went well, because when it fails it returns 0, which is a valid input.
      yellow(false); //Turn off the yellow LED, because we are about to show a response
      green(true); //Turn on the green LED to show that the uid has accepted by the server.
      delay(2000); //Wait two seconds to give time for the user to see the response.
      green(false); //Turn off the green LED
    }
    else{
      yellow(false); //Turn off the yellow LED, because we are about to show a response
      red(true); //Turn on the red LED to show that the uid has rejected by the server. The user is either not registered, or the uid has not read correctly.
      delay(2000); //Wait two seconds to give time for the user to see the response.
      red(false); //Turn off the red LED
    }

    http.end();

    mfrc522.PICC_HaltA(); //Halt PICC
    mfrc522.PCD_StopCrypto1(); //Stop encryption on PCD
  }
}
