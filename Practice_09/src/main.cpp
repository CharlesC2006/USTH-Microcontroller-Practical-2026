// PIR
#include <Arduino.h>

const uint8_t PIR_PIN    = 2; // D2
const uint8_t LED_PIN    = 3; // D3
const uint8_t BUZZER_PIN = 4; // D4

bool lastMotionState = false;

void setup() {
  Serial.begin(9600);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // PIR cần thời gian ổn định cảm biến hồng ngoại sau khi cấp nguồn
  Serial.println("PIR warming up...");
  delay(2000);
  Serial.println("PIR ready");
}

void loop() {
  bool motionDetected = (digitalRead(PIR_PIN) == HIGH);

  // Chỉ gửi UART khi trạng thái THAY ĐỔI, không lặp lại liên tục
  if (motionDetected != lastMotionState) {
    lastMotionState = motionDetected;

    digitalWrite(LED_PIN, motionDetected ? HIGH : LOW);
    digitalWrite(BUZZER_PIN, motionDetected ? HIGH : LOW);

    Serial.println(motionDetected ? "Motion detected" : "No motion");
  }

  delay(1000); // giảm tải CPU, PIR không đổi trạng thái quá nhanh
}