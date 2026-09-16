/*
 * =========================================================================
 * Project: ESP32 4-Seat Smart Booking System (IoT Firmware)
 * Hardware: ESP32 + 4 Analog Force/IR Sensors + 4x4 Keypad Matrix
 * Communication: Wi-Fi HTTP POST & WebSockets to Node.js Backend
 * =========================================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>

// --- Wi-Fi NETWORK CREDENTIALS ---
const char* ssid     = "YOUR_WIFI_SSID";     // Change to your Wi-Fi name
const char* password = "YOUR_WIFI_PASSWORD"; // Change to your Wi-Fi password

// --- BACKEND SERVER CONFIGURATION ---
// Replace with your PC's local IP address (e.g. http://192.168.1.50:3000)
const char* serverUrlUpdate  = "http://192.168.1.100:3000/api/seats/update";
const char* serverUrlKeypad  = "http://192.168.1.100:3000/api/seats/keypad";

// --- SENSOR CONFIGURATION (ESP32 ADC1 Pins) ---
const int NUM_SEATS = 4;
const int sensorPins[NUM_SEATS] = {34, 35, 32, 33}; // GPIO 34 (S1), 35 (S2), 32 (S3), 33 (S4)
const int THRESHOLD = 500;                          // Analog threshold (0-4095 on ESP32): >500 = Occupied

// Store previous state to send updates only when state changes
int lastSeatState[NUM_SEATS] = {-1, -1, -1, -1};

// --- 4x4 KEYPAD CONFIGURATION ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// GPIO pins safe for Keypad on ESP32
byte rowPins[ROWS] = {13, 12, 14, 27}; 
byte colPins[COLS] = {26, 25, 18, 19}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================================");
  Serial.println("   ESP32 Smart Seat Booking System Starting...   ");
  Serial.println("=================================================");

  // Connect to Wi-Fi
  connectToWiFi();
}

void loop() {
  // Maintain Wi-Fi Connection
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // 1. Read sensors and check for seat status changes
  checkSeatSensors();

  // 2. Read keypad input
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print("\n[Keypad Input Detected]: ");
    Serial.println(key);

    if (key >= '1' && key <= '4') {
      int requestedSeats = key - '0';
      sendKeypadBookingRequest(requestedSeats);
    } else {
      Serial.println("Invalid Key! Please select a digit between 1 and 4.");
    }
  }

  delay(200); // Poll delay
}

// Function to connect ESP32 to Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wi-Fi Connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n⚠️ Wi-Fi Connection Failed! Will retry in loop.");
  }
}

// Read sensors and send update to Web Backend if status changes
void checkSeatSensors() {
  int currentSeatState[NUM_SEATS];
  bool stateChanged = false;
  int availableCount = 0;

  for (int i = 0; i < NUM_SEATS; i++) {
    int val = analogRead(sensorPins[i]);
    // 0 = Available, 1 = Occupied
    currentSeatState[i] = (val > THRESHOLD) ? 1 : 0;

    if (currentSeatState[i] == 0) {
      availableCount++;
    }

    if (currentSeatState[i] != lastSeatState[i]) {
      stateChanged = true;
    }
  }

  // Send update to web backend if seat state changed
  if (stateChanged && WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < NUM_SEATS; i++) {
      lastSeatState[i] = currentSeatState[i];
    }
    
    sendSeatUpdateToBackend(currentSeatState, availableCount);
  }
}

// HTTP POST request sending live seat array to Node.js backend
void sendSeatUpdateToBackend(int seats[], int availableCount) {
  HTTPClient http;
  http.begin(serverUrlUpdate);
  http.addHeader("Content-Type", "application/json");

  // Create JSON Payload: {"s1":0, "s2":1, "s3":0, "s4":0, "available":3}
  String jsonPayload = "{";
  jsonPayload += "\"s1\":" + String(seats[0]) + ",";
  jsonPayload += "\"s2\":" + String(seats[1]) + ",";
  jsonPayload += "\"s3\":" + String(seats[2]) + ",";
  jsonPayload += "\"s4\":" + String(seats[3]) + ",";
  jsonPayload += "\"available\":" + String(availableCount);
  jsonPayload += "}";

  Serial.print("Sending Seat Telemetry to Backend: ");
  Serial.println(jsonPayload);

  int httpCode = http.POST(jsonPayload);
  if (httpCode > 0) {
    Serial.print("Server Response Code: ");
    Serial.println(httpCode);
  } else {
    Serial.print("HTTP POST Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

// HTTP POST request sending Keypad booking request
void sendKeypadBookingRequest(int requestedSeats) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(serverUrlKeypad);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"requestedSeats\":" + String(requestedSeats) + "}";
  Serial.print("Sending Keypad Request to Backend: ");
  Serial.println(jsonPayload);

  int httpCode = http.POST(jsonPayload);
  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("Server Response: ");
    Serial.println(response);
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}
