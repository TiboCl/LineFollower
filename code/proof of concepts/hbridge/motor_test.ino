const int PIN_DRV8833_B_IN1 = 11;
const int PIN_DRV8833_B_IN2 = 12;
const int PIN_DRV8833_A_IN1 = 10;
const int PIN_DRV8833_A_IN2 = 9;

void setup() {
  
  pinMode(PIN_DRV8833_B_IN1, OUTPUT);
  pinMode(PIN_DRV8833_B_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN1, OUTPUT);
  pinMode(PIN_DRV8833_A_IN2, OUTPUT);
}

void loop() {
  forward();
  delay(2000);  
  
  backward();
  delay(2000);  
}

void forward() {
  digitalWrite(PIN_DRV8833_B_IN1, HIGH);
  digitalWrite(PIN_DRV8833_B_IN2, LOW);
  digitalWrite(PIN_DRV8833_A_IN1, HIGH);
  digitalWrite(PIN_DRV8833_A_IN2, LOW);
}

void backward() {
  digitalWrite(PIN_DRV8833_B_IN1, LOW);
  digitalWrite(PIN_DRV8833_B_IN2, HIGH);
  digitalWrite(PIN_DRV8833_A_IN1, LOW);
  digitalWrite(PIN_DRV8833_A_IN2, HIGH);
}


