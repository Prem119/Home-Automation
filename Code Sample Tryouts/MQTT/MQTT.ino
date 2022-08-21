//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//setup
#define CFL_PIN D2
#define FAN_PIN D3
#define MQTT_SERVER "192.168.0.119"
const char* ssid = "presha";
const char* password = "change_it";
char* stateTopicCFL = "bedroom/cflSwitch";
char* commandTopicCFL = "bedroom/cflSwitch/set";
char* stateTopicFan = "bedroom/fanSwitch";
char* commandTopicFan = "bedroom/fanSwitch/set";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]_");
  String message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message = message+(char)payload[i];
  }
  Serial.print("_\n");
  Serial.print("_"+message+"_");
  String topicStr = topic;
  
  if (topicStr == commandTopicCFL)
  {
    if  (message == "ON"){
      Serial.print("\nOn called\n");
      digitalWrite(CFL_PIN, HIGH);
    }else if (message == "OFF"){
      Serial.print("\nOff called\n");
      digitalWrite(CFL_PIN, LOW);
    }
  } else if (topicStr == commandTopicFan) {
    if  (message == "ON"){
      Serial.print("\nOn called\n");
      digitalWrite(FAN_PIN, HIGH);
    }else if (message == "OFF"){
      Serial.print("\nOff called\n");
      digitalWrite(FAN_PIN, LOW);
    }
  }
}

void reconnect() {
  Serial.println("reconnect()");
  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("Try again");
    }
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        //subscribe to topics here
        Serial.println("client connected");
        client.subscribe(commandTopicCFL);
        client.subscribe(commandTopicFan);
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

void state(){
  if (digitalRead(CFL_PIN) == HIGH){
    client.publish(stateTopicCFL, "ON");
  }else{
    client.publish(stateTopicCFL, "OFF");
  }

  if (digitalRead(FAN_PIN) == HIGH){
    client.publish(stateTopicFan, "ON");
  }else{
    client.publish(stateTopicFan, "OFF");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println("MQTT Bedroom");

  pinMode(CFL_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  //start wifi subsystem
  WiFi.begin(ssid, password);
  WiFi.enableAP(false);
  
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();
}

void loop() {
 state();

 if (!client.connected() && WiFi.status() == 3) { reconnect();  }
 
 client.loop();
 delay(1000);
}
