#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi AP
const char* ssid = "ESP32_MASTER";
const char* password = "12345678";

// Slave IPs
String slave1 = "192.168.4.3"; // RED
String slave2 = "192.168.4.4"; // BLUE
String slave3 = "192.168.4.5"; // YELLOW

// Pins
#define TRIG1 32
#define TRIG2 33
#define TRIG3 26

String lastState = "";

// Timing
unsigned long displayStartTime = 0;
const int displayHoldTime = 1000;

// OLED
void showMessage(String line1, String line2 = "") {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.println(line1);

  if (line2 != "") {
    display.setCursor(0, 35);
    display.println(line2);
  }

  display.display();
}

// 🔥 SEND COMMAND
void triggerSlave(String ip) {
  HTTPClient http;

  String url = "http://" + ip + "/command?cmd=RUN";
  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.println("Triggered: " + ip);
  } else {
    Serial.println("Failed: " + ip);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG1, INPUT);
  pinMode(TRIG2, INPUT);
  pinMode(TRIG3, INPUT_PULLDOWN);

  Wire.begin(21, 22);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // 🔥 START WIFI AP
  WiFi.softAP(ssid, password);
  Serial.println("Master AP Started");
  Serial.println(WiFi.softAPIP());

  showMessage("System Ready");
}

void loop() {

  bool t1 = digitalRead(TRIG1);
  bool t2 = digitalRead(TRIG2);
  bool t3 = digitalRead(TRIG3);

  unsigned long now = millis();

  if (now - displayStartTime < displayHoldTime) return;

  if (t1) {
    showMessage("Red Bin", "Opening");
    triggerSlave(slave1);
    lastState = "RED";
    displayStartTime = now;
  }

  else if (t2) {
    showMessage("Blue Bin", "Opening");
    triggerSlave(slave2);
    lastState = "BLUE";
    displayStartTime = now;
  }

  else if (t3) {
    showMessage("Yellow Bin", "Opening");
    triggerSlave(slave3);
    lastState = "YELLOW";
    displayStartTime = now;
  }

  else if (lastState != "IDLE") {
    showMessage("Smart Bin", "System");
    lastState = "IDLE";
    displayStartTime = now;
  }
}
