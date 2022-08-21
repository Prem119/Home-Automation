const int switch1 = 5;
const int switch2 = 4;
const int switch3 = 0;
const int switch4 = 2;
const int sensor = 13;
int lastState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(sensor, INPUT);
  pinMode(switch1, OUTPUT);
  pinMode(switch2, OUTPUT);
  pinMode(switch3, OUTPUT);
  pinMode(switch4, OUTPUT);
  
  digitalWrite(switch1, lastState);
  digitalWrite(switch2, lastState);
  digitalWrite(switch3, lastState);
  digitalWrite(switch4, lastState);
}

void loop() {
  long state = digitalRead(sensor);
  if(state == HIGH) {
    lastState = lastState == LOW ? HIGH : LOW;
    Serial.println("Detected!");
    digitalWrite(switch1, lastState);
    digitalWrite(switch2, lastState);
    digitalWrite(switch3, lastState);
    digitalWrite(switch4, lastState);
  }
  
  delay(1000);
}
