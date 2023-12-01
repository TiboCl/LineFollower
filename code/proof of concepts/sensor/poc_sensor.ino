const int OUT_PIN1 = 0;
const int OUT_PIN2 = 1;
const int OUT_PIN3 = 2;
const int OUT_PIN4 = 3;
const int OUT_PIN6 = 5;
const int OUT_PIN7 = 6;
const int OUT_PIN8 = 7;
const uint8_t SensorCount = 6;
static char serial_info_buffer[500] = {0};


void setup() {
Serial.begin(9600);
}
 
void loop() {
int sensorValue1 = analogRead(OUT_PIN1);
int sensorValue2 = analogRead(OUT_PIN2);
int sensorValue3 = analogRead(OUT_PIN3);
int sensorValue6 = analogRead(OUT_PIN6);
int sensorValue7 = analogRead(OUT_PIN7);
int sensorValue8 = analogRead(OUT_PIN8);

sprintf(serial_info_buffer, "PID_control: sensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", sensorValue1, sensorValue2, sensorValue3, sensorValue6, sensorValue7, sensorValue8);
Serial.write(serial_info_buffer);
}

