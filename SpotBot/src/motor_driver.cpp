/** @file motor_driver.cpp
 *  This program contains the class for a motor driver allowing the user to
 *  vary the duty, stop, and set the motor to full blast.
 * 
 *  @author Christian Clephan
 *  @date   11-16-22
 */

#include "motor_driver.h"

/** @brief   Constructor which creates a motor driver object.
 */
MotorDriver::MotorDriver (void)
{
  pinMode(INB1, OUTPUT);
  digitalWrite(INB1, HIGH);

  pinMode(INB2, OUTPUT);
  digitalWrite(INB2, LOW);

  pinMode(PWMB, OUTPUT);
}

/** @brief   Method which sets the duty of a motor in CW (+) or CCW (-)
 */
void MotorDriver::set_duty(int16_t duty)
{
    if (duty >= 0){
        digitalWrite(INB1, HIGH);
        digitalWrite(INB2, LOW);
        analogWrite(PWMB, duty);
    }
    else{
        digitalWrite(INB1, LOW);
        digitalWrite(INB2, HIGH);
        analogWrite(PWMB, -duty); 
    }
}

/** @brief   Method which stops the motor
 */
void MotorDriver::stop(void){
    analogWrite(PWMB, 0);
}

/** @brief   Method which sets the motor at max speed CW.
 */
void MotorDriver::SOS(void){
    digitalWrite(INB1, HIGH);
    digitalWrite(INB2, LOW);
    analogWrite(PWMB, MAX_DUTY);
}