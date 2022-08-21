#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define LIGHT_SWITCH_PIN D5
#define FAN_SWITCH_PIN D6
#define SOCKET_SWITCH_PIN D7
#define LIGHT_RELAY_PIN D0
#define FAN_RELAY_PIN D1
#define SOCKET_RELAY_PIN D2
// For NO(Normal Open) Relay, LOW is ON. Relay state may change based on manufacturer.
#define RELAY_ON LOW
#define RELAY_OFF HIGH
const char* ssid = "presha";
const char* password = "change_it";
const char* mqttServer = "192.168.0.119";
const int mqttPort = 1883;
const char* mqttClientName = "LivingRoom";
const char* swicthOn = "ON";
const char* switchOff = "OFF";
const char* stateTopicLIGHT = "livingroom/lightSwitch";
const char* commandTopicLIGHT = "livingroom/lightSwitch/set";
const char* stateTopicFan = "livingroom/fanSwitch";
const char* commandTopicFan = "livingroom/fanSwitch/set";
const char* stateTopicSocket = "livingroom/socketSwitch";
const char* commandTopicSocket = "livingroom/socketSwitch/set";
bool clientConnected = false;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String strTopic = topic;
  //Serial.print("Message arrived - " + strTopic);
  String message = "";
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    message = message + (char)payload[i];
  }
  //Serial.print("\n");
  //Serial.print("_"+ message +"_");

  uint8_t pinName = 0;
  if (strTopic == commandTopicLIGHT) {
    pinName = LIGHT_RELAY_PIN;
  } else if (strTopic == commandTopicFan) {
    pinName = FAN_RELAY_PIN;
  } else if (strTopic == commandTopicSocket) {
    pinName = SOCKET_RELAY_PIN;
  }

  if (pinName != 0) {
    digitalWrite(pinName, message == swicthOn ? RELAY_ON : RELAY_OFF);
  }
}

void CheckAndConnectMQTTServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No WiFi");
  }
  
  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() == WL_CONNECTED && !client.connected()){
    Serial.println("Connect MQTT");
    //if connected, subscribe to the topic(s) we want to be notified about
    if (client.connect(mqttClientName)) {
      //subscribe to topics here
      Serial.println("client connected");
      client.subscribe(commandTopicLIGHT);
      client.subscribe(commandTopicFan);
      client.subscribe(commandTopicSocket);
      clientConnected = true;
      delay(5000);
    }
  }
}

void publishSwitchState(){
  client.publish(stateTopicLIGHT, digitalRead(LIGHT_RELAY_PIN) == RELAY_ON ? swicthOn : switchOff);
  client.publish(stateTopicFan, digitalRead(FAN_RELAY_PIN) == RELAY_ON ? swicthOn : switchOff);
  client.publish(stateTopicSocket, digitalRead(SOCKET_RELAY_PIN) == RELAY_ON ? swicthOn : switchOff);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("MQTT Bedroom");

  // Set initial replay state to off
  digitalWrite(LIGHT_RELAY_PIN, RELAY_OFF);
  digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
  digitalWrite(SOCKET_RELAY_PIN, RELAY_OFF);
  
  pinMode(LIGHT_SWITCH_PIN, INPUT);
  pinMode(FAN_SWITCH_PIN, INPUT);
  pinMode(SOCKET_SWITCH_PIN, INPUT);
  
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(SOCKET_RELAY_PIN, OUTPUT);

  digitalWrite(LIGHT_SWITCH_PIN, LOW);
  digitalWrite(FAN_SWITCH_PIN, LOW);
  digitalWrite(SOCKET_SWITCH_PIN, LOW);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  //start wifi subsystem
  WiFi.begin(ssid, password);
  WiFi.enableAP(false);
  
  for (int i = 0; i < 10; i++) {
    //attempt to connect to the WIFI network and then connect to the MQTT server
    if (clientConnected) {
      break;
    }
    delay(1000);
    CheckAndConnectMQTTServer();
  }
}

void loop() {
  delay(1000);
  
  if (digitalRead(LIGHT_SWITCH_PIN) == HIGH) {
    digitalWrite(LIGHT_RELAY_PIN, digitalRead(LIGHT_RELAY_PIN) == RELAY_OFF ? RELAY_ON : RELAY_OFF);
  }
  if (digitalRead(FAN_SWITCH_PIN) == HIGH) {
    digitalWrite(FAN_RELAY_PIN, digitalRead(FAN_RELAY_PIN) == RELAY_OFF ? RELAY_ON : RELAY_OFF);
  }
  if (digitalRead(SOCKET_SWITCH_PIN) == HIGH) {
    digitalWrite(SOCKET_RELAY_PIN, digitalRead(SOCKET_RELAY_PIN) == RELAY_OFF ? RELAY_ON : RELAY_OFF);
  }

  CheckAndConnectMQTTServer();
  
  if(WiFi.status() == WL_CONNECTED && client.connected()){
    publishSwitchState();
    client.loop();
  }
}
