/*
 * ESP32 Blynk Controller for Medicine Dispenser
 * Connects to Blynk and communicates with RP2040 via UART
 * 
 * Connections:
 * ESP32 TX (GPIO17) -> RP2040 RX (GPIO1)
 * ESP32 RX (GPIO16) -> RP2040 TX (GPIO0)
 * GND -> GND
 */

#define BLYNK_TEMPLATE_ID "TMPL6uqNOv6R-"
#define BLYNK_TEMPLATE_NAME "Mechatronics"
#define BLYNK_AUTH_TOKEN "OtfC5pu_mdoHZWE2ZOl65pzdo1OpkdsH"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "Mix";
char pass[] = "mix252004";

// UART pins for communication with RP2040
#define RXD2 16
#define TXD2 17

// Virtual pins for medicine counts
int med1_count = 0;  // V4
int med2_count = 0;  // V5
int med3_count = 0;  // V6

// Motor status (received from RP2040)
bool motor1_running = false;  // V1
bool motor2_running = false;  // V2
bool motor3_running = false;  // V3

// ============================================================
// BLYNK VIRTUAL PIN HANDLERS
// ============================================================

// Medicine 1 numeric input (V4)
BLYNK_WRITE(V4) {
  med1_count = param.asInt();
  Serial.print("Med 1 count set to: ");
  Serial.println(med1_count);
}

// Medicine 2 numeric input (V5)
BLYNK_WRITE(V5) {
  med2_count = param.asInt();
  Serial.print("Med 2 count set to: ");
  Serial.println(med2_count);
}

// Medicine 3 numeric input (V6)
BLYNK_WRITE(V6) {
  med3_count = param.asInt();
  Serial.print("Med 3 count set to: ");
  Serial.println(med3_count);
}

// Order button (V0)
BLYNK_WRITE(V0) {
  int buttonState = param.asInt();
  
  if (buttonState == 1) {
    // Button pressed - send command to RP2040
    sendCommandToRP2040();
  }
}

// ============================================================
// COMMUNICATION FUNCTIONS
// ============================================================

void sendCommandToRP2040() {
  // Format: 3-digit string (e.g., "123" for M1=1, M2=2, M3=3)
  char command[4];
  snprintf(command, sizeof(command), "%d%d%d", med1_count, med2_count, med3_count);
  
  Serial2.println(command);
  Serial.print("Sent to RP2040: ");
  Serial.println(command);
  
  // Reset button on Blynk
  Blynk.virtualWrite(V0, 0);
}

void checkRP2040Status() {
  // Check for status updates from RP2040
  while (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    message.trim();
    
    Serial.print("Received from RP2040: ");
    Serial.println(message);
    
    parseRP2040Message(message);
  }
}

void parseRP2040Message(String message) {
  // Expected format: "STATUS:M1=0,M2=1,M3=0"
  if (message.startsWith("STATUS:")) {
    message.remove(0, 7); // Remove "STATUS:"
    
    // Parse motor statuses
    int m1_idx = message.indexOf("M1=");
    int m2_idx = message.indexOf("M2=");
    int m3_idx = message.indexOf("M3=");
    
    if (m1_idx >= 0) {
      motor1_running = message.charAt(m1_idx + 3) == '1';
      Blynk.virtualWrite(V1, motor1_running ? 255 : 0);
    }
    
    if (m2_idx >= 0) {
      motor2_running = message.charAt(m2_idx + 3) == '1';
      Blynk.virtualWrite(V2, motor2_running ? 255 : 0);
    }
    
    if (m3_idx >= 0) {
      motor3_running = message.charAt(m3_idx + 3) == '1';
      Blynk.virtualWrite(V3, motor3_running ? 255 : 0);
    }
  }
}

// ============================================================
// WIFI CONNECTION FUNCTION
// ============================================================

void connectWiFi() {
  Serial.println("\n========================================");
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.println("========================================");
  
  WiFi.begin(ssid, pass);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempt++;
    
    if (attempt > 40) { // 20 seconds timeout
      Serial.println("\n[ERROR] WiFi connection failed!");
      Serial.println("Restarting in 5 seconds...");
      delay(5000);
      ESP.restart();
    }
  }
  
  Serial.println("\n========================================");
  Serial.println("WiFi Connected Successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("========================================\n");
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  // Debug serial
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor
  
  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("   ESP32 Medicine Dispenser v1.0");
  Serial.println("========================================");
  
  // UART communication with RP2040
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("[OK] UART initialized (TX:17, RX:16)");
  
  // Connect to WiFi
  connectWiFi();
  
  // Connect to Blynk
  Serial.println("Connecting to Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  int blynk_attempt = 0;
  while (!Blynk.connect()) {
    delay(500);
    Serial.print(".");
    blynk_attempt++;
    
    if (blynk_attempt > 20) { // 10 seconds timeout
      Serial.println("\n[ERROR] Blynk connection failed!");
      Serial.println("Check your Auth Token and internet connection.");
      Serial.println("Restarting in 5 seconds...");
      delay(5000);
      ESP.restart();
    }
  }
  
  Serial.println("\n[OK] Connected to Blynk");
  
  // Initialize all LEDs to off
  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
  
  Serial.println("\n========================================");
  Serial.println("      SYSTEM READY!");
  Serial.println("========================================\n");
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WARNING] WiFi disconnected! Reconnecting...");
    connectWiFi();
  }
  
  // Check Blynk connection
  if (!Blynk.connected()) {
    Serial.println("[WARNING] Blynk disconnected! Reconnecting...");
    Blynk.connect();
  }
  
  Blynk.run();
  checkRP2040Status();
  
  delay(10); // Small delay for stability
}
