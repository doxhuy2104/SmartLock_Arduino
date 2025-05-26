#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Firebase_ESP_Client.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Preferences.h>

//Oled
#define SS_PIN 5
#define RST_PIN 2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//firebase
/* 1. Define the WiFi credentials */
// #define WIFI_SSID "Z9Tb"
// #define WIFI_PASSWORD "21102004"

/* 2. Define the API Key */
#define API_KEY "AIzaSyA5jxcI0Dle0iN5pJY-aqw8j7qtZVyiE1M"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://smartlock-25056-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "doxhuy2104@gmail.com"
#define USER_PASSWORD "wqqpwppr2104"

//Relay
#define RELAY_PIN 15

//Bluetooth
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define MAX_CARDS 10
//
bool isDone = false;

int x1 = 0, x2 = -128;
int count = 0;
String input = "";
int tryCount = 5;
int lockState = 1;
bool addCard = false;
bool isCheckWifi = true;

String PIN = "2104";
String newCardID = "";
String newCardName = "";
byte nuidPICC[4];

// byte listRFID[][4] = {
//   { 0x94, 0x64, 0x0F, 0x02 }
// };

byte listRFID[MAX_CARDS][4];
int numCards = 0;


MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const byte ROWS = 4;  //four rows
const byte COLS = 3;  //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 13, 12, 14, 27 };  //connect to the row pinouts of the keypad
byte colPins[COLS] = { 26, 25, 33 };      //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

String WIFI_SSID = "";
String WIFI_PASSWORD = "";
String uid = "";
String id = "";

//Preferences
Preferences preferences;

//CallBacks function of bluetooth
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      String received = "";
      for (int i = 0; i < value.length(); i++) {
        received += value[i];
      }
      Serial.print("received: ");
      Serial.println(received);
      if (value[0] == 'n') {
        for (int i = 1; i < value.length(); i++) {
          WIFI_SSID += value[i];
        }
        Serial.print("WIFI_SSID: ");
        Serial.println(WIFI_SSID);
        preferences.putString("ssid", WIFI_SSID);
      } else if (value[0] == 'p') {
        for (int i = 1; i < value.length(); i++) {
          WIFI_PASSWORD += value[i];
        }
        Serial.print("WIFI_PASSWORD: ");
        Serial.println(WIFI_PASSWORD);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        preferences.putString("password", WIFI_PASSWORD);
        Serial.print("Connecting to Wi-Fi");
        int count = 0;
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(500);
          count++;
          if (count >= 18) {
            WiFi.disconnect(true);
            pCharacteristic->setValue("WIFI_FAILED");
            pCharacteristic->notify();
            WIFI_SSID.clear();
            WIFI_PASSWORD.clear();
            preferences.putString("ssid", "");
            preferences.putString("password", "");
            return;
          }
        }
        Serial.println();
        Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        pCharacteristic->setValue("Connected");
        pCharacteristic->notify();
      } else if (value[0] == 'u') {
        for (int i = 1; i < value.length(); i++) {
          uid += value[i];
        }
        Serial.print("uid: ");
        Serial.println(uid);
        preferences.putString("uid", uid);
        pCharacteristic->setValue("Done");
        pCharacteristic->notify();
        isDone = true;
      }
      //  else if (value[0] == '-') {
      //   for (int i = 0; i < value.length(); i++) {
      //     id += value[i];
      //   }
      //   Serial.print("id: ");
      //   Serial.println(id);
      //   preferences.putString("id", id);

      //   pCharacteristic->setValue("Done");
      //   pCharacteristic->notify();
      //   BLEDevice::deinit(true);
      //   isDone = true;
      // }
    }
  }
};

