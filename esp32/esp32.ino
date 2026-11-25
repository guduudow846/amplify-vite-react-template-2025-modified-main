
#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <DHT.h>

// =================================
// DHT11 SENSOR
// =================================
#define DHTPIN D3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =================================
// AWS MQTT SETTINGS
// =================================
#define AWS_IOT_PUBLISH_TOPIC   "/telemetry"
#define AWS_IOT_SUBSCRIBE_TOPIC "/downlink"

unsigned long sendInterval = 60000;
const unsigned long reconnectInterval = 5000;

char THINGNAME[17];

WiFiClientSecure net;
MQTTClient client(1024);

unsigned long previousMillis = 0;
unsigned long lastReconnectAttempt = 0;

bool awsConnected = false;

// =================================
// MQTT MESSAGE HANDLER
// =================================
void messageHandler(String &topic, String &payload) {

  Serial.println();
  Serial.println("========== MQTT MESSAGE ==========");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(payload);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON Error: ");
    Serial.println(error.c_str());
  } else {

    Serial.println("JSON received successfully!");

    if (topic.endsWith("shadow/update/delta")) {

      if (doc["state"].containsKey("sendInterval")) {

        sendInterval = doc["state"]["sendInterval"].as<long>();

        Serial.print("Nytt intervall från Shadow: ");
        Serial.println(sendInterval);

        char updateTopic[128];

        snprintf(updateTopic, sizeof(updateTopic),
                 "$aws/things/%s/shadow/update", THINGNAME);

        char reportedPayload[128];

        snprintf(reportedPayload, sizeof(reportedPayload),
                 "{\"state\":{\"reported\":{\"sendInterval\":%lu}}}",
                 sendInterval);

        client.publish(updateTopic, reportedPayload);

        Serial.println("Shadow updated (reported)!");
      }
    }
  }

  Serial.println("==================================");
}

// =================================
// THING NAME
// =================================
void setThingName() {
  strcpy(THINGNAME, "esp32-hassan");
}

// =================================
// WIFI CONNECTION
// =================================
bool connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 30000) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection FAILED!");
    return false;
  }

  Serial.println("WiFi connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  return true;
}

// =================================
// NTP TIME SYNCHRONIZATION
// =================================
bool syncTime() {

  Serial.println();
  Serial.println("Synchronizing time...");

  configTime(0, 0, "pool.ntp.org", "time.google.com");

  time_t now = time(nullptr);

  unsigned long start = millis();

  while (now < 1700000000 &&
         millis() - start < 30000) {

    delay(500);
    Serial.print(".");

    now = time(nullptr);
  }

  Serial.println();

  if (now < 1700000000) {
    Serial.println("NTP synchronization FAILED!");
    return false;
  }

  Serial.println("Time synchronized:");
  Serial.println(ctime(&now));

  Serial.print("Unix timestamp: ");
  Serial.println((long)now);

  return true;
}

// =================================
// TLS CONNECTION TEST
// =================================
bool testAWS_TLS() {

  Serial.println();
  Serial.println("========== TLS TEST ==========");

  WiFiClientSecure tlsTest;

  tlsTest.setCACert(AWS_CERT_CA);
  tlsTest.setCertificate(AWS_CERT_CRT);
  tlsTest.setPrivateKey(AWS_CERT_PRIVATE);

  Serial.println("Connecting directly to AWS TLS...");

  Serial.print("Endpoint: ");
  Serial.println(AWS_IOT_ENDPOINT);

  Serial.println("Port: 8883");

  bool result = tlsTest.connect(AWS_IOT_ENDPOINT, 8883);

  if (result) {

    Serial.println();
    Serial.println("TLS CONNECTION SUCCESSFUL!");
    Serial.println("Certificate handshake completed.");

    tlsTest.stop();

    Serial.println("==============================");

    return true;

  } else {

    Serial.println();
    Serial.println("TLS CONNECTION FAILED!");

    char errorBuffer[200];

    int errorCode = tlsTest.lastError(
      errorBuffer,
      sizeof(errorBuffer)
    );

    Serial.print("TLS Error code: ");
    Serial.println(errorCode);

    Serial.print("TLS Error message: ");
    Serial.println(errorBuffer);

    tlsTest.stop();

    Serial.println("==============================");

    return false;
  }
}

// =================================
// AWS CERTIFICATE SETUP
// =================================
void setupAWSCertificates() {

  Serial.println();
  Serial.println("Loading AWS certificates...");

  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  Serial.println("TLS certificates loaded!");
}

