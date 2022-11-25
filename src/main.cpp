#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <secrets.h>
#include "bmp_sensor.h"
#include <Wire.h>
#include <ArduinoJson.h>
#include "time.h"
#include "externs.h"
#include "web_server.h"

BearSSL::X509List client_crt(deviceCertificatePemCrt);
BearSSL::PrivateKey client_key(devicePrivatePemKey);
BearSSL::X509List rootCert(awsCaPemCrt);

WiFiClientSecure wiFiClient;
void msgReceived(char *topic, byte *payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient);
unsigned long lastPublish;
int msgCount;

struct tm timeinfo;
time_t now;

int temperature = 0;
int pressure = 0;
int altitude = 0;

struct Configuration
{
  int8_t TIME_ZONE;
  int temperatureShift;
  int pressureShift;
  int altitudeShift;
  int timeout;
  String localIp;
};
Configuration config{
    1,
    0,
    0,
    0,
    60 * 60 * 1000,
    "0.0.0.0",
};

unsigned long
getTime()
{
  time(&now);
  return now;
}

void setCurrentTime()
{
  configTime(config.TIME_ZONE * 3600, 0, "pl.pool.ntp.org", "europe.pool.ntp.org", "pool.ntp.org");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  gmtime_r(&now, &timeinfo);
  Serial.print("Current GMT time: ");
  Serial.println(asctime(&timeinfo));
}

void pubSubCheckConnect()
{
  if (!pubSubClient.connected())
  {
    Serial.print("PubSubClient connecting to: ");
    Serial.print(awsEndpoint);
    while (!pubSubClient.connected())
    {
      Serial.print("PubSubClient state:");
      Serial.println(pubSubClient.state());
      pubSubClient.connect(thingName);
    }
    Serial.println(" connected");
    pubSubClient.subscribe(thingMqttTopicIn);
  }
  pubSubClient.loop();
}

void measure()
{
  if (bmp.takeForcedMeasurement())
  {
    char outputData[256];
    temperature = bmp.readTemperature() + config.temperatureShift;
    pressure = bmp.readPressure() / 100 + config.pressureShift;
    altitude = bmp.readAltitude(1005) + config.altitudeShift;

    unsigned long epochTime;
    epochTime = getTime();

    sprintf(outputData, "{\"timestamp\": %ld, \"temperature\": %d, \"pressure\": %d, \"altitude\": %d}", epochTime, temperature, pressure, altitude);

    pubSubClient.publish(thingMqttTopicOut, outputData);

    Serial.print(epochTime);
    Serial.println(" Published message");

    lastPublish = millis();
  }
  else
  {
    Serial.println("Forced measurement failed!");
  }
}

void getConfig()
{
  unsigned long epochTime;
  epochTime = getTime();
  char outputData[256];

  sprintf(outputData, "{\"timestamp\": %ld, \"localIp\": \"%s\", \"timeout\": %d, \"temperatureShift\": %d, \"pressureShift\": %d, \"altitudeShift\": %d}", epochTime, config.localIp.c_str(), config.timeout, config.temperatureShift, config.pressureShift, config.altitudeShift);

  pubSubClient.publish(thingMqttTopicOut, outputData);

  Serial.print(epochTime);
  Serial.println(" Published message");
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("ESP8266 AWS IoT MQTT Connection");

  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  String localIp = WiFi.localIP().toString();
  Serial.print(", WiFi connected, IP address: ");
  Serial.println(localIp);
  config.localIp = localIp;

  // get current time, otherwise certificates are flagged as expired
  setCurrentTime();

  wiFiClient.setClientRSACert(&client_crt, &client_key);
  wiFiClient.setTrustAnchors(&rootCert);

  setupBmpSensor();

  setupWebServer();

  measure();
}

void loop()
{
  pubSubCheckConnect();
  if (millis() - unsigned(lastPublish) > unsigned(config.timeout))
  {
    measure();
  }

  cleanupWsClients();
}

void msgReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message received on ");
  Serial.print(topic);
  Serial.print(": ");
  for (uint i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  JsonObject obj = doc.as<JsonObject>();
  String message = obj["message"];
  // message = obj["message"].as<String>(); // probably not required
  Serial.println(message);

  if (message == "measure")
  {
    measure();
  }
  if (message == "getConfig")
  {
    getConfig();
  }

  if (message == "updateConfig" && !obj["key"].isNull() && !obj["value"].isNull())
  {
    if (obj["key"] == "timeout")
    {
      config.timeout = int(obj["value"]);
      Serial.print("timeout: ");
      Serial.println(config.timeout);
    }

    if (obj["key"] == "temperatureShift")
    {
      config.temperatureShift = int(obj["value"]);
      Serial.print("temperatureShift: ");
      Serial.println(config.temperatureShift);
    }

    if (obj["key"] == "altitudeShift")
    {
      config.altitudeShift = int(obj["value"]);
      Serial.print("altitudeShift: ");
      Serial.println(config.altitudeShift);
    }

    if (obj["key"] == "pressureShift")
    {
      config.pressureShift = int(obj["value"]);
      Serial.print("pressureShift: ");
      Serial.println(config.pressureShift);
    }
  }
}
