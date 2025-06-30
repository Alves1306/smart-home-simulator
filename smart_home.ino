#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>

// --- Wi-Fi ---
const char* ssid = "LBEC3 2";      // Open network
const char* password = "";         // No password
const char* mqtt_server = "192.168.68.128";  // Replace with your broker's IP

WiFiClient espClient;
PubSubClient client(espClient);

// --- DHT22 Temperature & Humidity Sensor ---
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- DS18B20 External Temperature Sensor ---
#define DS18B20_PIN 5
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

// --- Light and Motion Sensors ---
#define LDR_PIN 34
#define PIR_PIN 25

// --- Servo Motors ---
Servo windowServo;
Servo fanServo;
#define WINDOW_SERVO_PIN 18
#define FAN_SERVO_PIN 19

// --- LED ---
#define LED_PIN 14
#define luxThreshold 300

unsigned long lastMotionTime = 0;
bool ledOn = false;

// --- Humidity Logic ---
unsigned long humidityStart = 0;
unsigned long fanHoldStart = 0;
bool fanHoldActive = false;

// --- Sensor Data ---
float tempIn = 0, humIn = 0, tempOut = 0;

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Climate")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  dht.begin();
  ds18b20.begin();
  windowServo.attach(WINDOW_SERVO_PIN);
  fanServo.attach(FAN_SERVO_PIN);

  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastSensorRead = 0;
  unsigned long now = millis();

  if (now - lastSensorRead >= 2000) {
    tempIn = dht.readTemperature();
    humIn = dht.readHumidity();
    ds18b20.requestTemperatures();
    tempOut = ds18b20.getTempCByIndex(0);
    lastSensorRead = now;

    // Publish JSON to MQTT
    String payload = "{\"temp\":";
    payload += tempIn;
    payload += ",\"hum\":";
    payload += humIn;
    payload += ",\"lux\":";
    payload += 4095 - analogRead(LDR_PIN);
    payload += "}";

    client.publish("climate/data", payload.c_str());
    Serial.println("Published MQTT: " + payload);
  }

  int luxRaw = analogRead(LDR_PIN);
  int lux = 4095 - luxRaw;
  bool motionDetected = digitalRead(PIR_PIN);

  if (isnan(tempIn) || isnan(humIn) || tempOut == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor read error");
    return;
  }

  if (motionDetected) {
    lastMotionTime = now;
  }

  bool ledShouldBeOn = (lux < luxThreshold && (now - lastMotionTime <= 10000));
  analogWrite(LED_PIN, ledShouldBeOn ? 255 : 0);
  ledOn = ledShouldBeOn;

  // Humidity conditions
  if (humIn > 75) {
    if (humidityStart == 0) humidityStart = now;
    if ((now - humidityStart >= 15UL * 60 * 1000) && !fanHoldActive) {
      fanHoldActive = true;
      fanHoldStart = now;
    }
  } else {
    humidityStart = 0;
    if (humIn < 60) {
      fanHoldActive = false;
    }
  }

  // Fan logic
  if (((humIn > 70 || tempIn > 28) && (tempOut < 18 || tempOut > 24)) || fanHoldActive) {
    fanServo.write(0);
  } else {
    fanServo.write(90);
  }

  if (fanHoldActive && (now - fanHoldStart >= 30UL * 60 * 1000)) {
    fanHoldActive = false;
  }

  // Window logic
  if ((humIn > 65 || tempIn > 25) && (tempOut >= 18 && tempOut <= 24)) {
    windowServo.write(180);
  } else {
    windowServo.write(0);
  }

  // Debug Output
  Serial.println("Smart Climate Data:");
  Serial.print("Indoor Temp: ");
  Serial.print(tempIn); Serial.print("C, Hum: ");
  Serial.print(humIn); Serial.print("%, Outdoor Temp: ");
  Serial.println(tempOut);

  Serial.print("Lux: ");
  Serial.println(lux);
  Serial.print("Motion: ");
  Serial.println(motionDetected ? "YES" : "NO");

  Serial.print("LED: ");
  Serial.println(ledShouldBeOn ? "ON" : "OFF");
}
  
