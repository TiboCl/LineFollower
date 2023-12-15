#include "arduino_secrets.h"
/* 
  Sketch generated by the Arduino IoT Cloud Thing "Untitled 2"
  https://create.arduino.cc/cloud/things/78f3fc69-e1b2-41cb-b69e-32d851cf3bda 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  int d;
  int i;
  int kd;
  int ki;
  int kp;
  int motor_base_speed;
  int motor_max_speed;
  int p;
  bool calibrate;
  bool is_calibrated;
  bool is_running;
  bool start;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/
#include "thingProperties.h"

typedef enum 
{
  FAST,
  SLOW
} DecayMode_t;

typedef enum 
{
  Motor_A,
  Motor_B
} Motor_t;

typedef enum
{
  FORWARD,
  REVERSE,
  NOT_RELEVANT
} Direction_t;

#define MAX_ANALOG_VAL (1023)

const int PIN_DRV8833_B_IN1 = 12;
const int PIN_DRV8833_B_IN2 = 11;
const int PIN_DRV8833_A_IN2 = 10;
const int PIN_DRV8833_A_IN1 = 9;

const uint8_t maxspeed_init = 100;
const uint8_t basespeed_init = 50; // -> kp = motor_base_speed /2500

const float kp_init = 0.014;
const float ki_init = 0.0;
const float kd_init = 0.0;

float real_kp;
float real_ki;
float real_kd;

const uint8_t SensorCount = 6;
const uint8_t sensorPins[SensorCount] = {A7, A6, A3, A2, A1, A0};
uint16_t _lastPosition = 0;
uint8_t _samplesPerSensor = 4; // only used for analog sensors
int lastError = 0;
const int buttonPin = 2;
int buttonState = LOW;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

static const char* const direction_strings[] = { "FORWARD", "REVERSE", "NOT_RELEVANT", 0 };
static const char* const motor_strings[] = { "A", "B", 0 };
static const char* const decaymode_strings[] = { "FAST", "SLOW", 0 };

DecayMode_t CUR_DECAY_MODE = FAST;

uint16_t sensorValues[SensorCount];
uint16_t maxSensorValues[SensorCount];
uint16_t minSensorValues[SensorCount] = {MAX_ANALOG_VAL};

static char serial_info_buffer[500] = {0};

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
    if(motor == Motor_A){
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
    if(motor == Motor_A){
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
    if(motor == Motor_A){
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
    if(motor == Motor_A){
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
  if(motor == Motor_A){
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
  if(motor == Motor_A){
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

void read(uint16_t * sensorValues)
{
  uint8_t start = 0;
  uint8_t step= 1;

  // reset the values
  for (uint8_t i = start; i < SensorCount; i += step)
  {
    sensorValues[i] = 0;
  }

  for (uint8_t j = 0; j < _samplesPerSensor; j++)
  {
    for (uint8_t i = start; i < SensorCount; i += step)
    {
      // add the conversion result
      sensorValues[i] += (uint16_t) analogRead(sensorPins[i]);
    }
  }

  // get the rounded average of the readings for each sensor
  for (uint8_t i = start; i < SensorCount; i += step)
  {
    // _samplesPerSensor >> 1 == _samplesPerSensor / 2 maar sneller voor de microcontroller
    sensorValues[i] = (sensorValues[i] + (_samplesPerSensor >> 1)) / _samplesPerSensor;
  }
  return;
}


void readCalibrated(uint16_t * sensorValues)
{
  if(!is_calibrated)
  {
      return;
  }

  // read the needed values
  read(sensorValues);
  sprintf(serial_info_buffer, "readCalibrated 0: sensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3], sensorValues[4], sensorValues[5]);
  Serial.write(serial_info_buffer);

  for (uint8_t k = 0; k < SensorCount; k++)
  {
    uint16_t calmin = minSensorValues[k];
    uint16_t calmax = maxSensorValues[k];

    uint16_t denominator = calmax - calmin;
    int16_t value = 0;

    if (denominator != 0)
    {
      value = (((int32_t)sensorValues[k]) - calmin) * 1000 / denominator;
    }

    if (value < 0) { value = 0; }
    else if (value > 1000) { value = 1000; }

    sensorValues[k] = value;
  }
  sprintf(serial_info_buffer, "readCalibrated 1: sensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3], sensorValues[4], sensorValues[5]);
  Serial.write(serial_info_buffer);
}

uint16_t readLineBlack(uint16_t * sensorValues)
{
  //mode = on
  bool invertReadings = false;
  bool onLine = false;
  uint32_t avg = 0; // this is for the weighted total
  uint16_t sum = 0; // this is for the denominator, which is <= 64000

      //sprintf(serial_info_buffer, "buttonState 1.1: %d\n", buttonState);
      //Serial.write(serial_info_buffer);
  readCalibrated(sensorValues);

      //sprintf(serial_info_buffer, "buttonState 1.2: %d\n", buttonState);
      //Serial.write(serial_info_buffer);
  for (uint8_t k = 0; k < SensorCount; k++)
  {
    uint16_t value = sensorValues[k];

    // keep track of whether we see the line at all
    if (value > 120) { onLine = true; }

    // only average in values that are above a noise threshold
    if (value > 80)
    {
      avg += (uint32_t)value * (k * 1000);
      sum += value;
    }
  }
      //sprintf(serial_info_buffer, "buttonState 1.3: %d\n", buttonState);
      //Serial.write(serial_info_buffer);

  if (!onLine)
  {
    // If it last read to the left of center, return 0.
    if (_lastPosition < (SensorCount - 1) * 1000 / 2)
    {
      return 0;
    }
    // If it last read to the right of center, return the max.
    else
    {
      return (SensorCount - 1) * 1000;
    }
  }

  _lastPosition = avg / sum;
  return _lastPosition;
}


void PID_control() {
  //    sprintf(serial_info_buffer, "buttonState 1: %d\n", buttonState);
  //    Serial.write(serial_info_buffer);
  uint16_t position = readLineBlack(sensorValues);
  //    sprintf(serial_info_buffer, "buttonState 2: %d\n", buttonState);
  //    Serial.write(serial_info_buffer);
  int error = 2500 - position;

  pid_p = error;
  pid_i = pid_i + error;
  pid_d = error - lastError;
  lastError = error;
  int speed_adjustment = pid_p*real_kp + pid_i*real_ki + pid_d*real_kd;
  
  motorspeed_a = motor_base_speed + speed_adjustment;
  motorspeed_b = motor_base_speed - speed_adjustment;
  
  if (motorspeed_a > motor_max_speed) {
    motorspeed_a = motor_max_speed;
  }
  if (motorspeed_b > motor_max_speed) {
    motorspeed_b = motor_max_speed;
  }
  if (motorspeed_a < -motor_max_speed) {
    motorspeed_a = -motor_max_speed;
  }
  if (motorspeed_b < -motor_max_speed) {
    motorspeed_b = -motor_max_speed;
  }
  if(motorspeed_a >= 0){
    set_speed(Motor_A, motorspeed_a, FORWARD);
  }else{
    set_speed(Motor_A, -motorspeed_a, REVERSE);
  }
  if(motorspeed_b >= 0){
    set_speed(Motor_B, motorspeed_b, FORWARD);
  }else{
    set_speed(Motor_B, -motorspeed_b, REVERSE);
  }
  sprintf(serial_info_buffer, "PID_control: position: %d, P: %d, I: %d, D: %d, motorspeed_a: %d, motorspeed_b: %d\n", position, pid_p, pid_i, pid_d, motorspeed_a, motorspeed_b);
  Serial.write(serial_info_buffer);
  //    sprintf(serial_info_buffer, "buttonState 3: %d\n", buttonState);
  //    Serial.write(serial_info_buffer);
  sprintf(serial_info_buffer, "PID_control: sensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3], sensorValues[4], sensorValues[5]);
  Serial.write(serial_info_buffer);
  
  //    sprintf(serial_info_buffer, "buttonState 4: %d\n", buttonState);
  //    Serial.write(serial_info_buffer);
  
  //set_speed(Motor_A, 0, FORWARD);
  //set_speed(Motor_B, 0, FORWARD);
}

void calibration() {
  Serial.write("Start calibration\n");

  // alles op de default waarde zetten
  for (uint8_t k = 0; k < SensorCount; k++)
  {
    // _samplesPerSensor >> 1 == _samplesPerSensor / 2 maar sneller voor de microcontroller
    minSensorValues[k] = MAX_ANALOG_VAL;
    maxSensorValues[k] = 0;
  }

  for (uint16_t k = 0; k < 10; k++)
  {
    read(sensorValues);
    sprintf(serial_info_buffer, "calibration: sensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3], sensorValues[4], sensorValues[5]);
    Serial.write(serial_info_buffer);
    delay(100);
  }

  Serial.write("done values\n");

  for (uint16_t k = 0; k < 500; k++)
  {
    read(sensorValues);
    for (uint8_t j = 0; j < SensorCount; j++)
    {
      if(sensorValues[j] < minSensorValues[j])
      {
        minSensorValues[j] = sensorValues[j];
      }
      if(sensorValues[j] > maxSensorValues[j])
      {
        maxSensorValues[j] = sensorValues[j];
      }
    }
    if(k%10 == 0){
      Serial.write(".");
      ArduinoCloud.update();
    }
    delay(10);
  }
  
  sprintf(serial_info_buffer, "calibration: minsensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", minSensorValues[0], minSensorValues[1], minSensorValues[2], minSensorValues[3], minSensorValues[4], minSensorValues[5]);
  Serial.write(serial_info_buffer);
  sprintf(serial_info_buffer, "calibration: maxsensor values: [ %04d, %04d, %04d, %04d, %04d, %04d]\n", maxSensorValues[0], maxSensorValues[1], maxSensorValues[2], maxSensorValues[3], maxSensorValues[4], maxSensorValues[5]);
  Serial.write(serial_info_buffer);
  Serial.write("done calibration\n");
  
  is_calibrated = true;
}

void stop_motors()
{
  set_speed(Motor_A, 0, NOT_RELEVANT);
  set_speed(Motor_B, 0, NOT_RELEVANT);
  
  coast_motor(Motor_A);
  coast_motor(Motor_B);
}

void setup() {
  delay(1500); 
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  Serial.write("Setup started\n");
  is_calibrated = false;

  delay(500);

  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(4);
  ArduinoCloud.printDebugInfo();
  
// alle analoge waarden uitlezen
// zelfde algoritme voor calibratie

  pinMode(PIN_DRV8833_B_IN1, OUTPUT);
  pinMode(PIN_DRV8833_B_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN2, OUTPUT);
  pinMode(PIN_DRV8833_A_IN1, OUTPUT);
  
  real_kp = kp_init;
  real_ki = ki_init;
  real_kd = kd_init;

  motor_base_speed = basespeed_init;
  motor_max_speed = maxspeed_init;

  Serial.write("Setup done\n");

/*
  */
  //stop_motors();
}

