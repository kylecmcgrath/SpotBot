/** @file motor_driver.cpp
 *  This program includes the motor driver class for creating motor objects although only one motor
 *  is used for our project. The motor driver can take two motors, although we are using one and still
 *  include the capability to attach two.
 * 
 *  @author Christian Clephan
 *  @date   11-16-22
 */

#include "motor_driver.h"

/** @brief   Constructor which creates a motor driver object.
 */
MotorDriver::MotorDriver (void)
{
  pinMode(INA1, OUTPUT);
  digitalWrite(INA1, HIGH);

  pinMode(INA2, OUTPUT);
  digitalWrite(INA2, LOW);

  pinMode(PWMA, OUTPUT);
}

void MotorDriver::set_duty(int16_t duty)
{
    if (duty >= 0){
        //Positive duty = CW = positive position
        digitalWrite(INA1, HIGH);
        digitalWrite(INA2, LOW);
        analogWrite(PWMA, duty);
    }
    else{
        //Negative duty = CCW = negative position
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, HIGH);
        analogWrite(PWMA, -duty); 
    }
}

void MotorDriver::stop(void){
    analogWrite(PWMA, 0);
}

void MotorDriver::SOS(void){
    digitalWrite(INA1, HIGH);
    digitalWrite(INA2, LOW);
    analogWrite(PWMA, MAX_DUTY);
}