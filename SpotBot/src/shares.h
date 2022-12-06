/** @file shares.h
 *  This file contains extern declarations of shares and queues which are used
 *  in more than one file of SpotBot project.
 * 
 *  @author JR Ridgely
 *  @author Christian Clephan
 *  @date   2021-Oct-23 Original file
 *  @date   2022-Nov-25 Edited for purposes of SpotBot project.
 *  @copyright (c) 2021 by JR Ridgely, released under the LGPL 3.0. 
 */

#ifndef _SHARES_H_
#define _SHARES_H_

#include "taskqueue.h"
#include "taskshare.h"

// A share which holds boolean whether or not to be spotted
extern Share<bool> spot_me_bro;

// A share which holds boolean whether spotting is completed or not
extern Share<bool> spot_complete;

// A share which holds boolean whether to send data or not
extern Share<bool> send_data;

// A queue which triggers a task to print the count at certain times
extern Queue<float> vel_queue;

// A queue which holds time values to be displayed by task_webserver
extern Queue<float> timer;

// A queue which holds right IMU velocity values to be displayed by task_webserver
extern Queue<float> vel_r;

// A queue which holds left IMU velocity values to be displayed by task_webserver
extern Queue<float> vel_l;

#endif // _SHARES_H_