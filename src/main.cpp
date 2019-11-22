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

#ifdef DEBUG_MSG
  Serial.println("WiFi config.");
#endif

  WiFi.mode(WIFI_OFF);
  WiFi.disconnect(true);

  WiFi.config(
    IPAddress(IP_HOST), 
    IPAddress(IP_GATEWAY), 
    IPAddress(IP_NETMASK),
    IPAddress(IP_DNS1)
  );

#ifdef DEBUG_MSG
  Serial.println("WiFi begin.");
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
#ifdef DEBUG_MSG
    Serial.print('.');
#endif
    delay(250);
    if ((millis() - startTime) > CONNECTION_TIMEOUT)
    {
      Serial.println("\n\n*** WiFi connection timeout ***\n\n");
      ESP.restart();
    }
  }

#ifdef DEBUG_MSG
  Serial.println("\nWiFi connected.");
#endif

  client.setServer(MQTT_HOST, MQTT_PORT);
#ifdef DEBUG_MSG
  Serial.println("Connecting to MQTT server.");
#endif
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
#ifdef DEBUG_MSG
    Serial.println("MQTT server connected.");
#endif
    char buffer[64];
    time_t endTime = millis();
    bootCounter++;
    allExecTime += endTime - startTime;
    sprintf(buffer, "%d %lums %lums", bootCounter, endTime - startTime, 
       allExecTime / bootCounter );

    client.publish(MQTT_OUTTOPIC, buffer, true);
    delay(100);
#ifdef DEBUG_MSG
    Serial.printf("%s : value send.\n", buffer);
#endif

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
