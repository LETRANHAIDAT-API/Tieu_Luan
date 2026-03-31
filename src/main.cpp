#define BLYNK_TEMPLATE_ID "TMPL6f87J3uz5"
#define BLYNK_TEMPLATE_NAME "BaiThi"
#define BLYNK_AUTH_TOKEN "1GzBPZWo1x1ydcGPbkqoPNl4RZJ-CSzu"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

#define SS_PIN    5
#define RST_PIN   22
#define RELAY_PIN 15
#define BUZZER_PIN 2

MFRC522 rfc522(SS_PIN, RST_PIN);
BlynkTimer timer;

String masterUID = "11 22 33 44"; 
unsigned long lockOpenedMillis = 0;   
const long lockInterval = 500;       
bool isLocked = true;                 

void playBeep(int duration) {
  tone(BUZZER_PIN, 1000); 
  delay(duration); 
  noTone(BUZZER_PIN);
}

void openDoor(String source) {
  Serial.println("Mo cua tu: " + source);
  digitalWrite(RELAY_PIN, HIGH);
  isLocked = false;
  lockOpenedMillis = millis();
  playBeep(500);

  Blynk.virtualWrite(V1, "Open"); 
  Serial.println("Trang thai: Open");
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  if (value == 1) {
    if (isLocked) {
      openDoor("App Blynk");
      Blynk.virtualWrite(V1, "Open"); 
    }
  } else {
    
    digitalWrite(RELAY_PIN, LOW);
    isLocked = true;
    Blynk.virtualWrite(V1, "Close"); 
    Serial.println("Cua dong tu App.");
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfc522.PCD_Init();
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("--- HE THONG SMART LOCK KET NOI BLYNK ---");
  Blynk.virtualWrite(V1, "He thong san sang!");
}

void loop() {
  Blynk.run();
  timer.run();

 
  unsigned long currentMillis = millis();
  if (!isLocked && (currentMillis - lockOpenedMillis >= lockInterval)) {
    digitalWrite(RELAY_PIN, LOW);
    isLocked = true;
    Serial.println("Cua da dong.");
    Blynk.virtualWrite(V1, "Close");
    Blynk.virtualWrite(V0, 0); 
  }

  
  if (!rfc522.PICC_IsNewCardPresent() || !rfc522.PICC_ReadCardSerial()) {
    return;
  }

  String content = "";
  for (byte i = 0; i < rfc522.uid.size; i++) {
    content.concat(String(rfc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  String scannedUID = content.substring(1);

  Serial.print("Quet the: ");
  Serial.println(scannedUID);

  if (scannedUID == masterUID) {
    openDoor("The RFID");
  } else {
    Serial.println("Canh bao: Sai the!");
    Blynk.virtualWrite(V1, "CANH BAO: Sai the!");
    for(int i=0; i<3; i++) {
      playBeep(100);
      delay(100);
    }
  }
}