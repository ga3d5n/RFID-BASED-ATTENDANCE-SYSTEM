#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 8

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte readcard[10];
char UIDbuffer[32];
String StrUID;

String studentName;
String studentDept;

// -------- YOUR CARD UID --------
byte nmeUID[4] = {0xA1, 0x8F, 0xD0, 0x06};

// -------- DUPLICATE CONTROL --------
unsigned long lastScanTime = 0;
String lastUID = "";
const unsigned long duplicateDelay = 10000; // 10 seconds

void setup() {

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();

  pinMode(BUZZER, OUTPUT);

  lcd.setCursor(0,0);
  lcd.print("Scan Your Card");

  // -------- PLX-DAQ HEADER --------
  Serial.println("CLEARDATA");
  Serial.println("LABEL,Date,Time,UID,Name,Department,Status");
}

void loop() {

  if (!getid()) return;

  // -------- DUPLICATE CHECK --------
  if (StrUID == lastUID && (millis() - lastScanTime) < duplicateDelay) {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Already Marked");
    lcd.setCursor(0,1);
    lcd.print("Please Wait");

    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);

    delay(2000);
    lcd.clear();
    lcd.print("Scan Your Card");
    return;
  }

  // Update last scan info
  lastUID = StrUID;
  lastScanTime = millis();

  // -------- AUTHORIZED CARD --------
  if (compareUID(readcard, nmeUID)) {

    studentName = "Ambrose Joel";
    studentDept = "EEE Dept";

    // Short Beep
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);

    // Screen 1: Name + Dept
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(studentName);
    lcd.setCursor(0,1);
    lcd.print(studentDept);
    delay(2000);

    // Screen 2: Status + Welcome
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Status: PRESENT");
    lcd.setCursor(0,1);
    lcd.print("Welcome!");
    delay(2000);

    // Excel Log
    Serial.print("DATA,DATE,TIME,");
    Serial.print(StrUID);
    Serial.print(",");
    Serial.print(studentName);
    Serial.print(",");
    Serial.print(studentDept);
    Serial.println(",PRESENT");
  }

  // -------- UNAUTHORIZED CARD --------
  else {

    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Access Denied");

    Serial.print("DATA,DATE,TIME,");
    Serial.print(StrUID);
    Serial.println(",UNKNOWN,UNKNOWN,DENIED");

    delay(2000);
  }

  lcd.clear();
  lcd.print("Scan Your Card");
}

// -------- READ UID --------
int getid() {

  if (!mfrc522.PICC_IsNewCardPresent()) return 0;
  if (!mfrc522.PICC_ReadCardSerial()) return 0;

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    readcard[i] = mfrc522.uid.uidByte[i];
  }

  array_to_string(readcard, mfrc522.uid.size, UIDbuffer);
  StrUID = UIDbuffer;

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return 1;
}

// -------- COMPARE UID --------
boolean compareUID(byte a[], byte b[]) {
  for (byte i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

// -------- UID TO STRING --------
void array_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2]   = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
    buffer[i*2+1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
  }
  buffer[len*2] = '\0';
}

