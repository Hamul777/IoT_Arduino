#include <Servo.h>
Servo servoMotor; // 서보 모터

const int trigPin = 9; // 초음파 센서 트리거 핀s
const int echoPin = 10; // 초음파 센서 에코 핀
const int redLed = 11; // 빨간 LED 핀
const int greenLed = 12; // 초록 LED 핀
const int piezo = 13; // 피에조 부저 핀

void setup() {
  servoMotor.attach(3); // 서보 모터 핀 설정
  pinMode(trigPin, OUTPUT); // 초음파 센서 출력 설정
  pinMode(echoPin, INPUT); // 초음파 센서 입력 설정
  pinMode(redLed, OUTPUT); // 빨간 LED 출력 설정
  pinMode(greenLed, OUTPUT); // 초록 LED 출력 설정
  pinMode(piezo, OUTPUT); // 피에조 출력 설정

  digitalWrite(redLed, HIGH); // 초기에 빨간 불 켜짐

  Serial.begin(9600); // 시리얼 통신 시작
}

void loop() {
  long duration, distance;

  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: "); // 시리얼 모니터에 거리 출력
  Serial.print(distance);
  Serial.println("cm");

  if (distance < 5) { 
    openGate();
  } else {
    closeGate();
  }
  delay(100);
}

void openGate() {
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, HIGH);
  servoMotor.write(0); // 문 열기
  tone(piezo, 1000, 200); // 피에조 소리
  delay(250);
  tone(piezo, 1000, 200); // 피에조 소리
  delay(2000);
}

void closeGate() {/Users/kim-woo-won/Downloads/final/final.ino
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  servoMotor.write(90); // 문 닫기
  noTone(piezo); // 소리 끄기
  delay(100);
}
