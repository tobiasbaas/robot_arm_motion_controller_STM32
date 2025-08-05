/***************************************************************************
 *  Current Control-Class
 *************************************************************************** 
 * 
 *  Copyright 2025 Tobias Baas
 *  tobias.baas@h-ka.de
 *
 *  VER: 00.01.00 // Erste Version Jan. 2025
*****************************************************************************/
#include "typedefs.h"
#include "axis.h"
#include "trajectory.h"
#include "main.h"
#include "set_pwm.h"

#ifndef _CURRENT_CONTROL_H
#define _CURRENT_CONTROL_H

typedef struct
{
	float	error;
	float	error_mem;
	float	pTerm;
	float	iTerm;
	float	pid_y_P;
	float	pid_y_I;
	float	pid_y;
	float	pid_y_max;
}PI_FOC;

class axis_class;
class set_pwm_class;
class trajectory_class;

class current_control_class : public parameter_class
{
private:
	axis_class* myAxis;
	set_pwm_class* myPWM;
	error_class* myError;
	parameter_class* myParameter;

	float mAngle = 0.0;

	bool refresh_parameter(void);

public:
    current_control_class();
    bool setParameter(CMD_STRUCT* cmd);
    bool run(void);
    void init(axis_class* Axis_ptr, parameter_class* myParameter_ptr, error_class* myError_ptr, set_pwm_class* myPWM_ptr, trajectory_class* trajectory);
    bool setAngle(float angle);
    float getPosition(void);


};

#endif

