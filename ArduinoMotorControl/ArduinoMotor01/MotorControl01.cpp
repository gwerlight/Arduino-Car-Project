/*
Name:		ArduinoMotor01.ino
Created:	31.12.2016 08:18:26
Author:	Christian Daumeter
*/

#include "MotorControl01.h"

void MotorControl01Class::init()
{
	pinMode(MOTOR1_IN1, OUTPUT);
	pinMode(MOTOR1_IN2, OUTPUT);
	pinMode(MOTOR2_IN1, OUTPUT);
	pinMode(MOTOR2_IN2, OUTPUT);
	pinMode(MOTOR1_POWER, OUTPUT);
	pinMode(MOTOR2_POWER, OUTPUT);
}

void MotorControl01Class::setM1(motor_direction aValue) {
	switch (aValue) {
	case STOP:
		digitalWrite(MOTOR1_IN1, LOW);
		digitalWrite(MOTOR1_IN2, LOW);
		break;
	case BACKWARD:
		digitalWrite(MOTOR1_IN1, HIGH);
		digitalWrite(MOTOR1_IN2, LOW);
		break;
	case FORWARD:
		digitalWrite(MOTOR1_IN1, LOW);
		digitalWrite(MOTOR1_IN2, HIGH);
		break;
	}
}

void MotorControl01Class::setM2(motor_direction aValue) {
	switch (aValue) {
	case STOP:
		digitalWrite(MOTOR2_IN1, LOW);
		digitalWrite(MOTOR2_IN2, LOW);
		break;
	case BACKWARD:
		digitalWrite(MOTOR2_IN1, LOW);
		digitalWrite(MOTOR2_IN2, HIGH);
		break;
	case FORWARD:
		digitalWrite(MOTOR2_IN1, HIGH);
		digitalWrite(MOTOR2_IN2, LOW);
		break;
	}
}

void MotorControl01Class::setPower(int aValue) {
	setM1Power(aValue);
	setM2Power(aValue);
}

void MotorControl01Class::setM1Power(int aValue) {
	if (aValue < MINSPEED) {
		aValue = 0;
	};
	if (aValue > MAXSPEED) {
		aValue = MAXSPEED;
	};
	String speedString(aValue, DEC);
	//Serial.println("Set M1 Speed : " + speedString);

	analogWrite(MOTOR1_POWER, aValue);
}

void MotorControl01Class::setM2Power(int aValue) {
	if (aValue < MINSPEED) {
		aValue = 0;
	};
	if (aValue > MAXSPEED) {
		aValue = MAXSPEED;
	};
	String speedString(aValue, DEC);
	//Serial.println("Set M2 Speed : " + speedString);

	analogWrite(MOTOR2_POWER, aValue);
}

MotorControl01Class MotorControl01;

