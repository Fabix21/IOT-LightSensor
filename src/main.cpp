#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Arduino.h>
#include <WifiManager.h>
#define wifi_ssid ""
#define wifi_password ""

#define mqtt_user ""
#define mqtt_password ""

#define lux_topic "sensor/lux"
#define lightbub_switch_power "cmnd/light/POWER"

#define maxLuxValue 100

const char* mqtt_server = "192.168.0.175";
const char* mqtt_port = "1883";
BH1750 lightMeter;
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lightMeter.begin();
  //setup_wifi();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server adress", mqtt_server, 40);
  wifiManager.addParameter(&custom_mqtt_server);

  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 40);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.autoConnect("IOT Light Sensor");

  mqtt_server = custom_mqtt_server.getValue();
  mqtt_port = custom_mqtt_port.getValue();
  int mqttPortInt = atol(mqtt_port);
 
  client.setServer(mqtt_server, mqttPortInt);
  Serial.print("mqtt server: ");
  Serial.printf(mqtt_server);
  Serial.print(" port: ");
  Serial.printf(mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
 
  while (!client.connected()) {
     Serial.print("mqtt server: ");
  Serial.printf(mqtt_server);
  Serial.print(" port: ");
  Serial.printf(mqtt_port);
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266 with BH1750", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed");
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
bool isEnoughLight(float lux){
   return (lux < maxLuxValue);
}

long lastMsg = 0;
float lux = 0.0;
float diff = 1.0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    float newLux = lightMeter.readLightLevel();
  
    if (checkBound(newLux, lux, diff)) {
      lux = newLux;
      Serial.print("New Lux:");
      Serial.println(String(lux).c_str());
      client.publish(lux_topic, String(lux).c_str(), true);
      client.publish(lux_topic, String(lux).c_str(), true);

     if(isEnoughLight(lux))
      client.publish(lightbub_switch_power, "ON", true);
     else
      client.publish(lightbub_switch_power, "OFF", true);
    }
  }
}
