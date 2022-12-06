/** @file task_spot.cpp
 *  This program includes spot task which reads velocity values obtained from task_IMU
 *  determine where the lifter is in the lift and if they need a spot or not. This starts
 *  with setting the spot timer (timer_counter) to 0 and making sure the bar is laying
 *  still. This moves to the next state so it is confirmed the bar is racked and the user
 *  hasn't started to lift. Once the IMUs both see downward motion this moves into the descent
 *  state and starts a timer on limiting the length of the rep.  If the bar is stopped again
 *  the user now has the barbell on their chest where the IMU will now check for upward motion.
 *  If the barbell moves up then down this counts as a fail and starts the motor, otherwise if the
 *  bar reaches a standstill again the bar must be racked (counting as 1 rep). The spot is also
 *  initiated if spot timer reaches 100 (a.k.a. roughly 5 seconds). If the spot is initiated the
 *  IMU values aren't read anymore and the slack must be reset on the robot.
 * 
 *  @author Christian Clephan
 *  @date   11-26-22
 */
#include <Arduino.h>
#include <PrintStream.h>
#include "task_spot.h"
#include "shares.h"

float r_vel;
float l_vel;
uint8_t state_spot = 0;
uint16_t timer_counter = 0;
uint8_t rep_counter = 0;
uint16_t spot_counter = 0;
uint8_t max_time = 60;

Share<bool> send_data("Send data");
Share<bool> spot_me_bro("Spot Trigger");


/** @brief Task motor interfaces with other tasks shares to turn on and off motor 
 *  @details Interrets the velocity queue values from task IMU to find where the
 *  lifter is in the program. The states go in order of barbell racked, bar descending,
 *  bar stopped at chest, bar moving upward, bar re-racked. If the rep takes to long or
 *  the bar begins descending if it should be moving upward then a spot is requested.
*/
void task_spot(void* p_params){
    while(1){
        if(state_spot != 6){
            r_vel = vel_queue.get();
            l_vel = vel_queue.get();
        }
        if(state_spot == 0){
            timer_counter = 0;
            if(r_vel == 0 && l_vel == 0){
                state_spot = 1;
            }
        }
        if(state_spot == 1){
            timer_counter = 0;
            Serial << "Bar is racked" << endl;
            if(r_vel < 0 && l_vel < 0){
                state_spot = 2;
            }
            }
        if (state_spot == 2){
            Serial << "Bar is in descent | Rep timer: " << timer_counter << endl;
            timer_counter++;
            if ((r_vel == 0 && r_vel == 0
            )){
                state_spot = 3;
            }
             if(timer_counter >= max_time){
                state_spot = 5;
            }           
        }
        if (state_spot == 3){
            Serial << "Bar is stopped at chest | Rep timer: " << timer_counter << endl;
            timer_counter++;
            if(r_vel > 0 && l_vel > 0){
                state_spot = 4;
                
            }
            if(timer_counter >= max_time){
                state_spot = 5;
            }
        }
        if (state_spot == 4){
            Serial << "Bar is going up | Rep timer: " << timer_counter << endl;
            timer_counter++;
            if (r_vel < -0.06 && l_vel < -0.06){
                state_spot = 5;
            }
            if (r_vel == 0 && l_vel == 0){
                rep_counter++;
                Serial << "Nice bench bro you've done " << rep_counter << " rep(s)" << endl;
                send_data.put(1);
                state_spot = 1;
                
            }
            if(timer_counter >= max_time){
                state_spot = 5;
            }
        }
        if (state_spot == 5){
            Serial << "Rep failed spotting initiated" << endl;
            spot_me_bro.put(1);
            send_data.put(1);
            if(spot_complete.get()){
                state_spot = 6;
            }
        }
        if (state_spot == 6){
            Serial << "Spot finished reset slack" << endl;
        }
        vTaskDelay(50);
    }
}