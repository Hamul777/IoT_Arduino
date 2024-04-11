#include <Wire.h> 
#include <WiFiEsp.h>
#include <Servo.h>
#ifndef HAVE_HWSERIAL1
#include <SoftwareSerial.h>
#endif

#define ECHO_1 2
#define TRIG_1 3 
#define ECHO_2 4
#define TRIG_2 5 
#define ECHO_3 6
#define TRIG_3 7 
#define RED_1 8
#define RED_2 9
#define RED_3 10

char ssid[] = "IoT";            // your network SSID (name)
char pass[] = "qwer1234";              // your network password
int status = WL_IDLE_STATUS;             // the Wifi radio's status

WiFiEspServer server(80);
SoftwareSerial Serial1(A1, A0);          // RX, TX
Servo myServo;                           // create servo object

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
const long interval = 1000;              // interval for checking the distance

int parkingCounter = 0;

bool parkingZone1 = false;
bool parkingZone2 = false;
bool parkingZone3 = false;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  WiFi.init(&Serial1);
  myServo.attach(A5);                    // Attach the servo to digital pin 11

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  server.begin();


  pinMode(TRIG_1, OUTPUT); // Ultrasonic sensors
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);
  pinMode(TRIG_3, OUTPUT);
  pinMode(ECHO_3, INPUT);

  pinMode(RED_1, OUTPUT); // LEDs
  pinMode(RED_2, OUTPUT);
  pinMode(RED_3, OUTPUT);

}

void loop() {
  WiFiEspClient client = server.available();

  if (client) {
    Serial.println("New client");
    boolean currentLineIsBlank = true;
    String currentLine = ""; 

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c != '\n' && c != '\r') {
          currentLine += c;
        }

        if (c == '\n') {
          if (currentLine.startsWith("GET /checkout")) {
            operateServo();
            sendResponseAfterServoOperation(client);
            break;
          } else if (currentLineIsBlank) {
            sendDefaultResponse(client);
            break;
          }
          currentLine = ""; 
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(10);
    client.stop();
    Serial.println("Client disconnected");
  }

  checkParkingSpaces();
}

void checkParkingSpaces() {
  // Function to check parking spaces and update LED status
  long distance1 = calculateDistance(TRIG_1, ECHO_1);
  long distance2 = calculateDistance(TRIG_2, ECHO_2);
  long distance3 = calculateDistance(TRIG_3, ECHO_3);

  updateParkingStatus(distance1, &parkingZone1, RED_1, &previousMillis1);
  updateParkingStatus(distance2, &parkingZone2, RED_2, &previousMillis2);
  updateParkingStatus(distance3, &parkingZone3, RED_3, &previousMillis3);
}

void updateParkingStatus(long distance, bool *parkingZone, int ledPin, unsigned long *previousMillis) {
  if (distance <= 5 && !*parkingZone) {
    // 거리가 5cm 이하이고, 주차 공간이 비어 있을 때
    *parkingZone = true;
    parkingCounter++;
    digitalWrite(ledPin, HIGH); // LED를 켠다
  } else if (distance > 5 && distance <= 7) {
    // 거리가 5cm 초과 7cm 이하일 때
    blinkLED(200, ledPin, previousMillis); // 더 빠른 속도로 깜빡인다
  } else if (distance > 7 && distance <= 10) {
    // 거리가 7cm 초과 10cm 이하일 때
    blinkLED(1000, ledPin, previousMillis); // 느린 속도로 깜빡인다
  } else if (distance > 10 && *parkingZone) {
    // 거리가 10cm를 초과하고, 주차 공간이 차 있을 때
    *parkingZone = false;
    parkingCounter--;
    digitalWrite(ledPin, LOW); // LED를 끈다
  }
}


long calculateDistance(int TRIG, int ECHO) {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH);
  return duration / 2 / 29.1;
}


void operateServo() {
  // Operate the servo to open and then close
  myServo.write(0);  // Open the servo (adjust angle as needed)
  delay(3000);        // Wait for 1 second
  myServo.write(90);   // Close the servo
}

void sendResponseAfterServoOperation(WiFiEspClient &client) {
  // Send a response back to the client after servo operation
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<!DOCTYPE HTML><html>");
  client.println("<head><title>Servo Operated</title></head>");
  client.println("<body>");
  client.println("<h1>Checkout Complete</h1>");
  client.println("<p>Servo has been operated.</p>");
  client.println("<button onclick=\"location.href='/'\">Back to Home</button>");
  client.println("</body></html>");
}

void sendDefaultResponse(WiFiEspClient &client) {
  // Send default web page
  int parking = 3 - parkingCounter;
  client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
  client.println("<!DOCTYPE HTML><html>");
  client.println("<head><title>Parking System</title></head>");
  client.println("<body>");
  client.println("<h1>Parking System</h1>");
  client.println("<h3>empty parking: " + String(parking) + "</h3>");
  client.println("<button onclick=\"location.href='/checkout'\">Check out</button>");
  client.println("<style>");
  client.println("button {width: 200px; height: 100px; font-size: 24px; margin: 10px;}");
  client.println(".parking-space {margin-bottom: 15px;}");
  client.println("</style>");
  client.println("</body></html>");
}

void blinkLED(unsigned long interval, int ledPin, unsigned long *previousMillis) {
  unsigned long currentMillis = millis();

  if (currentMillis - *previousMillis >= interval) {
    *previousMillis = currentMillis;
    digitalWrite(ledPin, !digitalRead(ledPin));
  }
}

