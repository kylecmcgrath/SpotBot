/** @file task_motor.cpp
 *  This program includes motor task which interfaces with the motor by creating an object
 *  of the motor driver class and turning on/off the motor based on other task data. This
 *  also includes encoder functionality using an ISR to track motor position. Task motor runs
 *  through two states where it is off checking if there is a spot needed or turned on and
 *  checking if it has reached the rack.
 * 
 *  @author Christian Clephan
 *  @date   11-25-22
 */

#include "motor_driver.h"
#include "task_motor.h"
#include "PrintStream.h"
#include "shares.h"
#include <Arduino.h>

#define OUTA 36
#define OUTB 39

uint8_t state = 0;
MotorDriver motor;
bool spot = 0;

bool OUTA_val1 = digitalRead(OUTA);
bool OUTA_val2 = digitalRead(OUTA);
bool OUTB_val1 = digitalRead(OUTB);
int32_t counter = 0;
float calib_coeff = (3.4/4096)/2; //converting ticks to revolutions
float rev_to_mm = 2*3.1415*3;
float pos = 0;
int16_t my_duty = 100;
float encoder_pos = 0;
float spot_distance = 207; //(distance from bench to highest rack - depth of persons chest) 

Share<bool> spot_complete("Is complete?");

/** @brief ISR that updates encoder count when there is a change in encoder digitalRead value
*/
void update_pos(){
  OUTA_val1 = digitalRead(OUTA);
  if (OUTA_val1 != OUTA_val2 && OUTA_val1 == 1){
    if (OUTB_val1 == OUTA_val1){
      counter ++;
    }
    else{
      counter --;
    }
  }
  OUTA_val2 = OUTA_val1;
}

/** @brief Task motor interfaces with other tasks shares to turn on and off motor 
 *  @details First the pins for the encoder are set and the ISRs are set to run when
 *  digitalREAD of either pin OUTA or OUTB is changed. Next, the state machine is run
 *  starting at checking if a spot is needed, and if so spinning the motor and transitioning 
 *  states. In the next state the encoder is used to track motor position to if the barbell
 *  has been pulled enough to reach the safety rack.
*/
void task_motor(void* p_params){
    pinMode(OUTA, INPUT);
    pinMode(OUTB, INPUT);
    attachInterrupt(OUTA, update_pos, CHANGE);
    attachInterrupt(OUTB, update_pos, CHANGE);
    while(1){
        if (state == 0){
            spot = spot_me_bro.get();
            if(spot){
                motor.set_duty(my_duty); //This can be varied depending on the weight or if it needs to go faster
                state = 1;
            }
        }
        if (state == 1){
            encoder_pos = counter*calib_coeff*rev_to_mm; //convert ticks to revolutions and revolutions to millimeters
            if (my_duty > 0){    //if duty is positive flip sign
                encoder_pos *= -1;
            }
            Serial << encoder_pos << endl;
            if(encoder_pos >= spot_distance){ //Checks if motor has pulled passed distance between user and rack
                state = 2;
            }
        }
        if (state == 2){
          motor.stop();
          spot_complete.put(1); //Set spot complete share to true
          state = 0;
          spot_me_bro.put(0); //We no longer need a spot
        }
        vTaskDelay(50);
    }

}