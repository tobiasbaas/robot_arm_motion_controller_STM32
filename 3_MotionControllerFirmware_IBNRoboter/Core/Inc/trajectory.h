/***************************************************************************
 *  Trajectory-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _TRAJECTORY_H
#define _TRAJECTORY_H

#include "typedefs.h"
#include "parameter.h"
#include "axis.h"

// forward declaration
class axis_class;

typedef enum {
        PHASE_STANDSTILL = 0,
        PHASE_MOVE        = 1,
        PHASE_ACCELERATE  = 2
    } MovePhase;

class trajectory_class
{
private:
	axis_class* 		myAxis;
	parameter_class* 	myParameter;
	error_class* 		myError;

	// time Parameters
	double delta_t;
	double delta_t2;

	//private values (time base servo loop)
	float d_s_max;				// maximum velocity
	float dd_s_max_acc;			// maximum acceleration
	float dd_s_max_dec;			// maximum deceleration
	float d_s;					// actual velocity
	float dd_s;					// actual acceleration/deceleration
	float act_pos;				// actual position


public:
	trajectory_class();
	bool setParameter(CMD_STRUCT* cmd);
	bool set_target(float target);
	bool calc(void);
	bool refresh_parameter(void);
	void init(axis_class* Axis_ptr,parameter_class* myParameter_ptr,error_class* myError_ptr);
	void motion_init(void);

	#define MAX_NUM_PARAMETER_TRAJEKTORY 6
    PARAM_DEV module_parameter[MAX_NUM_PARAMETER_TRAJEKTORY] =
    {
    	{0x01, _UINT16, 	MAX_NUM_PARAMETER_TRAJEKTORY, 			0, 100, 	true, "Number of Trajectory Module Parameters"}, // every Module needs this parameter with number of
		{0x10, _FLOAT, 		{.f32=DEFAULT_MAX_VELOCITY}, 			0, 1000, 	false, "Maximum Velocity of Axis"},
        {0x11, _FLOAT, 		{.f32=DEFAULT_MAX_ACCELERATION}, 		0, 500, 	false, "Maximum Acceleration of Axis"},
		{0x12, _FLOAT, 		{.f32=DEFAULT_MAX_ACCELERATION}, 		0, 500, 	false, "Maximum Deceleration of Axis"},
    	{0x21, _FLOAT, 		{.f32=DEFAULT_MAX_TRAVELRANGE}, 		0, 1000, 	false, "Maximum Travel Range of Axis"},		// [4]
    	{0x22, _FLOAT, 		{.f32=DEFAULT_MIN_TRAVELRANGE}, 		0, 1000, 	false, "Minimum Travel Range of Axis"}
    };


    PARAM_DEV *module_parameter_ptr = &module_parameter[0];

    MovePhase move_phase;

	// initial values
    float slowed_target;	// position of trajectory module
    float vel;				// actual velocity of trajectory module
    float acc;				// actual acceleration of trajectory module
    float max_travel_limit;	// Maximum travel limit
	float min_travel_limit;	// minimum travel limit
    float setpoint;			// position target
    float vel_max;			// maximum allowed velocity
    float acc_max;			// maximum allowed acceleration
    float dec_max;			// maximum allowed deceleration
    float s_delta ;			// difference actual position to target position
    float s_br;				// brake distance at current velocity
    bool motion_on;			// motion in progress or standstill
    bool ont;				// on target flag
};

#endif
