#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<Keypad.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Firebase_ESP_Client.h>

//Oled
#define SS_PIN 5
#define RST_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//firebase
/* 1. Define the WiFi credentials */
#define WIFI_SSID "10/149/211 KT"
#define WIFI_PASSWORD "03082002"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA5jxcI0Dle0iN5pJY-aqw8j7qtZVyiE1M"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://smartlock-25056-default-rtdb.asia-southeast1.firebasedatabase.app/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "doxhuy2104@gmail.com"
#define USER_PASSWORD "wqqpwppr2104"

//
int x1=0, x2=-128;
int count = 0;
String input = "";
int tryCount=5;
int lockState=1;

String PIN="2104";

byte nuidPICC[4];

byte listRFID[][4]= {
  {0x94,0x64,0x0F,0x02}
};

int numCards = sizeof(listRFID) / sizeof(listRFID[0]);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

void setup() {
    Serial.begin(115200);
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Không tìm thấy màn hình OLED");
        while (true);
    }

    display.clearDisplay();

    display.setTextSize(1); 
    display.setTextColor(SSD1306_WHITE); 
      display.setCursor(20,18);
      display.print("Nhap mat khau");
      display.display();

      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;
}

void loop() {
  readRFID();
  enterPIN();
  getFirebaseData();
}

void oledDisplay(const char *text){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println(text);
  display.display();
}
void mainScreen(){
display.clearDisplay();
      display.setTextSize(1); 
      display.setTextColor(SSD1306_WHITE); 
      display.setCursor(20,18);
      display.print("Nhap mat khau");
      display.display();
}

void readRFID(){
// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

// display
  // display.clearDisplay();
  // display.setCursor(0,0);
  // display.println(F("RFID Detected!"));
  
  // display.print(F("HEX: "));
  // for (byte i = 0; i < rfid.uid.size; i++) {
  //   display.print(rfid.uid.uidByte[i], HEX);
  //   display.print(" ");
  // }
  if (check(rfid.uid.uidByte)) {
    Serial.println(F("Thẻ hợp lệ!"));

    oledDisplay("Mo khoa thanh cong!");
        tryCount=5;
        input="";
        delay(5000);
        display.clearDisplay();

      display.setTextSize(1); 
      display.setTextColor(SSD1306_WHITE); 
      display.setCursor(20,18);
      display.print("Nhap mat khau");
      display.display();
  }
  else{
    oledDisplay("The sai!");
    delay(2000);
    mainScreen();

  }
  
  // display.println();
  // display.print(F("DEC: "));
  // for (byte i = 0; i < rfid.uid.size; i++) {
  //   display.print(rfid.uid.uidByte[i], DEC);
  //   display.print(" ");
  // }

  // display.display();

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void enterPIN(){
char customKey = customKeypad.getKey();
  
  if (customKey){
    if(count==0){
      display.clearDisplay();
      display.setCursor(20,18);
          display.setTextSize(2); 

      }
    display.print("* ");
    display.display();
    input+=customKey;
    Serial.println(customKey);
    count++;
    if(count==4){
      
      count=0;
      if(input==PIN){
        oledDisplay("Mo khoa thanh cong!");
        tryCount=5;
        input="";
        delay(5000);
        mainScreen();
      }
      else{
        input="";
        tryCount--;
        oledDisplay("Sai mat khau");
        oledDisplay(("Ban con " + String(tryCount) + " lan thu").c_str());
        if(tryCount==0){
          delay(300000);
          tryCount=5;
        }
      }
    }
    
  }
}

void getFirebaseData(){
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

   if(Firebase.RTDB.getInt(&fbdo, "/lock/state", &lockState)){
    if(!lockState){
      oledDisplay("Mo khoa thanh cong!");
        delay(5000);
        mainScreen();
        lockState=1;
        Firebase.RTDB.setInt(&fbdo,"/lock/state", 1);
    } 
   }else{
    // Serial.println(fbdo.errorReason().c_str());
   }
  }
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}

bool check(byte *id){
  for(int i=0;i<numCards;i++){
    if (id[0]==listRFID[i][0]&&id[1]==listRFID[i][1]&&id[2]==listRFID[i][2]&&id[3]==listRFID[i][3]) {
      return true;
    }
  }
  return false;
}


