const int PIN_DRV8833_B_IN1 = 12;
const int PIN_DRV8833_B_IN2 = 11;
const int PIN_DRV8833_A_IN2 = 10;
const int PIN_DRV8833_A_IN1 = 9;
const int DELAY_BETWEEN_TESTS_MS = 2000;

typedef enum 
{
  FAST,
  SLOW
} DecayMode_t;

typedef enum 
{
  A,
  B
} Motor_t;

typedef enum
{
  FORWARD,
  REVERSE,
  NOT_RELEVANT
} Direction_t;

static const char* const direction_strings[] = { "FORWARD", "REVERSE", "NOT_RELEVANT", 0 };
static const char* const motor_strings[] = { "A", "B", 0 };
static const char* const decaymode_strings[] = { "FAST", "SLOW", 0 };
static char serial_info_buffer[100] = {0};

DecayMode_t CUR_DECAY_MODE = FAST;

static void digitalWritePWMOff( pin_size_t ulPin, PinStatus ulVal )
{
  pinMode(ulPin, OUTPUT);
  digitalWrite(ulPin, ulVal);
}

// Table 2. PWM Control of Motor Speed
// xIN1   xIN2    FUNCTION
// PWM    0       Forward PWM, fast decay
// 1      PWM     Forward PWM, slow decay
// 0      PWM     Reverse PWM, fast decay
// PWM    1       Reverse PWM, slow decay

static void forward(Motor_t motor, uint8_t speed)
{
  // Assume speed > 0, if it's zero, we should break or coast
  if(CUR_DECAY_MODE == FAST){
    // PWM    0       Forward PWM, fast decay
    if(motor == A){
      analogWrite(PIN_DRV8833_A_IN1, speed);
      // analogWrite(PIN_DRV8833_A_IN2, 0);
      digitalWritePWMOff(PIN_DRV8833_A_IN2, LOW);
    }else{
      analogWrite(PIN_DRV8833_B_IN1, speed);
      // analogWrite(PIN_DRV8833_B_IN2, 0);
      digitalWritePWMOff(PIN_DRV8833_B_IN2, LOW);
    }
  }else{
    // 1      PWM     Forward PWM, slow decay
    if(motor == A){
      // analogWrite(PIN_DRV8833_A_IN1, 255);
      digitalWritePWMOff(PIN_DRV8833_A_IN1, HIGH);
      analogWrite(PIN_DRV8833_A_IN2, speed);
    }else{
      // analogWrite(PIN_DRV8833_B_IN1, 255);
      digitalWritePWMOff(PIN_DRV8833_B_IN1, HIGH);
      analogWrite(PIN_DRV8833_B_IN2, speed);
    }
  }
}

static void reverse(Motor_t motor, uint8_t speed)
{
  if(CUR_DECAY_MODE == FAST){
    // 0      PWM     Reverse PWM, fast decay
    if(motor == A){
      //analogWrite(PIN_DRV8833_A_IN1, 0);
      digitalWritePWMOff(PIN_DRV8833_A_IN1, LOW);
      analogWrite(PIN_DRV8833_A_IN2, speed);
    }else{
      // analogWrite(PIN_DRV8833_B_IN1, 0);
      digitalWritePWMOff(PIN_DRV8833_B_IN1, LOW);
      analogWrite(PIN_DRV8833_B_IN2, speed);
    }
  }else{
    // PWM    1       Reverse PWM, slow decay
    if(motor == A){
      analogWrite(PIN_DRV8833_A_IN1, speed);
      // analogWrite(PIN_DRV8833_A_IN2, 255);
      digitalWritePWMOff(PIN_DRV8833_A_IN2, HIGH);
    }else{
      analogWrite(PIN_DRV8833_B_IN1, speed);
      // analogWrite(PIN_DRV8833_B_IN2, 255);
      digitalWritePWMOff(PIN_DRV8833_B_IN2, HIGH);
    }
  }
}

// Table 1. H-Bridge Logic
// xIN1   xIN2    xOUT1   xOUT2   FUNCTION
// 0      0       Z       Z       Coast/fast decay
// 0      1       L       H       Reverse
// 1      0       H       L       Forward
// 1      1       L       L       Brake/slow decay
static void coast_motor(Motor_t motor)
{
  // coast
  if(motor == A){
    // analogWrite(PIN_DRV8833_A_IN1, 0);
    // analogWrite(PIN_DRV8833_A_IN2, 0);
    digitalWritePWMOff(PIN_DRV8833_A_IN1, LOW);
    digitalWritePWMOff(PIN_DRV8833_A_IN2, LOW);
  }else{
    // analogWrite(PIN_DRV8833_B_IN1, 0);
    // analogWrite(PIN_DRV8833_B_IN2, 0);
    digitalWritePWMOff(PIN_DRV8833_B_IN1, LOW);
    digitalWritePWMOff(PIN_DRV8833_B_IN2, LOW);
  }
}

