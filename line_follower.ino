#include <QTRSensors.h>
const int m11Pin = 7;
const int m12Pin = 6;
const int m21Pin = 5;
const int m22Pin = 4;
const int m1Enable = 11;
const int m2Enable = 10;

 

int m1Speed = 0;
int m2Speed = 0;

 


// increase kp’s value and see what happens


 float kp = 12;
float  ki = 0.01;
 float kd = 3;



 

int p = 1;
int i = 0;
int d = 0;

 

int error = 0;
int lastError = 0;

 

const int maxSpeed = 255;
const int minSpeed = -255;

 

const int baseSpeed = 220;
const int calibrationSpeed = 200;

 

QTRSensors qtr;

 

const int sensorCount = 6;
int sensorValues[sensorCount];
int sensors[sensorCount] = {0, 0, 0, 0, 0, 0};
void setup() {

 

  // pinMode setup
  pinMode(m11Pin, OUTPUT);
  pinMode(m12Pin, OUTPUT);
  pinMode(m21Pin, OUTPUT);
  pinMode(m22Pin, OUTPUT);
  pinMode(m1Enable, OUTPUT);
  pinMode(m2Enable, OUTPUT);

  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5}, sensorCount);

 
 calibrateSensor();
 Serial.begin(9600);

}

 

void loop() {
  // inefficient code, written in loop. You must create separate functions
  int error = map(qtr.readLineBlack(sensorValues), 0, 5000, -50, 50);
 // pidControl(error, kp, kd, ki);

 

  p = error;
  i = i + error;
  d = error - lastError;
  lastError = error;

 

  int motorSpeed = kp * p + ki * i + kd * d; // = error in this case

  m1Speed = baseSpeed;
  m2Speed = baseSpeed;

 

  // a bit counter intuitive because of the signs
  // basically in the first if, you substract the error from m1Speed (you add the negative)
  // in the 2nd if you add the error to m2Speed (you substract the negative)
  // it's just the way the values of the sensors and/or motors lined u p
  if (error < 0) {
    m1Speed += motorSpeed;
  }
  else if (error > 0) {
    m2Speed -= motorSpeed;
  }
  // make sure it doesn't go past limits. You can use -255 instead of 0 if calibrated programmed properly.
  // making sure we don't go out of bounds
  // maybe the lower bound should be negative, instead of 0? This of what happens when making a steep turn
  m1Speed = constrain(m1Speed,-80 , maxSpeed);
  m2Speed = constrain(m2Speed, -80, maxSpeed);

 


  setMotorSpeed(m1Speed, m2Speed);

 

  
//  DEBUGGING
//  Serial.print("Error: ");
//  Serial.println(error);
//  Serial.print("M1 speed: ");
//  Serial.println(m1Speed);
//
//  Serial.print("M2 speed: ");
//  Serial.println(m2Speed);
//
//  delay(250);
}

 


// calculate PID value based on error, kp, kd, ki, p, i and d.
void pidControl(int error, float kp, float kd, float ki) {
Serial.println(error);

if (error <= 15 || error >= -15)
{
 kp = 12;
 ki = 0.1;
 kd = 3;

}

if ((error <= 25 || error >= -25) && (error > 15 || error < -15))
{
  kp = 8;
  ki = 0.01;
  kd = 2.2;

}

if ((error <= 40 || error >= -40) && (error > 25 || error < -25))
{
  kp = 13;
  ki = 0.01;
  kd = 3;
}
Serial.println(kp);
Serial.println(ki);
Serial.println(kd);




}

 


// each arguments takes values between -255 and 255. The negative values represent the motor speed in reverse.
void setMotorSpeed(int motor1Speed, int motor2Speed) {
  // remove comment if any of the motors are going in reverse 
   motor1Speed = -motor1Speed;
  //  motor2Speed = -motor2Speed;
  if (motor1Speed == 0) {
    digitalWrite(m11Pin, LOW);
    digitalWrite(m12Pin, LOW);
    analogWrite(m1Enable, motor1Speed);
  }
  else {
    if (motor1Speed > 0) {
      digitalWrite(m11Pin, HIGH);
      digitalWrite(m12Pin, LOW);
      analogWrite(m1Enable, motor1Speed);
    }
    if (motor1Speed < 0) {
      digitalWrite(m11Pin, LOW);
      digitalWrite(m12Pin, HIGH);
      analogWrite(m1Enable, -motor1Speed);
    }
  }
  if (motor2Speed == 0) {
    digitalWrite(m21Pin, LOW);
    digitalWrite(m22Pin, LOW);
    analogWrite(m2Enable, motor2Speed);
  }
  else {
    if (motor2Speed > 0) {
      digitalWrite(m21Pin, HIGH);
      digitalWrite(m22Pin, LOW);
      analogWrite(m2Enable, motor2Speed);
    }
    if (motor2Speed < 0) {
      digitalWrite(m21Pin, LOW);
      digitalWrite(m22Pin, HIGH);
      analogWrite(m2Enable, -motor2Speed);
    }
  }
}

void calibrateSensor()
{
 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode
  int move = 0; // 0 -> left 1 -> right
  
  unsigned long moveStartTime = millis();
  unsigned long moveDuration = 50;
  // calibrate the sensor. For maximum grade the line follower should do the movement itself, without human interaction.
  for (uint16_t i = 0; i < 200; i++)
  {
      if (move == 0)
      {
        // Move the motors left
      setMotorSpeed(-calibrationSpeed, 0);//calibrationSpeed);
      moveStartTime = millis();
      qtr.calibrate();
      // check if the line is not detected
    
      int line =  map(qtr.readLineBlack(sensorValues), 0, 5000, -50, 50);
      //delay(50);
      Serial.println(line);
      if (line > 49 ) {
        // line is passed, stop the motors
        setMotorSpeed(0, 0);
        Serial.println("stop left");
        move = 1;
        //delay(200);
      }
    }
    
    if (move == 1)
    {
      // Move the motors  right
      setMotorSpeed(calibrationSpeed, 0);//-calibrationSpeed);
      moveStartTime = millis();
      qtr.calibrate();
      // check if the line is detected
      int line =  map(qtr.readLineBlack(sensorValues), 0, 5000, -50, 50);
      //delay(50);
      Serial.println(line);
      if (line < -49) 
      {
        // line is detected, stop the motors
        setMotorSpeed(0, 0);
        Serial.println("stop right");
        move = 0;
        //delay(200);
      }

    }
    
    while (millis() - moveStartTime < moveDuration) 
    {
    // wait for movement to complete
    }

  }
  digitalWrite(LED_BUILTIN, LOW);
  // Stop the motors
  setMotorSpeed(0, 0);
}




