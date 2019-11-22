#include <App.hpp>
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

RTC_DATA_ATTR int bootCounter = 0; // This var ist stored in ulp/rtc data
RTC_DATA_ATTR time_t allExecTime = 0; // This var ist stored in ulp/rtc data

WiFiClient wifiClient;
PubSubClient client(wifiClient);
time_t startTime = millis();

void setup()
{
  Serial.begin(115200);

  Serial.println("WiFi config.");

  WiFi.mode(WIFI_OFF);
  WiFi.disconnect(true);

  WiFi.config(
    IPAddress(IP_HOST), 
    IPAddress(IP_GATEWAY), 
    IPAddress(IP_NETMASK),
    IPAddress(IP_DNS1)
  );

  Serial.println("WiFi begin.");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(250);
    if ((millis() - startTime) > CONNECTION_TIMEOUT)
    {
      Serial.println("\n\n*** WiFi connection timeout ***\n\n");
      ESP.restart();
    }
  }

  Serial.println("\nWiFi connected.");

  client.setServer(MQTT_HOST, MQTT_PORT);
  Serial.println("Connecting to MQTT server.");
}

void loop()
{

#ifdef MQTT_USEAUTH
    client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD);
#else
    client.connect(MQTT_CLIENTID);
#endif

  if (client.connected())
  {
    Serial.println("MQTT server connected.");

    char buffer[64];
    time_t endTime = millis();
    bootCounter++;
    allExecTime += endTime - startTime;
    sprintf(buffer, "%d %lums %lums", bootCounter, endTime - startTime, 
       allExecTime / bootCounter );

    client.publish(MQTT_OUTTOPIC, buffer, true);
    delay(100);
    Serial.printf("%s : value send.\n", buffer);

    ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  }
  else
  {
    if ((millis() - startTime) > CONNECTION_TIMEOUT)
    {
      Serial.println("\n\n*** MQTT connection timeout ***\n\n");
      ESP.restart();
    }
  }

  delay(500);
}
