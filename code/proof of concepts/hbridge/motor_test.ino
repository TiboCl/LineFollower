#include "motor_test.h"
const int PIN_DRV8833_B_IN1 = 11;
const int PIN_DRV8833_B_IN2 = 12;
const int PIN_DRV8833_A_IN1 = 10;
const int PIN_DRV8833_A_IN2 = 9;

static void brake(Motor_t motor)
{
  if(motor == MOTOR_A){
    analogWritePWMOff(PIN_DRV8833_A_IN1, LOW);
    analogWritePWMOff(PIN_DRV8833_A_IN2, LOW);
  }else{
    analogWritePWMOff(PIN_DRV8833_B_IN1, LOW);
    analogWritePWMOff(PIN_DRV8833_B_IN2, LOW);
  }
  sprintf(serial_info_buffer, "brake done\n");
  Serial.write(serial_info_buffer);
}

static void forward(Motor_t motor, uint8_t speed)
{
  if(motor == MOTOR_A){
    analogWrite(PIN_DRV8833_A_IN1, speed);
    analogWritePWMOff(PIN_DRV8833_A_IN2, LOW);
  }else{
    analogWrite(PIN_DRV8833_B_IN1, speed);
    analogWritePWMOff(PIN_DRV8833_B_IN2, LOW);
  }
  sprintf(serial_info_buffer, "forward done: %d\n", speed);
  Serial.write(serial_info_buffer);
}

static void reverse(Motor_t motor, uint8_t speed)
{
  if(motor == MOTOR_A){
    analogWritePWMOff(PIN_DRV8833_A_IN1, LOW);
    analogWrite(PIN_DRV8833_A_IN2, speed);
  }else{
    analogWritePWMOff(PIN_DRV8833_B_IN1, LOW);
    analogWrite(PIN_DRV8833_B_IN2, speed);
  }
    
  sprintf(serial_info_buffer, "reverse done: %d\n", speed);
  Serial.write(serial_info_buffer);
}

static void set_speed(Motor_t motor, uint8_t speed, Direction_t direction)
{
  if(speed == 0){
    brake(motor);
  }else{
    if(direction == FORWARD){
      forward(motor, speed);
    }else{
      reverse(motor, speed);
    }
  }
  
  sprintf(serial_info_buffer, "set speed done: %d\n", speed);
  Serial.write(serial_info_buffer);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.write("Setup started (zorg dat uw batterij aanstaat)\n");
  pinMode(PIN_DRV8833_B_IN1, OUTPUT);
  pinMode(PIN_DRV8833_B_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN1, OUTPUT);
  Serial.write("Setup done\n");
}

void loop() {
  delay(5000);

  sprintf(serial_info_buffer, "Start Forward 127\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 127, FORWARD);
  set_speed(MOTOR_B, 120, FORWARD);
  delay(2000);

  sprintf(serial_info_buffer, "Start Forward 0 (1)\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 0, FORWARD);
  set_speed(MOTOR_B, 0, FORWARD);
  delay(2000);

  sprintf(serial_info_buffer, "Start Forward 250\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 250, FORWARD);
  set_speed(MOTOR_B, 250, FORWARD);
  delay(2000);
  
  sprintf(serial_info_buffer, "Start Forward 0 (2)\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 0, FORWARD);
  set_speed(MOTOR_B, 0, FORWARD);
  delay(2000);
  
  sprintf(serial_info_buffer, "Start reverse 127\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 127, REVERSE);
  set_speed(MOTOR_B, 127, REVERSE);
  delay(2000);

  sprintf(serial_info_buffer, "Start Reverse 0 (1)\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 0, REVERSE);
  set_speed(MOTOR_B, 0, REVERSE);
  delay(2000);

  sprintf(serial_info_buffer, "Start reverse 250\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 250, REVERSE);
  set_speed(MOTOR_B, 250, REVERSE);
  delay(2000);

  sprintf(serial_info_buffer, "Start Reverse 0 (2)\n");
  Serial.write(serial_info_buffer);
  set_speed(MOTOR_A, 0, REVERSE);
  set_speed(MOTOR_B, 0, REVERSE);
  delay(2000);
}
