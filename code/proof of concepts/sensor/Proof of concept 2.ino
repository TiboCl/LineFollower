const int OUT_PIN1 = 0;
const int OUT_PIN2 = 1;
const int OUT_PIN3 = 2;
const int OUT_PIN4 = 3;
const int OUT_PIN5 = 4;
const int OUT_PIN6 = 5;
const int OUT_PIN7 = 6;
const int OUT_PIN8 = 7;

void setup() {
Serial.begin(9600);
}
 
void loop() {
Serial.println(analogRead(OUT_PIN1));
Serial.println(analogRead(OUT_PIN2));
Serial.println(analogRead(OUT_PIN3));
Serial.println(analogRead(OUT_PIN4));
Serial.println(analogRead(OUT_PIN5));
Serial.println(analogRead(OUT_PIN6));
Serial.println(analogRead(OUT_PIN7));
Serial.println(analogRead(OUT_PIN8));
}

