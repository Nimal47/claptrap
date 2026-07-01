#include <WiFi.h>
#include <WebServer.h>

// Replace with your actual network credentials
const char* ssid = "Nimal";
const char* password = "Nimal@47";

WebServer server(80);

// --- Motor A Pins ---
const int PWMA = 25;
const int AIN1 = 26;
const int AIN2 = 27;

// --- Motor B Pins ---
const int PWMB = 14;
const int BIN1 = 12;
const int BIN2 = 13;

// --- Standby Pin ---
const int STBY = 33;

// Motors can't usefully exceed this PWM under load, so it's the hard cap.
const int MAX_SPEED = 160;

void setup() {
  Serial.begin(115200);

  // Configure all motor pins as outputs
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  
  pinMode(STBY, OUTPUT);

  // Enable the motor driver (Must be HIGH for motors to spin)
  digitalWrite(STBY, HIGH);
  
  // Make sure motors are stopped on boot
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("TARGET IP (Enter this in your Dashboard): ");
  Serial.println(WiFi.localIP());

  // Setup the web server route
  server.on("/drive", handleDrive);
  server.begin();
}

// Drives one motor at a signed speed: positive = forward, negative = reverse, 0 = stop.
void driveMotor(int pwmPin, int in1, int in2, int value) {
  if (value > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (value < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(pwmPin, abs(value));
}

void handleDrive() {
  if (server.hasArg("left") && server.hasArg("right")) {
    int leftVal = server.arg("left").toInt();
    int rightVal = server.arg("right").toInt();

    leftVal = constrain(leftVal, -MAX_SPEED, MAX_SPEED);
    rightVal = constrain(rightVal, -MAX_SPEED, MAX_SPEED);

    driveMotor(PWMA, AIN1, AIN2, leftVal);
    driveMotor(PWMB, BIN1, BIN2, rightVal);

    Serial.printf("Drive -> L: %d | R: %d\n", leftVal, rightVal);

    // Acknowledge the command so the laptop dashboard fetch() completes
    server.send(200, "text/plain", "OK");
  }
  else {
    // Failsafe if the endpoint is hit without both motor speeds
    Serial.println("WARNING: Ping received, but 'left'/'right' was missing!");
    server.send(400, "text/plain", "ERR: Missing left/right");
  }
}

void loop() {
  // Continuously listen for incoming dashboard commands
  server.handleClient();
}