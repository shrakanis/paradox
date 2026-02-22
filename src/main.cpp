#include "paradox.h"
#include <ArduinoHA.h>
#include <WebServer_WT32_ETH01.h>
#include <WebServer_WT32_ETH01.hpp>
#include <ElegantOTA.h>
#include "settings.h"

//#define BROKER_ADDR IPAddress(192,168,0,17)

String broker_ip="192.168.0.155";
String broker_port="1883";
String broker_user="mqtt";
String broker_pass="mqtt8412";

#define LOG_LINES 20
#define LOG_LENGTH 120

String logBuffer[LOG_LINES];
int logIndex = 0;

void addLog(const String &msg)
{
  Serial.println(msg);

  logBuffer[logIndex] = msg;
  logIndex = (logIndex + 1) % LOG_LINES;
}

WebServer server(80);
// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);
IPAddress BROKER_ADDR(192,168,0,155);

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

WiFiClient client;
HADevice device(mac, sizeof(mac));

unsigned long lastAvailabilityToggleAt = millis();
unsigned long old;
unsigned long delayTime;

HAMqtt mqtt(client, device);

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
    // This callback is called when message from MQTT broker is received.
    // Please note that you should always verify if the message's topic is the one you expect.
    // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }

    Serial.print("New message on topic: ");
    Serial.println(topic);
    Serial.print("Data: ");
    Serial.println((const char*)payload);
    code = String((const char*)payload);
}

void onMqttConnected() {
    Serial.println("Connected to the broker!");

    // You can subscribe to custom topic if you need
    mqtt.subscribe("mastercode");
}

void onMqttDisconnected() {
    Serial.println("Disconnected from the broker!");
}

void onMqttStateChanged(HAMqtt::ConnectionState state) {
    Serial.print("MQTT state changed to: ");
    Serial.println(static_cast<int8_t>(state));
}

HABinarySensor valve("z1");
HABinarySensor valve2("z2");
HABinarySensor valve3("z3");
HABinarySensor valve4("z4");
HABinarySensor valve5("z5");
HABinarySensor valve6("z6");
HABinarySensor valve7("z7");
HABinarySensor valve8("z8");
HABinarySensor valve9("z9");

HASensorNumber zoneinalarm("ZoneinAlarm");
HASensorNumber zonerestore("ZoneRestore");
HASensorNumber userdisarmafteralarm("UserDisarmAfterAlarm");

Paradox paradox(Serial1, 9600);
byte group, event, area;

void setup() {

  //ElegantOTA.setAutoReboot(true);
  byte mac[ETH_PHY_ADDR];
  device.setUniqueId(mac, sizeof(mac));

  Serial.begin(9600);
  Serial.println("Begin setup");
  
  paradox.begin();
  Serial.println("Paradox setup done");
 
  ElegantOTA.setAutoReboot(true);
  ElegantOTA.begin(&server);    // Start ElegantOTA
  
  // To be called before ETH.begin()
  WT32_ETH01_onEvent();
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);
  WT32_ETH01_waitForConnect();

  device.setName("Paradox");
  device.setSoftwareVersion("1.0.0");

  device.enableSharedAvailability();
  device.enableLastWill();
  mqtt.onMessage(onMqttMessage);
  mqtt.onConnected(onMqttConnected);
  mqtt.onDisconnected(onMqttDisconnected);
  mqtt.onStateChanged(onMqttStateChanged);
  mqtt.begin(BROKER_ADDR, 1883, "mqtt", "mqtt8412");
  
  delayTime=15000;
  old=millis();

  server.on("/reboot", HTTP_POST, []() {

  server.send(200, "text/plain", "Rebooting...");
  addLog("Reboot requested from web");

  delay(500);   // allow response to be sent
  ESP.restart();
  });

  server.on("/logs", HTTP_GET, []() {

  String output = "";

  int index = logIndex;

  for (int i = 0; i < LOG_LINES; i++) {
    int pos = (index + i) % LOG_LINES;
    if (logBuffer[pos].length() > 0) {
      output += logBuffer[pos] + "\n";
    }
  }

  server.send(200, "text/plain", output);
});
   server.begin();
}

void loop() 
{
  mqtt.loop();
  server.handleClient();
  ElegantOTA.loop();
  
  //ElegantOTA.loop();
  int status = paradox.read(group, event, area);
  if(status ==0)
  {
    if(event==PARADOX_ZONE_IN_ALARM)
    { 
      Serial.println("Zone in alarm" + SECURITY_ZONE_NAMES[event]);
      zoneinalarm.setValue(event);
    } 
    else if (event == PARADOX_DISARM_AFTER_ALARM) 
    {
      Serial.println("System disarmed after alarm by user: " + event);
      userdisarmafteralarm.setValue(event);
    } 
    else if (event == PARADOX_ZONE_ALARM_RESTOR) 
    {
      Serial.println("Zone restored: " + event);
      zonerestore.setValue(event);
    } 
    else if (event == PARADOX_TROUBLE) 
    {
      Serial.println("Trouble with system: " + PARADOX_TROUBLE_CODES[event]); 
    } 
    else if (event == PARADOX_TROUBLE_OK) 
    {
      Serial.println("Trouble restored: " + PARADOX_TROUBLE_CODES[event]); 
    }
    if(millis()-old >= delayTime)
    {
      old=millis();
    }
  }
}