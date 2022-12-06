/** @file main.cpp
 *  This program includes task_IMU and creates/runs the other tasks using FreeRTOS. The program
 *  runs through tasks based off priority and timing in order to accomplish robot functionality.
 *  First data is collected using the IMUs whihc is sent to spot task to determine where the user
 *  is during the lift. This information determines whether the motor needs to be spun to save the
 *  user from being crushed.
 * 
 *  @author Christian Clephan
 *  @date   12-2-22
 */


#include <Arduino.h>
#include <PrintStream.h>
#include <Wire.h>
#include <cstdlib>
#include "taskqueue.h"
#include "shares.h"
#include "task_spot.h"
#include "task_motor.h"
#include "task_webserver.h"

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const int MPU_ADDR2 = 0x69;
float calib_const = 1825.5; // Calibrating IMU 1 acceleration to m/s^2
float calib_const2 = 1485.2; // Calibrating IMU 2 acceleration to m/s^2
float thresh = 0.3; // Threshold to get rid of acceleration noise when IMU isn't moving
float vel = 0; // IMU 1 current velocity
float vel2 = 0; // IMU 2 current velocity
float vel_init = 0; //IMU 1 initial velocity
float vel_init2 = 0; //IMU 2 initial velocity
uint8_t IMU_state = 0; //State variable for IMU task
uint16_t tim_count = 0;

uint16_t vel_size = 100; //Number of acceleration values averaged to calculate velocity

int16_t accelerometer_x, accelerometer_y, accelerometer_z, accelerometer_z_2; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

Queue<float> vel_queue(2, "Velocities");

Queue<float> vel_r(1000, "Right Velocities recorded over a rep");

Queue<float> vel_l(1000, "Left Velocities recorded over a rep");

Queue<float> timer(1000, "Time over rep");

/** @brief Task IMU grabs data from IMUs and converts into velocities to be used by other tasks
 *  @details First transmision is made between MCU and I2C devices where the IMUs are "woken up".
 *  Next, we combine the upper and lower byte values from the z acceleration data address to get
 *  the "tick" value of acceleration in z. This is then converted to m/s^2 and zero'd after some
 *  data analysis. The acceleration noise is set to zero and values are summed up in order to integrate.
 *  Velociteis are found through numerical integration and error propogation is somewhat mitigated and
 *  the values are put in their respective queues.  
*/
void task_IMU(void* p_params){
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU_ADDR2); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  while (1){
    if (IMU_state == 0){
      for (uint8_t i = 0; i < vel_size; i++){
      //Reading IMU 1
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x3F); // starting with register 0x3F (ACCEL_ZOUT_H) 
      Wire.endTransmission(false); // the parameter indicates that the ESP32 will send a restart. As a result, the connection is kept active.
      Wire.requestFrom(MPU_ADDR, 2, true); // request a total of 2 registers
      accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
      float az = accelerometer_z/calib_const - 9.81;  //Adjusting acceleration in z to be 0 m/s^2

      //Reading IMU 2
      Wire.beginTransmission(MPU_ADDR2);
      Wire.write(0x3F); // starting with register 0x3F (ACCEL_ZOUT_H) 
      Wire.endTransmission(false); // the parameter indicates that the ESP32 will send a restart. As a result, the connection is kept active.
      Wire.requestFrom(MPU_ADDR2, 2, true); // request a total of 2 registers
      accelerometer_z_2 = Wire.read()<<8 | Wire.read();
      float az_2 = accelerometer_z_2/calib_const2 - 9.81; //Adjusting acceleration in z to be 0 m/s^2

      //Using threshold value to get rid of acceleration noise
      if (abs(az) < thresh || abs(az_2) < thresh){
        az = 0;
        az_2 = 0;
      }
      //Serial << "IMU 1: " << az << " | IMU 2: " << az_2 << endl;
      //Summing velocities together to eventually average over 1 second (v = v0 + a*dt)
        vel += az;
        vel2 += az_2;
        vTaskDelay(1);
      }
      IMU_state = 1; //Done with acceleration data collection now we have to convert to velocity
    }
    if (IMU_state == 1){
      //New velocity being averaged and adding to previous velocity (NUMERICAL INTEGRATION)
      vel = vel/vel_size*.1 + vel_init;
      vel2 = vel2/vel_size*.1 + vel_init2;

      //IMU loses track of velocity due to error propogation so if it is stopped it may still read nonzero values
      //that don't change so this condition checks for that and sets velocity back to 0
      if(vel < vel_init+.02 && vel > vel_init-0.02){
        vel = 0;
        vel2 = 0;
      }
      //Setting initial velocities to new velocity to calculate for the next time the task is run
      vel_init = vel;
      vel_init2 = vel2;

      Serial << "IMU 1: " << vel << " | IMU 2: " << vel2 << endl;

      //Putting values into queues to be shared with other tasks and make use of data
      vel_queue.put(vel);
      vel_queue.put(vel2);
      tim_count++; // Counts roughly every 100ms (vtaskdelay value)*100 iterations
      timer.put(tim_count); 
      vel_r.put(vel);
      vel_l.put(vel2);

      IMU_state = 0;
    }
  }
}
void setup() {
  Serial.begin(115200);
  while (!Serial) { } 
  //Set up network connection for ESP32 to interface with PC
  setup_wifi();
  xTaskCreate(task_IMU, "IMU", 2048, NULL, 5, NULL);
  xTaskCreate(task_spot, "Ey you need a spot bro", 2048, NULL, 4, NULL);
  xTaskCreate(task_motor, "Motor go brrr", 2048, NULL, 3, NULL);
  xTaskCreate(task_webserver, "Handle Webserver", 8192, NULL, 2, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}