class MyCallbacks2 : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      String received = "";
      for (int i = 0; i < value.length(); i++) {
        received += value[i];
      }
      Serial.print("received: ");
      Serial.println(received);
      if (value[0] == 'n') {
        for (int i = 1; i < value.length(); i++) {
          WIFI_SSID += value[i];
        }
        Serial.print("WIFI_SSID: ");
        Serial.println(WIFI_SSID);
        preferences.putString("ssid", WIFI_SSID);
      } else if (value[0] == 'p') {
        for (int i = 1; i < value.length(); i++) {
          WIFI_PASSWORD += value[i];
        }
        Serial.print("WIFI_PASSWORD: ");
        Serial.println(WIFI_PASSWORD);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        preferences.putString("password", WIFI_PASSWORD);
        Serial.print("Connecting to Wi-Fi");
        int count = 0;
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
          delay(500);
          count++;
          if (count >= 18) {
            WiFi.disconnect(true);
            pCharacteristic->setValue("WIFI_FAILED");
            pCharacteristic->notify();
            WIFI_SSID.clear();
            WIFI_PASSWORD.clear();
            preferences.putString("ssid", "");
            preferences.putString("password", "");
            return;
          }
        }
        Serial.println();
        Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();
        pCharacteristic->setValue("Connected");
        pCharacteristic->notify();
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Không tìm thấy màn hình OLED");
    while (true)
      ;
  }

  Serial.println(WiFi.macAddress());

  preferences.begin("lock-config", false);

    WIFI_SSID = preferences.getString("ssid", "");
  WIFI_PASSWORD = preferences.getString("password", "");
  uid = preferences.getString("uid", "");
  // id = preferences.getString("id", "");
  numCards = preferences.getInt("num_cards", 0);
  if (numCards > 0 && numCards <= MAX_CARDS) {
    preferences.getBytes("card_list", listRFID, numCards * 4);
    Serial.println("Khôi phục danh sách thẻ từ Preferences:");
    for (int i = 0; i < numCards; i++) {
      Serial.print("Thẻ ");
      Serial.print(i + 1);
      Serial.print(": ");
      printHex(listRFID[i], 4);
      Serial.println();
    }
  }

  if (WIFI_SSID != "" && WIFI_PASSWORD != "" && uid != "") {
    Serial.println("Khôi phục thông tin từ bộ nhớ:");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("Password: ");
    Serial.println(WIFI_PASSWORD);
    Serial.print("UID: ");
    Serial.println(uid);
    // Serial.print("ID: ");
    // Serial.println(id);

    // Kết nối WiFi với thông tin đã lưu
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    Serial.print("Connecting to Wi-Fi");
    int count = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
      count++;
      // if (count >= 18) {
      //   Serial.println("Kết nối WiFi thất bại, chờ thông tin mới");
      //   WiFi.disconnect(true);
      //   // Xóa thông tin đã lưu để yêu cầu nhập lại
      //   preferences.putString("ssid", "");
      //   preferences.putString("password", "");
      //   preferences.putString("uid", "");
      //   preferences.putString("id", "");
      //   WIFI_SSID.clear();
      //   WIFI_PASSWORD.clear();
      //   uid.clear();
      //   id.clear();
      //   break;
      // }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected with IP: ");
      Serial.println(WiFi.localIP());
      isDone = true;
    }
  } else {
    Serial.println("Không tìm thấy thông tin đầy đủ trong bộ nhớ");
  }
if (WIFI_SSID == "" && WIFI_PASSWORD == "" && uid == ""){
  //bluetooth
  BLEDevice::init("My Lock");
  BLEAddress bleAddress = BLEDevice::getAddress();
  id =bleAddress.toString();
  id.toUpperCase();
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();


  // pinMode(RELAY_PIN, OUTPUT);
  // digitalWrite(RELAY_PIN, HIGH);

 
  while (!isDone) {
    delay(100);
  }
  delay(5000);
  BLEDevice::deinit(true);
}
 display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 18);
  display.print("Nhap mat khau");
  display.display();

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
  if(isCheckWifi){
    checkWifi();
  }
}

void oledDisplay(const char *text) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.println(text);
  display.display();
}
void mainScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 18);
  display.print("Nhap mat khau");
  display.display();
}

