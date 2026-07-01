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

void handleDrive() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    
    // Default speed is 0 if not provided by the dashboard
    int currentSpeed = 0; 
    
    // Capture the live speed from the EvoFox trigger data
    if (server.hasArg("speed")) {
      currentSpeed = server.arg("speed").toInt();
    }
    
    // Safety check: Ensure speed is within valid 8-bit PWM bounds (0-255)
    if (currentSpeed > 255) currentSpeed = 255;
    if (currentSpeed < 0) currentSpeed = 0;

    // --- DEBUGGING PRINT STATEMENTS ---
    Serial.print("Ping Received -> CMD: [");
    Serial.print(command);
    Serial.print("] | SPEED: [");
    Serial.print(currentSpeed);
    Serial.println("]");
    // ----------------------------------

    // --- MOVEMENT LOGIC ---
    if (command == "STOP") {
      analogWrite(PWMA, 0);
      analogWrite(PWMB, 0);
    }
    
    else if (command == "FORWARD") {
      // Changed to HIGH/LOW to match physical forward movement
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      
      analogWrite(PWMA, currentSpeed);
      analogWrite(PWMB, currentSpeed);
    } 
    else if (command == "BACKWARD") {
      // Changed to LOW/HIGH to match physical backward movement
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      
      analogWrite(PWMA, currentSpeed);
      analogWrite(PWMB, currentSpeed);
    }
    else if (command == "LEFT") {
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      
      analogWrite(PWMA, currentSpeed);
      analogWrite(PWMB, currentSpeed);
    } 
    else if (command == "RIGHT") {
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      
      analogWrite(PWMA, currentSpeed);
      analogWrite(PWMB, currentSpeed);
    } 
    
    // Acknowledge the command so the laptop dashboard fetch() completes
    server.send(200, "text/plain", "OK");
  } 
  else {
    // Failsafe if the endpoint is hit without a command
    Serial.println("WARNING: Ping received, but 'cmd' was missing!");
    server.send(400, "text/plain", "ERR: Missing CMD");
  }
}

void loop() {
  // Continuously listen for incoming dashboard commands
  server.handleClient();
}