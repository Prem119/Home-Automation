#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//setup
#define CFL_SWITCH_PIN D5
#define FAN_SWITCH_PIN D6
#define SOCKET_SWITCH_PIN D7
#define CFL_PIN D2
#define FAN_PIN D3
#define SOCKET_PIN D4
#define MQTT_SERVER "192.168.0.119"
const char* ssid = "presha";
const char* password = "change_it";
const char* swicthOn = "ON";
const char* switchOff = "OFF";
const char* stateTopicCFL = "bedroom/cflSwitch";
const char* commandTopicCFL = "bedroom/cflSwitch/set";
const char* stateTopicFan = "bedroom/fanSwitch";
const char* commandTopicFan = "bedroom/fanSwitch/set";
const char* stateTopicSocket = "bedroom/socketSwitch";
const char* commandTopicSocket = "bedroom/socketSwitch/set";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String strTopic = topic;
  Serial.print("Message arrived - " + strTopic);
  String message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message = message + (char)payload[i];
  }
  Serial.print("\n");
  Serial.print("_"+ message +"_");

  uint8_t pinName = 0;
  if (strTopic == commandTopicCFL) {
    pinName = CFL_PIN;
  } else if (strTopic == commandTopicFan) {
    pinName = FAN_PIN;
  } else if (strTopic == commandTopicSocket) {
    pinName = SOCKET_PIN;
  }

  if (pinName != 0) {
    digitalWrite(pinName, message == swicthOn ? HIGH : LOW);
  }
}

void reconnect() {
  Serial.println("reconnect()");
  //loop while we wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Try again");
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED && !client.connected()){
    // Generate client name based on MAC address and last 8 bits of microsecond counter
    String clientName;
    clientName += "MasterBedroom-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    
    // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      delay(500);
      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        //subscribe to topics here
        Serial.println("client connected");
        client.subscribe(commandTopicCFL);
        client.subscribe(commandTopicFan);
        client.subscribe(commandTopicSocket);
      }
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5){
      result += ':';
    }
  }
  return result;
}

void publishSwitchState(){
  client.publish(stateTopicCFL, digitalRead(CFL_PIN) == HIGH ? swicthOn : switchOff);
  client.publish(stateTopicFan, digitalRead(FAN_PIN) == HIGH ? swicthOn : switchOff);
  client.publish(stateTopicSocket, digitalRead(SOCKET_PIN) == HIGH ? swicthOn : switchOff);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println("MQTT Bedroom");

  pinMode(CFL_SWITCH_PIN, INPUT);
  pinMode(FAN_SWITCH_PIN, INPUT);
  pinMode(SOCKET_SWITCH_PIN, INPUT);
  
  pinMode(CFL_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SOCKET_PIN, OUTPUT);

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  //start wifi subsystem
  WiFi.begin(ssid, password);
  WiFi.enableAP(false);
  
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();
}

void loop() {
  if (digitalRead(CFL_SWITCH_PIN) == HIGH) {
    digitalWrite(CFL_PIN, digitalRead(CFL_PIN) == LOW ? HIGH : LOW);
  }
  if (digitalRead(FAN_SWITCH_PIN) == HIGH) {
    digitalWrite(FAN_PIN, digitalRead(FAN_PIN) == LOW ? HIGH : LOW);
  }
  if (digitalRead(SOCKET_SWITCH_PIN) == HIGH) {
    digitalWrite(SOCKET_PIN, digitalRead(SOCKET_PIN) == LOW ? HIGH : LOW);
  }
  
  publishSwitchState();
  
  if (!client.connected() || WiFi.status() != WL_CONNECTED) {
    reconnect();
  }
  
  client.loop();
  delay(1000);
}
