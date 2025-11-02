#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 26
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define LED_PIN 2          // Built-in LED for status indication

// WiFi credentials - CHANGE THESE!
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Soil moisture calibration (adjust based on your sensor)
const int DRY_VALUE = 4095;    // Sensor value in dry soil
const int WET_VALUE = 1500;     // Sensor value in wet soil

WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

// Sensor data
float temperature = 0;
float humidity = 0;
int soilMoisture = 0;
bool sensorError = false;

// Thresholds for alerts
const int MOISTURE_LOW = 30;
const int MOISTURE_HIGH = 70;
const float TEMP_HIGH = 35.0;

// HTML page with enhanced styling and status indicators
String htmlPage() {
  String status = "OK";
  String statusColor = "#4CAF50";
  
  if (sensorError) {
    status = "Sensor Error";
    statusColor = "#f44336";
  } else if (soilMoisture < MOISTURE_LOW) {
    status = "Low Moisture - Irrigation Needed";
    statusColor = "#ff9800";
  } else if (temperature > TEMP_HIGH) {
    status = "High Temperature Alert";
    statusColor = "#ff9800";
  }
  
  String moistureColor = "#4CAF50";
  if (soilMoisture < MOISTURE_LOW) moistureColor = "#f44336";
  else if (soilMoisture < MOISTURE_HIGH) moistureColor = "#ff9800";
  
  String page = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<meta http-equiv='refresh' content='5'>";
  page += "<title>Irrigation Monitor</title>";
  page += "<style>";
  page += "body{font-family:'Segoe UI',Arial,sans-serif;text-align:center;";
  page += "background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);";
  page += "margin:0;padding:20px;min-height:100vh;}";
  page += "h2{color:#fff;margin:20px 0;text-shadow:2px 2px 4px rgba(0,0,0,0.3);}";
  page += ".container{max-width:600px;margin:0 auto;}";
  page += ".status{background:" + statusColor + ";color:#fff;padding:15px;";
  page += "border-radius:10px;margin:20px 0;font-weight:bold;font-size:1.2rem;";
  page += "box-shadow:0 4px 6px rgba(0,0,0,0.2);}";
  page += ".card{background:#fff;padding:25px;border-radius:15px;margin:15px 0;";
  page += "box-shadow:0 8px 16px rgba(0,0,0,0.2);transition:transform 0.2s;}";
  page += ".card:hover{transform:translateY(-5px);}";
  page += ".reading{display:flex;justify-content:space-between;align-items:center;";
  page += "margin:15px 0;padding:15px;border-radius:8px;background:#f5f5f5;}";
  page += ".label{font-size:1.1rem;color:#555;font-weight:600;}";
  page += ".value{font-size:1.6rem;font-weight:bold;color:#333;}";
  page += ".progress{background:#e0e0e0;border-radius:10px;height:20px;margin:10px 0;overflow:hidden;}";
  page += ".progress-bar{height:100%;background:" + moistureColor + ";transition:width 0.3s;}";
  page += ".icon{font-size:2rem;margin-right:10px;}";
  page += ".footer{color:#fff;margin-top:30px;opacity:0.8;}";
  page += "@media(max-width:600px){.value{font-size:1.3rem;}}";
  page += "</style></head><body>";
  page += "<div class='container'>";
  page += "<h2>🌱 ESP32 Irrigation Monitor</h2>";
  page += "<div class='status'>" + status + "</div>";
  
  page += "<div class='card'>";
  page += "<div class='reading'>";
  page += "<span class='label'>🌡️ Temperature</span>";
  page += "<span class='value'>" + String(temperature, 1) + " °C</span>";
  page += "</div>";
  page += "<div class='reading'>";
  page += "<span class='label'>💧 Humidity</span>";
  page += "<span class='value'>" + String(humidity, 1) + " %</span>";
  page += "</div>";
  page += "<div class='reading' style='flex-direction:column;align-items:stretch;'>";
  page += "<div style='display:flex;justify-content:space-between;'>";
  page += "<span class='label'>🌾 Soil Moisture</span>";
  page += "<span class='value'>" + String(soilMoisture) + " %</span>";
  page += "</div>";
  page += "<div class='progress'>";
  page += "<div class='progress-bar' style='width:" + String(soilMoisture) + "%'></div>";
  page += "</div>";
  page += "</div>";
  page += "</div>";
  
  page += "<div class='footer'>";
  page += "IP: " + WiFi.localIP().toString() + " | ";
  page += "Uptime: " + String(millis() / 1000) + "s<br>";
  page += "Auto-refresh every 5 seconds";
  page += "</div>";
  page += "</div></body></html>";
  
  return page;
}

void handleRoot() {
  // Read DHT sensor with error handling
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    sensorError = true;
    temperature = 0;
    humidity = 0;
    Serial.println("Failed to read from DHT sensor!");
  } else {
    sensorError = false;
  }
  
  // Read and calibrate soil moisture
  int rawValue = analogRead(SOIL_PIN);
  soilMoisture = map(constrain(rawValue, WET_VALUE, DRY_VALUE), 
                     DRY_VALUE, WET_VALUE, 0, 100);
  soilMoisture = constrain(soilMoisture, 0, 100);
  
  // Log readings
  Serial.printf("Temp: %.1f°C, Humidity: %.1f%%, Soil: %d%% (Raw: %d)\n", 
                temperature, humidity, soilMoisture, rawValue);
  
  server.send(200, "text/html", htmlPage());
}

// JSON API endpoint for external integrations
void handleAPI() {
  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"soilMoisture\":" + String(soilMoisture) + ",";
  json += "\"sensorError\":" + String(sensorError ? "true" : "false") + ",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("\n\n=== ESP32 Irrigation Monitor ===");
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT11 sensor initialized");
  
  // Configure ADC for soil sensor
  analogSetAttenuation(ADC_11db);  // For full 0-3.3V range
  
  // Connect to WiFi with timeout
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Blink LED
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);  // LED on when connected
    Serial.println("\n✓ Connected to WiFi!");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n✗ Failed to connect to WiFi");
    Serial.println("Please check credentials and try again");
  }
  
  // Configure web server routes
  server.on("/", handleRoot);
  server.on("/api", handleAPI);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("================================\n");
}

void loop() {
  server.handleClient();
  
  // Reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.reconnect();
    delay(5000);
  }
  
  delay(10);  // Small delay to prevent watchdog issues
}