const int buttonPin = 2;
const int ledPin = 13;

volatile bool ledState = LOW;

void setup() {
  pinMode(buttonPin, INPUT);
  //digitalWrite(buttonPin, HIGH);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonInterrupt, FALLING);
}

void loop() {
  
}

void buttonInterrupt() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
  delay(1000);
}
