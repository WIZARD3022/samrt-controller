#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "ESP32_MASTER";
const char* password = "12345678";

// STATIC IP (CHANGE FOR EACH SLAVE)
IPAddress local_IP(192, 168, 4, 5);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// Motor pins
#define IN1 26
#define IN2 27
#define ENA 25

// Motor state
bool running = false;
unsigned long motorStart = 0;
int motorStage = 0;

// 🚀 START MOTOR
void startMotor() {
  Serial.println("Motor Triggered!");
  running = true;
  motorStart = millis();
  motorStage = 1;
}

// 🔁 NON-BLOCKING MOTOR CONTROL
void updateMotor() {
  if (!running) return;

  unsigned long now = millis();

  if (motorStage == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(ENA, 200);

    if (now - motorStart > 3000) {
      motorStage = 2;
      motorStart = now;
    }
  }

  else if (motorStage == 2) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(ENA, 200);

    if (now - motorStart > 3000) {
      motorStage = 3;
    }
  }

  else {
    ledcWrite(ENA, 0);
    running = false;
  }
}

// 🌐 HANDLE HTTP REQUEST
void handleCommand() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");

    if (cmd == "RUN" && !running) {
      startMotor();
      server.send(200, "text/plain", "Motor Started");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid Command");
}

void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // ✅ NEW PWM API
  ledcAttach(ENA, 1000, 8);

  // WiFi setup
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Server
  server.on("/command", handleCommand);
  server.begin();
}

void loop() {
  server.handleClient();
  updateMotor();
}
