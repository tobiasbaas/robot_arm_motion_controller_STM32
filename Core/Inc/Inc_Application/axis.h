/***************************************************************************
 *  Axis-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/

#ifndef _AXIS_H
#define _AXIS_H

#include "typedefs.h"
#include "parameter.h"
#include "trajectory.h"
#include "controller_error.h"
#include "parameter.h"
#include "pos_sensor.h"
#include "current_control.h"
#include "pid.h"
#include "data_recorder.h"

// forward declaration
class PID_class;
class pos_sensor_class;
class trajectory_class;
class data_recorder_class;
class current_control_class;

namespace axis_const
{
	constexpr float V_SOLL_DEFAULT = 5.0f;    // [rad/s] Standard-Solldrehzahl, mechanisch
    constexpr float V_SOLL_MAX     = 20.0f;   // [rad/s] Sicherheits-Clamp
    constexpr float LEAD_MAX = 0.1745f;    // [rad]  maximaler Vorhalt (ca. 10°)
   // constexpr float kp_iq    = 8.6f;     // [A/rad] (= 0,15 A/° der Vergleichsimplementierung)
    constexpr float IQ_MAX   = 1.5f;     // [A]
};

class axis_class    
{
    public:
        axis_class();
        bool init(uint8_t i,error_class* myError,trajectory_class* trajectory, pos_sensor_class* pos_sensor, PID_class* pid_pos, current_control_class* current_control, data_recorder_class* DR, parameter_class* myParameter);
        bool run(void);
        bool setParameter(CMD_STRUCT* cmd);
        bool refresh_parameter(void);
        bool set_target(float target);
        void enable(void);
        void disable(void);
        bool stop_axis(void);


        uint8_t 					my_instance;
        parameter_class*			myParameter;
        trajectory_class* 			myTrajectory;
        pos_sensor_class* 			myPos;
        PID_class* 					myPid_Pos;
        current_control_class* 		myCurrent_Control;
        error_class*				myError;
        data_recorder_class*		myDR;
        bool						enabled;	// enable axis to run Motor...

        #define MAX_NUM_PARAMETER_AXIS_MOD 3
        // parameter TYPES: _UINT64, _UINT32, _UINT16, _INT64, _INT32, _INT16, _FLOAT, _DOUBLE, _STRING, _BOOL, _INIT
        PARAM_DEV module_parameter[MAX_NUM_PARAMETER_AXIS_MOD] =
        {
            {0x01, _UINT16, 	MAX_NUM_PARAMETER_AXIS_MOD, 0, 0, true, "Number of Axis Module Parameters"}, // every Module needs this parameter with number of
            {0x21, _FLOAT, {.f32=8.6}, 0, 15, false, "P-Term for Positioncontrol"}, 
            {0x22, _FLOAT, {.f32=axis_const::V_SOLL_DEFAULT}, -20, 20, false, "Ramp Velocity [rad/s]"},
        };

        //float position;
        //float velocity;
        uint32_t servoload;				    // Servo load in % --> ok until 70%
        //float pos_travel_limit;			// positive travel limit
        //float neg_travel_limit;			// negative travel limit

        float theta_ref;    // für Winkelsollwertrampe
        float lag;          // Lastwinkel
        float kp_iq;        // P-Term für Positionsregler
        float v_soll;       // Rampen-/Solldrehzahl [rad/s], vorzeichenbehaftet

        bool firstLoop;

    private:
        /* data */      
};

#endif

