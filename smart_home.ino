\begin{lstlisting}[language=C++, caption={Código do ESP32}]
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ======= CONFIG WIFI & MQTT =======
const char* ssid = "DoNotConnect";
const char* password = "";
const char* mqtt_server = "192.168.162.58";

WiFiClient espClient;
PubSubClient client(espClient);

// ======= OLED SH1107 =======
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// ======= SENSORS =======
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define DS18B20_PIN 5
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

#define PIR_PIN 25
#define LDR_PIN 34

// ======= ACTUATORS =======
Servo fanServo;
Servo windowServo;
#define FAN_SERVO_PIN 19
#define WINDOW_SERVO_PIN 18
#define LED_PIN 14

// ======= STATES =======
String fanState = "OFF";
String windowState = "CLOSED";
String lightState = "OFF";
String mode = "auto";

// ======= CALLBACK MQTT =======
void callback(char* topic, byte* payload, unsigned int length) {
  String command;
  for (int i = 0; i < length; i++) command += (char)payload[i];

  if (String(topic) == "home/fan/set") {
    fanState = (command == "ON") ? "ON" : "OFF";
  }

  if (String(topic) == "home/window/set") {
    windowState = (command == "OPEN") ? "OPEN" : "CLOSED";
    windowServo.write(windowState == "OPEN" ? 180 : 0);
  }

  if (String(topic) == "home/light/set") {
    lightState = (command == "ON") ? "ON" : "OFF";
    digitalWrite(LED_PIN, lightState == "ON" ? HIGH : LOW);
  }

  if (String(topic) == "home/mode") {
    if (command == "manual" || command == "auto") {
      mode = command;
      Serial.println("Mode Updated to: " + mode);
    }
  }
  
}

// ======= RECONNECT MQTT =======
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("home/fan/set");
      client.subscribe("home/window/set");
      client.subscribe("home/light/set");
      client.subscribe("home/mode");

    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  ds18b20.begin();

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  fanServo.attach(FAN_SERVO_PIN);
  windowServo.attach(WINDOW_SERVO_PIN);
  fanServo.write(90);
  windowServo.write(0);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Wire.begin(21, 23); // SDA, SCL
  if (!display.begin(0x3C, true)) {
    Serial.println("OLED init failed");
    while (true);
  }
  display.setRotation(1);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sistema Iniciado!");
  display.display();
}

// ======= LOOP =======
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  static unsigned long lastRead = 0;
  static unsigned long lastFanMove = 0;
  static int fanPos = 90;
  static int fanDir = 1;

  unsigned long now = millis();

  // FAN MOVIMENTO OSILATÓRIO
  if (fanState == "ON") {
    if (now - lastFanMove > 30) {
      fanPos += fanDir * 2;
      if (fanPos >= 120 || fanPos <= 60) fanDir *= -1;
      fanServo.write(fanPos);
      lastFanMove = now;
    }
  } else {
    fanServo.write(90); // parado
  }

  // PUBLICAÇÃO E OLED a cada 2 segundos
  if (now - lastRead > 2000) {
    lastRead = now;

    float tempIn = dht.readTemperature();
    float hum = dht.readHumidity();
    ds18b20.requestTemperatures();
    float tempOut = ds18b20.getTempCByIndex(0);
    int lux = 4095 - analogRead(LDR_PIN);
    bool motion = digitalRead(PIR_PIN);

    // MQTT: sensores
    String sensorPayload = "{";
    sensorPayload += "\"temp_in\":" + String(tempIn) + ",";
    sensorPayload += "\"temp_out\":" + String(tempOut) + ",";
    sensorPayload += "\"humidity\":" + String(hum) + ",";
    sensorPayload += "\"motion\":" + String(motion ? "true" : "false") + ",";
    sensorPayload += "\"lux\":" + String(lux);
    sensorPayload += "}";

    client.publish("home/sensors", sensorPayload.c_str());

    // MQTT: status
    String statusPayload = "{";
    statusPayload += "\"fan\":\"" + fanState + "\",";
    statusPayload += "\"window\":\"" + windowState + "\"";
    statusPayload += "}";

    client.publish("home/status", statusPayload.c_str());

    // OLED DISPLAY
    display.clearDisplay();
    display.setCursor(0, 0);

    display.print("MQTT: ");
    display.println(client.connected() ? "ON" : "OFF");

    display.print("Modo: ");
    display.println(mode == "manual" ? "MANUAL" : "AUTO");

    display.print("IN: ");
    display.print(tempIn, 1); display.println(" C");

    display.print("OUT: ");
    display.println(tempOut == -127.0 ? "N/A" : String(tempOut, 1) + " C");

    display.print("Hum: ");
    display.print(hum, 1); display.println(" %");

    display.print("LUX: ");
    display.println(lux);

    display.print("MOTION: ");
    display.println(motion ? "SIM" : "NAO");

    display.print("Fan: ");
    display.print(fanState);
    display.print(" Win: ");
    display.println(windowState);

    display.print("LED: ");
    display.println(lightState);

    display.display();

    Serial.println(sensorPayload);
    Serial.println(statusPayload);
  }
}

\end{lstlisting}