void toggle_running()
{
  Serial.write("Toggle running\n");
  is_running = !is_running;
  pid_p = 0;
  pid_i = 0;
  pid_d = 0;
}

void update_start_button()
{
  
  //Serial.write("Updating start button\n");
  int btn_value = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:
  
  //sprintf(serial_info_buffer, "btn_value: %d, lastButtonState: %d\n", btn_value, lastButtonState);
  //Serial.write(serial_info_buffer);

  // If the switch changed, due to noise or pressing:
  if (btn_value != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:      
    //sprintf(serial_info_buffer, "buttonState before: %d\n", buttonState);
    //Serial.write(serial_info_buffer);


    // if the button state has changed:
    if (btn_value != buttonState) {
      buttonState = btn_value;
      //sprintf(serial_info_buffer, "buttonState after: %d\n", buttonState);
      //Serial.write(serial_info_buffer);

      // only toggle the running state if the new button state is LOW
      if (buttonState == LOW) {
        toggle_running();
      }
    }
  }
  
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = btn_value;
  //delay(1000);

}

void loop() {
  Serial.write("start loop\n");
  ArduinoCloud.update();
  //Serial.write("update done\n");
  update_start_button();

  if (is_running == true) {
    Serial.write("Running\n");
    PID_control();
  }
  else {
    Serial.write("Not Running\n");
    stop_motors();
  }
}


