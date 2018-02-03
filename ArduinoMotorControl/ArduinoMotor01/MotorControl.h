#pragma once
// MotorControl01.h

#ifndef _MOTORCONTROL_h
#define _MOTORCONTROL_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

typedef enum { STOP, FORWARD, BACKWARD } motor_direction;

// Motoranschluﬂ
static const int MOTOR1_IN1 = 6;
static const int MOTOR1_IN2 = 7;
static const int MOTOR2_IN1 = 8;
static const int MOTOR2_IN2 = 13;

static const int MOTOR1_POWER = 9;
static const int MOTOR2_POWER = 10;

// Laufsteuerung
static const int MAXSPEED = 230;
static const int MINSPEED = 40;

class MotorControl01Class
{
protected:


public:
	void init();

	void setM1(motor_direction aValue);

	void setM2(motor_direction aValue);

	void setPower(int aValue);

	void setM1Power(int aValue);

	void setM2Power(int aValue);
};

extern MotorControl01Class MotorControl01;

#endif

