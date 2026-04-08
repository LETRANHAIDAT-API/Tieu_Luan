#define BLYNK_TEMPLATE_ID "TMPL6f87J3uz5"
#define BLYNK_TEMPLATE_NAME "BaiThi"
#define BLYNK_AUTH_TOKEN "1GzBPZWo1x1ydcGPbkqoPNl4RZJ-CSzu"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <UniversalTelegramBot.h>

char ssid[] = "Wokwi-GUEST"; 
char pass[] = "";

#define BOT_TOKEN "8603859822:AAGZ0lCTspWXLyZJjS_-mqLcoWau48BxE_0"
#define GROUP_ID "-5139517971"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

#define SS_PIN      5
#define RST_PIN     22
#define LED_GREEN   4  
#define LED_RED     15
#define BUZZER_PIN  2

MFRC522 rfc522(SS_PIN, RST_PIN);
BlynkTimer timer;

String masterUID = "11 22 33 44"; 
unsigned long lockOpenedMillis = 0;   
const long lockInterval = 2000;      
bool isLocked = true;                

void playBeep(int duration) {
  tone(BUZZER_PIN, 1000); 
  delay(duration); 
  noTone(BUZZER_PIN);
}

void sendTelegramMessage(String message) {
  Serial.println("Dang gui Telegram...");
  bot.sendMessage(GROUP_ID, message, "");
}

void openDoor(String source) {
  Serial.println("Mo cua tu: " + source);
  digitalWrite(LED_GREEN, HIGH); 
  isLocked = false;
  lockOpenedMillis = millis();
  playBeep(500); 
  digitalWrite(LED_GREEN, LOW); 

  Blynk.virtualWrite(V1, "Cửa đã mở"); 
  
  String msg = "THÔNG BÁO*: Cửa đã được mở!\n";
  msg += " Nguồn: " + source;
  sendTelegramMessage(msg);
}

BLYNK_WRITE(V0) {
  int value = param.asInt();
  if (value == 1) {
    if (isLocked) {
      openDoor("App Blynk");
    }
  } else {
    isLocked = true;
    Blynk.virtualWrite(V1, "Cửa đã đóng"); 
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfc522.PCD_Init();
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  secured_client.setInsecure();

  Serial.println("--- HE THONG SMART LOCK READY ---");
  Blynk.virtualWrite(V1, "He thong san sang!");
}

void loop() {
  Blynk.run();
  timer.run();

  unsigned long currentMillis = millis();
  if (!isLocked && (currentMillis - lockOpenedMillis >= lockInterval)) {
    isLocked = true;
    Blynk.virtualWrite(V1, "Cửa đã đóng");
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
    
    for(int i=0; i<3; i++) {
      digitalWrite(LED_RED, HIGH);
      playBeep(100);
      digitalWrite(LED_RED, LOW);
      delay(100);
    }

    Blynk.virtualWrite(V1, "CANH BAO: Sai the!");
    String warnMsg = "*CẢNH BÁO NGUY HIỂM*\n";
    warnMsg += "Phát hiện quẹt thẻ lạ!\n Mã: `" + scannedUID + "`";
    sendTelegramMessage(warnMsg);
  }
}