void readRFID() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    if (addCard) {
      newCardID = "";
      for (byte i = 0; i < 4; i++) {
        char hex[3];
        sprintf(hex, "%02X", rfid.uid.uidByte[i]);
        newCardID += hex;
      }
      if (Firebase.RTDB.setString(&fbdo, uid + "/" + id + "/cards/" + newCardID, newCardName)) {
        Firebase.RTDB.set(&fbdo, uid + "/" + id + "/cards/add", "0");
        addCard = false;
        //add to listRFID
        if (numCards < MAX_CARDS) {
          byte cardID[4];
          if (hexStringToByteArray(newCardID, cardID)) {
            memcpy(listRFID[numCards], cardID, 4);
            numCards++;
            preferences.putBytes("card_list", listRFID, numCards * 4);
            preferences.putInt("num_cards", numCards);
            Serial.print("Đã thêm thẻ: ");
            Serial.println(newCardID);
          } else {
            Serial.println("Lỗi: ID thẻ không hợp lệ");
          }
        } else {
          Serial.println("Lỗi: Danh sách thẻ đã đầy");
        }
        oledDisplay("Them the thanh cong!");
        delay(2000);
        mainScreen();
      } else {
        Serial.println("Lỗi thêm thẻ: " + fbdo.errorReason());
        oledDisplay("Loi them the!");
        delay(2000);
        mainScreen();
      }
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  } else Serial.println(F("Card read previously."));

  // display
  // display.clearDisplay();
  // display.setCursor(0,0);
  // display.println(F("RFID Detected!"));

  // display.print(F("HEX: "));
  // for (byte i = 0; i < rfid.uid.size; i++) {
  //   display.print(rfid.uid.uidByte[i], HEX);
  //   display.print(" ");
  // }
  if (!addCard && check(rfid.uid.uidByte)) {
    Serial.println(F("Thẻ hợp lệ!"));

    oledDisplay("Mo khoa thanh cong!");
    tryCount = 5;
    input = "";
    delay(5000);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 18);
    display.print("Nhap mat khau");
    display.display();
  } else if (!addCard) {
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

void enterPIN() {
  char customKey = customKeypad.getKey();

  if (customKey) {
    if (count == 0) {
      display.clearDisplay();
      display.setCursor(20, 18);
      display.setTextSize(2);
    }
    display.print("* ");
    display.display();
    input += customKey;
    Serial.println(customKey);
    count++;
    if (count == 4) {

      count = 0;
      if (input == PIN) {
        oledDisplay("Mo khoa thanh cong!");
        tryCount = 5;
        input = "";
        delay(5000);
        mainScreen();
      } else {
        input = "";
        tryCount--;
        oledDisplay("Sai mat khau");
        oledDisplay(("Ban con " + String(tryCount) + " lan thu").c_str());
        if (tryCount == 0) {
          delay(300000);
          tryCount = 5;
        }
      }
    }
  }
}



void getFirebaseData() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.getInt(&fbdo, uid + "/" + id + "/state", &lockState)) {
      if (!lockState) {
        oledDisplay("Mo khoa thanh cong!");
        delay(5000);
        mainScreen();
        lockState = 1;
        Firebase.RTDB.setInt(&fbdo, uid + "/" + id + "/state", 1);
      }
    } else {
      Serial.println(fbdo.errorReason().c_str());
    }
    if (Firebase.RTDB.get(&fbdo, uid + "/" + id+ "/PIN")) {
      PIN = fbdo.stringData();
    }
    if (Firebase.RTDB.get(&fbdo, uid + "/" + id+ "/cards/add")) {

      if (fbdo.stringData() != "0") {
        newCardName = fbdo.stringData();
        oledDisplay("Quet the de them");
        addCard = true;
      }
    }
  }
}

//RFID
bool hexStringToByteArray(String hexStr, byte *byteArray) {
  // if (hexStr.length() != 8) return false;
  for (int i = 0; i < 4; i++) {
    String byteStr = hexStr.substring(i * 2, i * 2 + 2);
    byteArray[i] = (byte)strtol(byteStr.c_str(), NULL, 16);
  }
  return true;
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

bool check(byte *id) {
  for (int i = 0; i < numCards; i++) {
    if (id[0] == listRFID[i][0] && id[1] == listRFID[i][1] && id[2] == listRFID[i][2] && id[3] == listRFID[i][3]) {
      return true;
    }
  }
  return false;
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    isCheckWifi = false;
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    isDone = false;

    BLEDevice::init("My Lock");
    BLEServer *pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pCharacteristic =
      pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks2());

    pCharacteristic->setValue("Hello World");
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
  }
}