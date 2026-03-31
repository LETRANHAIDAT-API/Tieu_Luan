#include <SPI.h>
#include <MFRC522.h>

// Định nghĩa các chân cắm
#define SS_PIN    5
#define RST_PIN   22
#define RELAY_PIN 15
#define BUZZER_PIN 2

MFRC522 rfc522(SS_PIN, RST_PIN);

// Cấu hình hệ thống
String masterUID = "11 22 33 44"; 
unsigned long lockOpenedMillis = 0;   // Lưu thời điểm mở khóa
const long lockInterval = 500;       // Thời gian mở khóa (1 giây)
bool isLocked = true;                 // Trạng thái khóa

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfc522.PCD_Init();
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);       // Mặc định khóa đóng
  digitalWrite(BUZZER_PIN, LOW);      // Mặc định còi tắt
  
  Serial.println("--- HE THONG KHOA CUA RFID KHOI DONG ---");
}

// Hàm tạo tiếng bíp không dùng delay (ms là thời gian kêu)
// void playBeep(int duration) {
//   digitalWrite(BUZZER_PIN, HIGH);
//   unsigned long startBeep = millis();
//   while (millis() - startBeep < duration) {
//     // Đợi cho đến khi đủ thời gian mà không dừng toàn bộ chương trình
//   }
//   digitalWrite(BUZZER_PIN, LOW);
// }

void playBeep(int duration) {
  // Phát âm thanh tần số 1000Hz (tiếng bíp cao)
  tone(BUZZER_PIN, 1000); 
  
  unsigned long startBeep = millis();
  while (millis() - startBeep < duration) {
    // Đợi đủ thời gian duration
  }
  
  noTone(BUZZER_PIN); // Tắt âm thanh
}

void loop() {
  // 1. Kiểm tra và tự động đóng khóa sau 3 giây
  unsigned long currentMillis = millis();
  if (!isLocked && (currentMillis - lockOpenedMillis >= lockInterval)) {
    digitalWrite(RELAY_PIN, LOW);
    isLocked = true;
    Serial.println("Cua da tu dong dong.");
  }

  // 2. Kiểm tra xem có thẻ mới hay không
  if (!rfc522.PICC_IsNewCardPresent() || !rfc522.PICC_ReadCardSerial()) {
    return;
  }

  // 3. Đọc UID của thẻ
  String content = "";
  for (byte i = 0; i < rfc522.uid.size; i++) {
    content.concat(String(rfc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  String scannedUID = content.substring(1);

  Serial.print("UID the vua quet: ");
  Serial.println(scannedUID);

  // 4. Kiểm tra quyền truy cập
  if (scannedUID == masterUID) {
    Serial.println("Xac thuc thanh cong! Mo cua...");
    
    digitalWrite(RELAY_PIN, HIGH);      // Mo khoa
    playBeep(500);                     // Keu bip 0.5s bao hieu
    
    lockOpenedMillis = currentMillis;  // Luu lai thoi diem mo
    isLocked = false;                  // Cap nhat trang thai
  } 
  else {
    Serial.println("Sai the! Tu choi truy cap.");
    // Keu 3 tieng bip ngan bao loi
    for(int i=0; i<3; i++) {
      playBeep(100);
      delay(100); // Delay ngan nay co the chap nhan vi la bao loi
    }
  }
}