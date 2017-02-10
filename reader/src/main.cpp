#include <Arduino.h>
#include <SPI.h>
#include "MFRC522.h"
#include <sha256.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/******************************************* START OF CONFIGURATIONS ******************************************/

//SPI bus pins
#define RST_PIN         0   //D3 //DO NOT F****** CONNECT
#define SS_PIN          2   //D4

//LED pins
#define RED_LED_PIN     16  //D0
#define YELLOW_LED_PIN  5   //D1
#define GREEN_LED_PIN   4   //D2

//WiFi credentials
#define wifiSSID			"ssid"
#define wifiPassword	"password"

uint8_t hmacKey[]={
  0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
};

#define HMAC_KEY_LENGTH 20 //This number must reflect the number of bytes in 'hmacKey'.

const char* host = "haxor.fe.up.pt";
int port = 4444;

/******************************************** END OF CONFIGURATIONS *******************************************/
/******************* DO NOT CHANGE CODE AFTER THIS POINT UNLESS YOU KNOW WHAT YOU ARE DOING *******************/

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

char payload_c = '1';
int payload_i = 1;
bool con = false;
WiFiClient client;

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

String URLEncode(const char* msg){
    const char *hex = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}

bool connect(){
	if(client.connect(host, port)){
		con = true;
		return true;
	}
	else
		return false;
}

void setup(){
  Serial.begin(115200);
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
	
	while(!connect()){}

  red(false);     //
  yellow(false);  //Turn off all LEDs
  green(false);   //
}

void loop(){

  payload_c = '1';
  payload_i = 1;

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

	String PostData = "uid=" + URLEncode(generateHash(uid, hmacKey, HMAC_KEY_LENGTH));
	
	if(con){
		client.println("POST /checkin HTTP/1.1");
		client.println("Host: " + String(host) + ":" + port);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.println("Cache-Control: no-cache");
		client.print("Content-Length: ");
		client.println(PostData.length());
		client.println();
		client.println(PostData);
		
		long interval = 2000;
		unsigned long currentMillis = millis(), previousMillis = millis();

		while(!client.available()){

			if( (currentMillis - previousMillis) > interval ){
				Serial.println("Timeout");
				client.stop();
				con = false;
				return;
			}
			currentMillis = millis();
		}

		while (client.connected()) {
			if ( client.available() ) {
				payload_c = client.read();
			}
			else
				break;
		}
	}
	else
		while(!connect()){}
	
	payload_i = payload_c - '0'; //Converts the receieved char to an integer.
	
	Serial.println("");
  Serial.println("http end");

  if(!payload_i){
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
  Serial.println("end of cycle\n");
  mfrc522.PICC_HaltA(); //Halt PICC
  mfrc522.PCD_StopCrypto1(); //Stop encryption on PCD
}
