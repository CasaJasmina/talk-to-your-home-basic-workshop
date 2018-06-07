/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"


  This code talks to an Home Assistant Server hosted on a custom ip address. 
  This is the config.yaml that it talks to:

  switch:
  - platform: mqtt
    name: "Davide"
    state_topic: "status" # lo rimandi tu quando non sei in optimistic
    command_topic: "relay/set"
    availability_topic: "relay/set/available"
    payload_on: "1"
    payload_off: "0"
    optimistic: true
    qos: 0
    retain: true


  This is the view section in order to see it

  group:
  default_view:
    view: yes
    entities:
      - switch.Davide


remember to run 

$ sudo systemctl restart home-assistant.service

Every time you commit a change to the configuration.yaml file in order to see any changes on the UI

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "OfficineInnesto";
const char* password = "OfficineRulez";
const char* mqtt_server = "192.168.0.108";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const int relayPin = D1;
const long interval = 2000;  // pause for two seconds



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

   digitalWrite(relayPin, HIGH);

   delay(1000);

    digitalWrite(relayPin, LOW);
    delay(1000);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    Serial.println("Payload received");
    client.publish("relay/set/status", "1");
    client.publish("office/light1/switch", "ON");
    digitalWrite(relayPin, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
    delay(1000);
  } else if((char)payload[0] == '0'){
    digitalWrite(relayPin, LOW);  // Turn the LED off by making the voltage HIGH
    client.publish("relay/set/status", "0");
    client.publish("office/light1/switch", "OFF");

  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    //  client.publish("sensorbox/sensorbox/gas", "hello world");
      // ... and resubscribe
      client.subscribe("relay/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensorbox/sensorbox/gas", msg);
  }
}
