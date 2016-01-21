#include "SPI.h"

#include <LWiFi.h>
#include <MQTTClient.h>
#include <LWiFiClient.h>
#include <ArduinoJson.h>

#define WIFI_AP "yunba.io"
#define WIFI_PASSWORD "Hiyunba2013"
#define WIFI_AUTH LWIFI_WPA

const int NewMsgLedPin = 13;
const char yunba_appkey[] = "563c4afef085fc471efdf803";
const char yunba_topic[] = "linkltone";
const char yunba_devid[] = "linkltone_board2";
const char *opt_json = "{\"time_to_live\":\"120\", \"qos\":\"2\"}";
boolean netstatus = false;
char url[56];
char broker_addr[56];
uint16_t port;
unsigned long lastMillis = 0;
char client_id[56];
char username[56];
char password[56];
char device_id[56];

LWiFiClient net;
MQTTClient client;
StaticJsonBuffer<200> jsonBuffer;

bool get_ip_pair(const char *url, char *addr, uint16_t *port) {
  char *p = strstr(url, "tcp://");
  if (p) {
    p += 6;
    char *q = strstr(p, ":");
    if (q) {
      int len = strlen(p) - strlen(q);
      if (len > 0) {
        memcpy(addr, p, len);
        //sprintf(addr, "%.*s", len, p);
        *port = atoi(q + 1);
        return true;
      }
    }
  }
  return false;
}

bool get_host_v2(const char *appkey, char *url) {
  uint8_t buf[256];
  bool rc = false;
  LWiFiClient net_client;
  while (0 == net_client.connect("tick-t.yunba.io", 9977)) {
    Serial.println("Re-connect to tick");
    delay(1000);
  }
  delay(100);

  String data = "{\"a\":\"" + String(appkey) + "\",\"n\":\"1\",\"v\":\"v1.0\",\"o\":\"1\"}";
  int json_len = data.length();
  int len;

  buf[0] = 1;
  buf[1] = (uint8_t)((json_len >> 8) & 0xff);
  buf[2] = (uint8_t)(json_len & 0xff);
  len = 3 + json_len;
  memcpy(buf + 3, data.c_str(), json_len);
  net_client.flush();
  net_client.write(buf, len);

  while (!net_client.available()) {
    Serial.println(json_len, len);
    Serial.println(len);
    Serial.println("wailt data");
    delay(100);
  }

  memset(buf, 0, 256);
  int v = net_client.read(buf, 256);
  if (v > 0) {
    len = (uint16_t)(((uint8_t)buf[1] << 8) | (uint8_t)buf[2]);
    if (len == strlen((char *)(buf + 3))) {
      Serial.println((char *)(&buf[3]));
      JsonObject& root = jsonBuffer.parseObject((char *)&buf[3]);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        goto exit;
      }
      strcpy(url, root["c"]);
      Serial.println(url);
      rc = true;
    }
  }
exit:
  net_client.stop();
  return rc;
}

bool setup_with_appkey_and_devid(const char* appkey, const char *deviceid) {
  uint8_t buf[256];
  bool rc = false;
  LWiFiClient net_client;

  if (appkey == NULL) return false;

  while (0 == net_client.connect("reg-t.yunba.io", 9944)) {
    Serial.println("Re-connect to tick");
    delay(1000);
  }
  delay(100);

  String data;
  if (deviceid == NULL)
    data = "{\"a\": \"" + String(appkey) + "\", \"p\":4}";
  else
    data = "{\"a\": \"" + String(appkey) + "\", \"p\":4, \"d\": \"" + String(deviceid) + "\"}";
  int json_len = data.length();
  int len;

  buf[0] = 1;
  buf[1] = (uint8_t)((json_len >> 8) & 0xff);
  buf[2] = (uint8_t)(json_len & 0xff);
  len = 3 + json_len;
  memcpy(buf + 3, data.c_str(), json_len);
  net_client.flush();
  net_client.write(buf, len);

  while (!net_client.available()) {
    Serial.println(json_len, len);
    Serial.println(len);
    Serial.println(data);
    Serial.println("wailt data");
    delay(100);
  }

  memset(buf, 0, 256);
  int v = net_client.read(buf, 256);
  if (v > 0) {
    len = (uint16_t)(((uint8_t)buf[1] << 8) | (uint8_t)buf[2]);
    if (len == strlen((char *)(buf + 3))) {
      Serial.println((char *)(&buf[3]));
      JsonObject& root = jsonBuffer.parseObject((char *)&buf[3]);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        net_client.stop();
        return false;
      }
      strcpy(username, root["u"]);
      strcpy(password, root["p"]);
      strcpy(client_id, root["c"]);
      Serial.println(username);
      rc = true;
    }
  }

  net_client.stop();
  return rc;
}

void set_alias(const char *alias) {
  client.publish(",yali", alias);
}

void publish_to_alias(const char *alias, char *message) {
  String topic = ",yta/" + String(alias);
  client.publish(topic, message);
}

void setup() {
  pinMode(NewMsgLedPin, OUTPUT);
  digitalWrite(NewMsgLedPin, LOW);

  Serial.begin(9600);
  Serial.println("Serial set up");
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD))) {
    Serial.println(" . ");
    delay(1000);
  }
  Serial.println("Connected to AP");

  //TODO: if we can't get reg info and tick info
  get_host_v2(yunba_appkey, url);
  setup_with_appkey_and_devid(yunba_appkey, yunba_devid/*, &info*/);

  get_ip_pair(url, broker_addr, &port);
  //  client.begin("192.168.2.136", port, net);
  client.begin(broker_addr, port, net);

  connect();
}

void flash(int LedPin)
{
  for (int i = 0; i < 3; i++) {
    digitalWrite(LedPin, HIGH);
    delay(100);
    digitalWrite(LedPin, LOW);
    delay(100);
  }
}

void connect() {
  //  Serial.print("\nconnecting...");
  while (!client.connect(client_id, username, password)) {
    Serial.print(".");
  }

  Serial.println("\nconnected!");
  client.subscribe(yunba_topic);
  set_alias(yunba_devid);
}

void check_connect(uint32_t interval) {
  if (millis() - lastMillis > interval) {
    boolean st = client.connected();
    if (st != netstatus) {
      Serial.print("connect status:");
      netstatus = st;
      Serial.println(netstatus);
    }

    if (!st) {
      connect();
    }
  }
}

void loop() {
  client.loop();

  check_connect(6000);
  if (millis() - lastMillis > 20000) {
    lastMillis = millis();
    client.publish(yunba_topic, "publish test");
    client.publish2ToAlias("PC", "publish2", opt_json);
  }
  delay(100);
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  flash(NewMsgLedPin);
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

void extMessageReceived(EXTED_CMD cmd, int status, String payload, unsigned int length) {
  flash(NewMsgLedPin);
  Serial.print("incoming ext message: ");
  Serial.print(cmd);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

