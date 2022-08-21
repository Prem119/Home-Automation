int ledstatus = 12;
int sensor = 13;
int lastState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(sensor, INPUT);
  pinMode(ledstatus, OUTPUT);
}

void loop() {
  long state = digitalRead(sensor);
  if(state == HIGH) {
    lastState = lastState == LOW ? HIGH : LOW;
    digitalWrite(ledstatus, lastState);
    Serial.println("Detected!");
  }
  delay(1000);
}