// =================================
// CONNECT TO AWS IOT
// =================================
bool connectAWS() {

  if (!connectWiFi()) {
    return false;
  }

  setThingName();

  Serial.println();
  Serial.println("ESP32 Thing Name:");
  Serial.println(THINGNAME);

  if (!syncTime()) {
    return false;
  }

  setupAWSCertificates();

  Serial.print("AWS Endpoint: ");
  Serial.println(AWS_IOT_ENDPOINT);

  if (!testAWS_TLS()) {

    Serial.println();
    Serial.println("AWS TLS TEST FAILED!");
    Serial.println("MQTT connection will not be attempted.");

    return false;
  }

  Serial.println();
  Serial.println("Starting MQTT client...");

  net.stop();

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(messageHandler);

  Serial.println("Connecting to AWS IoT...");

  if (!client.connect(THINGNAME)) {

    Serial.println();
    Serial.println("AWS MQTT CONNECTION FAILED!");

    Serial.print("MQTT Error: ");
    Serial.println(client.lastError());

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    Serial.print("TLS socket connected: ");
    Serial.println(net.connected());

    return false;
  }

  Serial.println();
  Serial.println("AWS MQTT CONNECTED!");

  char topic[128];

  snprintf(topic, sizeof(topic),
           "%s%s",
           THINGNAME,
           AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.print("Subscribing to: ");
  Serial.println(topic);

  if (!client.subscribe(topic)) {

    Serial.println("MQTT subscription FAILED!");

    client.disconnect();

    return false;
  }

  Serial.println("MQTT subscription successful!");

  snprintf(topic, sizeof(topic),
           "$aws/things/%s/shadow/get/accepted", THINGNAME);
  client.subscribe(topic);

  snprintf(topic, sizeof(topic),
           "$aws/things/%s/shadow/get/rejected", THINGNAME);
  client.subscribe(topic);

  snprintf(topic, sizeof(topic),
           "$aws/things/%s/shadow/update/delta", THINGNAME);
  client.subscribe(topic);

  snprintf(topic, sizeof(topic),
           "$aws/things/%s/shadow/get", THINGNAME);
  client.publish(topic, "");

  Serial.println("Shadow subscriptions + get request sent!");

  Serial.println();
  Serial.println("================================");
  Serial.println(" AWS IOT CONNECTION SUCCESSFUL");
  Serial.println("================================");

  awsConnected = true;

  return true;
}

// =================================
// SEND DHT11 TELEMETRY TO AWS
// =================================
bool sendTelemetry() {

  if (!client.connected()) {

    Serial.println("MQTT disconnected!");

    awsConnected = false;

    return false;
  }

  Serial.println();
  Serial.println("Reading DHT11 sensor...");

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("DHT11 SENSOR READ FAILED!");
    Serial.println("Check wiring and sensor connection.");

    return false;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  JsonDocument doc;

  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  String payload;

  serializeJson(doc, payload);

  char topic[64];

  snprintf(topic, sizeof(topic),
           "%s%s",
           THINGNAME,
           AWS_IOT_PUBLISH_TOPIC);

  Serial.println();
  Serial.println("========== TELEMETRY ==========");

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  bool result = client.publish(topic, payload);

  if (result) {
    Serial.println("Telemetry published successfully!");
  } else {
    Serial.println("Telemetry publishing FAILED!");
  }

  Serial.println("================================");

  return result;
}

// =================================
// ARDUINO SETUP
// =================================
void setup() {

  Serial.begin(115200);

  pinMode(0, INPUT_PULLUP);

  delay(2000);

  dht.begin();

  Serial.println("DHT11 sensor started on D3.");

  Serial.println();
  Serial.println("================================");
  Serial.println(" STARTING ESP32 AWS IOT");
  Serial.println("================================");

  awsConnected = connectAWS();

  if (!awsConnected) {

    Serial.println();
    Serial.println("Initial AWS connection failed.");
    Serial.println("Will retry automatically.");
  }
}

// =================================
// ARDUINO LOOP
// =================================
void loop() {

  if (WiFi.status() != WL_CONNECTED ||
      !client.connected()) {

    awsConnected = false;

    if (millis() - lastReconnectAttempt >=
        reconnectInterval) {

      lastReconnectAttempt = millis();

      Serial.println();
      Serial.println("Reconnecting to AWS...");

      client.disconnect();
      net.stop();

      awsConnected = connectAWS();

      if (awsConnected) {

        previousMillis = 0;

        Serial.println("Reconnection successful!");
      }
    }

    delay(10);

    return;
  }

  // Process incoming MQTT messages
  client.loop();

  // BOOT button: send telemetry immediately
  if (digitalRead(0) == LOW) {

    delay(50);

    if (digitalRead(0) == LOW) {

      Serial.println("Button pressed! Sending telemetry...");

      sendTelemetry();

      while (digitalRead(0) == LOW) {
        delay(10);
      }
    }
  }

  // Send telemetry at selected interval
  if (millis() - previousMillis >= sendInterval ||
      previousMillis == 0) {

    previousMillis = millis();

    sendTelemetry();
  }

  delay(10);
}
