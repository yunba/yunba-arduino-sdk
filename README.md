# yunba-arduino-sdk

This SDK is developed based on project [**arduino-mqtt**](https://github.com/256dpi/arduino-mqtt.git) . This SDK should be compatible with these boards supported by [**arduino-mqtt**](https://github.com/256dpi/arduino-mqtt.git). It is only tested on Linkltone board now.

Please note that this sdk doesn't support standard mqtt currently.

You should Install **ArduinoJson** library from Library Manager arduino IDE.

## Example

At first, we get MQTT broker and registration information. 

```c++
#include <MQTTClient.h>
#include <ArduinoJson.h>

MQTTClient client;
StaticJsonBuffer<200> jsonBuffer;

const char yunba_appkey[] = "<your-appkey>";

unsigned long lastMillis = 0;

char broker_addr[56];
uint16_t port;
char client_id[56];
char username[56];
char password[56];
char device_id[56];

bool get_host_v2(const char *appkey, char *url) {
  ...
}

bool setup_with_appkey_and_devid(const char* appkey, const char *deviceid) {
  ...
}
```

You can find implemented source code under path **examples/Linkltone_yunba/YunbaWIFI.ino** for your reference.  

The following example the MQTTClient to connect to Yunba message bus:

```c++

void setup() {
  Serial.begin(9600);
  client.begin(broker_addr, port, net);

  connect();
}

void connect() {
  Serial.print("connecting...");
  while (!client.connect(client_id, username, password)) {
    Serial.print(".");
  }

  Serial.println("\nconnected!");

  client.subscribe("<your-topic>");
  // client.unsubscribe("<your-topic>");
}

void loop() {
  client.loop();

  if(!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if(millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("<your-topic>", "world");
  }
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

void extMessageReceived(EXTED_CMD cmd, int status, String payload, unsigned int length) {
  Serial.print("incoming ext message: ");
  Serial.print(cmd);
  Serial.print(" - ");
  Serial.print(payload);
}
```

Please note that variable **net** can be defined different type.

With Arduino Ethernet Shield:
```c++
EthernetClient net;
```

With Arduino WiFi Shield:

```c++
WiFiClient net;
```

LinkltOne WiFI:

```c++
LWiFiClient net;
```

## API

Initialize the object using the hostname of the broker, the brokers port (default: `1883`) and the underlying Client class for network transport:

```c++
boolean begin(const char * hostname, Client& client);
boolean begin(const char * hostname, int port, Client& client);
```

Set the will message that gets registered on a connect:

```c++
void setWill(const char * topic);
void setWill(const char * topic, const char * payload);
```

Connect to broker using the supplied client id and username and password:

```c++
boolean connect(const char * clientId, const char * username, const char * password);
```

_This functions returns a value that indicates if the connection has been established successfully._

Publishes a message to the broker with an optional payload:

```c++
void publish(String topic);
void publish(String topic, String payload);
void publish(const char * topic, String payload);
void publish(const char * topic, const char * payload);
```

Publish2 a message to the broker. About opt_json, you can read [option json](https://github.com/yunba/docs/blob/master/restful_Quick_Start.md)

```c++
void publish2(const char * topic, const char * payload, const char  *opt_json);
```

Publish2 a message to one alias.

```c++
void publish2ToAlias(const char * alias, const char * payload);
```

Subscribe to a topic: 

```c++
void subscribe(String topic);
void subscribe(const char * topic);
```

Unsubscribe from a topic:

```c++
void unsubscribe(String topic);
void unsubscribe(const char * topic);
```

Sends and receives packets: 

```c++
void loop();
```

_This function should be called in every `loop`._

Check if the client is currently connected:

```c++
boolean connected();
```

Disconnects from the broker:

```c++
void disconnect();
```

You can find one example under path **examples/Linkltone_yunba/** .