void onStartChange()  {
  bool current_start = start;
  // Add your code here to act upon Start change
  sprintf(serial_info_buffer, "Start pressed: %d\n", current_start ? 1 : 0);
  Serial.write(serial_info_buffer);
    // Your code here 
  if(current_start) {
    toggle_running();
    if(is_running == true) {
      Serial.write("onoff is on\n");
      delay(1000);
    }
    else {
      Serial.write("onoff is off\n");
    }
  }
}

void onCalibrateChange()  {
  bool current_calibrate = calibrate;
  // Add your code here to act upon Calibrate change
  sprintf(serial_info_buffer, "Calibrate pressed: %d\n", current_calibrate ? 1 : 0);
  Serial.write(serial_info_buffer);

  if(current_calibrate) {
    calibration();
    calibrate = false;
  }
}

void onKdChange()  {
  // Add your code here to act upon Kd change
  
  sprintf(serial_info_buffer, "Kd changed: %d\n", kd);
  Serial.write(serial_info_buffer);
  real_kd = (float)kd/10000.0;
}


void onKiChange()  {
  // Add your code here to act upon Ki change
  sprintf(serial_info_buffer, "Ki changed: %d\n", ki);
  Serial.write(serial_info_buffer);
  real_ki = (float)ki/10000.0;
}


void onKpChange()  {
  // Add your code here to act upon Kp change
  sprintf(serial_info_buffer, "Kp changed: %d\n", kp);
  Serial.write(serial_info_buffer);
  real_kp = (float)kp/10000.0;
}

/*
  Since IsStarted is READ_WRITE variable, onIsStartedChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onIsStartedChange()  {
  // Add your code here to act upon IsStarted change
}

/*
  Since IsRunning is READ_WRITE variable, onIsRunningChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onIsRunningChange()  {
  // Add your code here to act upon IsRunning change
}

/*
  Since MotorBaseSpeed is READ_WRITE variable, onMotorBaseSpeedChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMotorBaseSpeedChange()  {
  // Add your code here to act upon MotorBaseSpeed change
}

/*
  Since MotorMaxSpeed is READ_WRITE variable, onMotorMaxSpeedChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMotorMaxSpeedChange()  {
  // Add your code here to act upon MotorMaxSpeed change
}