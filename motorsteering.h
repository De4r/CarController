#ifndef MOTORSTEERING_H
#define MOTORSTEERING_H

#include <QObject>
#include <QString>
#include <pigpiod_if2.h>
#include <stdio.h>
#include <stdlib.h>
#include <sma.h>
#include <iostream>


typedef void (*sonarCB_t)(float);


class MotorSteering : public QObject
{
    Q_OBJECT
public:
    MotorSteering(int pi, unsigned char pwmLeft, unsigned char pwmRight, unsigned char inLeft1, unsigned char inLeft2, unsigned char inRight3, unsigned char inRight4, QObject* parent=NULL);
    ~MotorSteering();

    int MAX_CAR_SPEED = 255;
    int MAX_CAR_INPUT_SPEED = 1000;
    int DEAD_RANGE = 400;
    int pi;
    unsigned char pwmLeft, pwmRight, inLeft1, inLeft2, inRight3, inRight4;
    int START_RAMP_VEL = 30;
    int STOP_RAMP_VEL = 10;
    int MIN_SPEED = 100;
    float slope1 = (MAX_CAR_SPEED-MIN_SPEED) / (MAX_CAR_INPUT_SPEED - pow(DEAD_RANGE, 0.5));
    int speedLevelLeft, speedLevelRight = 0; // for movement with calculateSpeed
    int speedLevelRightTrue, speedLevelLeftTrue = 0;
    float slope = 1.0/(START_RAMP_VEL-STOP_RAMP_VEL);
    int linearizationOfPWM(int val, int dir);
    void stopCar();
    void calculateSpeedFromXY();
    void updatePWM();
    void emergencyStop();
    void setDrivePins();
    bool debugOutputs = 0;

signals:
    void sendMessage(QString message);
};

#endif // MOTORSTEERING_H
