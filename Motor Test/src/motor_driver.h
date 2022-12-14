/** @file motor_driver.h
 *  This is the header for the motor driver file
 * 
 *  @author Christian Clephan
 */

#include <Arduino.h>
#include "PrintStream.h"

#define INA1 12
#define INA2 14
#define PWMA 27

#define INB1 0
#define INB2 2
#define PWMB 4
#define MAX_DUTY 255

/** @brief   Class which drives geared dc motor
 * 
 */
class MotorDriver
{
protected:
    int16_t duty = 0;
public:
    MotorDriver (void);
    void set_duty(int16_t);
    void stop(void);
    void SOS(void);
};