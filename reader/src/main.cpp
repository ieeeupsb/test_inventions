#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <sha256.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/******************************************* START OF CONFIGURATIONS ******************************************/

//SPI bus pins
#define RST_PIN         0   //D3
#define SS_PIN          2   //D4

//LED pins
#define RED_LED_PIN     16  //D0
#define YELLOW_LED_PIN  5   //D1
#define GREEN_LED_PIN   4   //D2

//WiFi credentials
#define wifiSSID "ssid"
#define wifiPassword "password"

uint8_t hmacKey[]={
  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
};

#define HMAC_KEY_LENGTH 20 //This number must reflect the number of bytes in 'hmacKey'.

const char* url = "http://haxor.fe.up.pt:3000/entry";

/******************************************** END OF CONFIGURATIONS *******************************************/
/******************* DO NOT CHANGE CODE AFTER THIS POINT UNLESS YOU KNOW WHAT YOU ARE DOING *******************/

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String payload_s = "";
int payload_i = 0;

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

  mfrc522.PCD_Init(); //Init each MFRC522 card
  mfrc522.PCD_DumpVersionToSerial();

  red(false);     //
  yellow(false);  //Turn off all LEDs
  green(false);   //

}

void loop(){

  payload_s = "";
  payload_i = 0;

  // Look for new cards
	if(!mfrc522.PICC_IsNewCardPresent())
		return;

	// Select one of the cards
	if(!mfrc522.PICC_ReadCardSerial())
		return;

  yellow(true); //Indicate that the reading has started.

  String uid = "";
  byte *buffer = mfrc522.uid.uidByte;
  byte bufferSize = mfrc522.uid.size;
  for (byte i = 0; i < bufferSize; i++){ //Populate the 'uid' string
    uid += buffer[i] < 0x10 ? " 0" : " ";
    uid += buffer[i], HEX;
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST(generateHash(uid, hmacKey, HMAC_KEY_LENGTH)); //HTTP POST and http response code to such POST

  // httpCode will be negative on error
  if(httpCode > 0){
    // HTTP header has been send and Server response header has been handled
    //Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      payload_s = http.getString(); //Get response string to the POST.
      payload_i = payload_s.toInt(); //Converts the receieved string to an integer.

      Serial.println(payload_s);
    }
  }
  //else
    //Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());

  http.end();

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

  mfrc522.PICC_HaltA(); //Halt PICC
  mfrc522.PCD_StopCrypto1(); //Stop encryption on PCD
}