static void brake_motor(Motor_t motor)
{
  // break
  if(motor == A){
    // analogWrite(PIN_DRV8833_A_IN1, 255);
    // analogWrite(PIN_DRV8833_A_IN2, 255);
    digitalWritePWMOff(PIN_DRV8833_A_IN1, HIGH);
    digitalWritePWMOff(PIN_DRV8833_A_IN2, HIGH);
  }else{
    // analogWrite(PIN_DRV8833_B_IN1, 255);
    // analogWrite(PIN_DRV8833_B_IN2, 255);
    digitalWritePWMOff(PIN_DRV8833_B_IN1, HIGH);
    digitalWritePWMOff(PIN_DRV8833_B_IN2, HIGH);
  }
}

static void set_speed(Motor_t motor, uint8_t speed, Direction_t direction)
{
  if(speed == 0){
    // break or coast, depending on current DecayMode (CUR_DECAY_MODE)
    if(CUR_DECAY_MODE == FAST){
      coast_motor(motor);
    }else{
      brake_motor(motor);
    }
  }else{
    if(direction == FORWARD){
      forward(motor, speed);
    }else{
      reverse(motor, speed);
    }
  }
}


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(5000);
  Serial.write("Setup started\n");
  pinMode(PIN_DRV8833_B_IN1, OUTPUT);
  pinMode(PIN_DRV8833_B_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN1, OUTPUT);
  Serial.write("Setup done\n");
}

static void test_motor_direction(Motor_t motor, Direction_t direction)
{ 
  uint8_t cur_speed = 127;

  // Motor forward 50%
  sprintf(serial_info_buffer, "Test motor %s, %d, direction %s (decaymode: %s)\n", motor_strings[motor], cur_speed, direction_strings[direction], decaymode_strings[CUR_DECAY_MODE]);
  Serial.write(serial_info_buffer);
  set_speed(motor, cur_speed, direction);
  delay(DELAY_BETWEEN_TESTS_MS);

  // Motor forward 100%
  cur_speed = 255;
  sprintf(serial_info_buffer, "Test motor %s, %d, direction %s (decaymode: %s)\n", motor_strings[motor], cur_speed, direction_strings[direction], decaymode_strings[CUR_DECAY_MODE]);
  Serial.write(serial_info_buffer);
  set_speed(motor, cur_speed, direction);
  delay(DELAY_BETWEEN_TESTS_MS);

  // Motor coast/brake
  cur_speed = 0;
  sprintf(serial_info_buffer, "Test motor %s, %d, direction %s (decaymode: %s)\n", motor_strings[motor], cur_speed, direction_strings[direction], decaymode_strings[CUR_DECAY_MODE]);
  Serial.write(serial_info_buffer);
  set_speed(motor, cur_speed, NOT_RELEVANT);
  delay(DELAY_BETWEEN_TESTS_MS);
  coast_motor(motor); // make sure the motor signals are 0 after the test
}

static void test_direction(Direction_t direction)
{
  test_motor_direction(A, direction);
  test_motor_direction(B, direction);

  sprintf(serial_info_buffer, "End of test %s (decaymode: %s)\n", direction_strings[direction], decaymode_strings[CUR_DECAY_MODE]);
  Serial.write(serial_info_buffer);
}

static void reset_motors()
{
  // set all DRV8833 inputs to 0 by setting the motors to coast
  coast_motor(A);
  coast_motor(B);
  Serial.write("Reset both motors to coast\n");
}

void loop() 
{
  // put your main code here, to run repeatedly:
  Serial.write("Inside loop\n");
  delay(3000);

  // Test forward fast decay
  Serial.write("Test Forward Fast decay/coast\n");
  CUR_DECAY_MODE = FAST;
  test_direction(FORWARD);
   // set all DRV8833 inputs to 0 by setting the motors to coast
  reset_motors();

  // Test forward slow decay
  Serial.write("Test Forward Slow decay/brake\n");
  CUR_DECAY_MODE = SLOW;
  test_direction(FORWARD);
  // set all DRV8833 inputs to 0 by setting the motors to coast
  reset_motors();

  // Test reverse fast decay
  Serial.write("Test reverse Fast decay/coast\n");
  CUR_DECAY_MODE = FAST;
  test_direction(REVERSE);
  // set all DRV8833 inputs to 0 by setting the motors to coast
  reset_motors();

  // Test reverse slow decay
  Serial.write("Test reverse Slow decay/brake\n");
  CUR_DECAY_MODE = SLOW;
  test_direction(REVERSE);
  // set all DRV8833 inputs to 0 by setting the motors to coast
  reset_motors();
  Serial.write("\n\nReset and wait 2 seconds\n\n");
  delay(2000);

  // Done testing
  Serial.write("Done with testloop\n\n");
}
