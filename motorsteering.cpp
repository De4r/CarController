#include "motorsteering.h"
#include <QObject>
#include <QString>
#include <pigpiod_if2.h>
#include <stdio.h>
#include <stdlib.h>
#include <sma.h>
#include <QDebug>
#include "common.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

float map_value(int val, int inStart, int inEnd, int outStart, int outEnd){
    float slope = (outEnd - outStart) / (inEnd - inStart);
    float output = outStart + slope*(val - inStart);
    return output;
}

int MotorSteering::linearizationOfPWM(int val, int dir){
    bool stop = 0;
    val = val*dir;
    if (val > MAX_CAR_INPUT_SPEED) val = MAX_CAR_INPUT_SPEED;
    float f_val = 0;
    if (val) f_val = MIN_SPEED + slope1*val;
    if (dir == 1) {
        if (cardata.frontRadar < this->START_RAMP_VEL){
            f_val = f_val*slope*(cardata.frontRadar-STOP_RAMP_VEL);
        }
        if (cardata.frontRadar < 10.0) {
            emergencyStop();
            stop = 1;
        }
    } else if (dir == -1) {
        if (cardata.rearRadar < this->START_RAMP_VEL){
            f_val = f_val*slope*(cardata.rearRadar-STOP_RAMP_VEL);
        }
        if (cardata.rearRadar < 10.0) {
            emergencyStop();
            stop = 1;
        }
    }
    if (f_val < 0 || stop) f_val = 0;

    return int(f_val);
}

MotorSteering::MotorSteering(int pi, unsigned char pwmLeft, unsigned char pwmRight, unsigned char inLeft1, unsigned char inLeft2, unsigned char inRight3, unsigned char inRight4, QObject* parent)  : QObject(parent)
{
    int error = 0;
    this->pi=pi;
    this->pwmLeft = pwmLeft;
    this->pwmRight = pwmRight;
    this->inLeft1 = inLeft1;
    this->inLeft2 = inLeft2;
    this->inRight3 = inRight3;
    this->inRight4 = inRight4;

    set_PWM_dutycycle(pi, pwmLeft, 0);
    set_PWM_dutycycle(pi, pwmRight, 0);

    set_mode(pi, inLeft2, PI_OUTPUT); //in 1
    set_mode(pi, inLeft1, PI_OUTPUT); //in 2

    set_mode(pi, inRight3, PI_OUTPUT); // in3
    set_mode(pi, inRight4, PI_OUTPUT); // in4
    connect(this, SIGNAL(sendMessage(QString)), parent, SLOT(printMessage(QString)));
    if (!error) {
        emit sendMessage("Inicjalizacja sterowania silnikami.");
    } else {
        qDebug() << "Blad inicjalizacja sterowania silnikami.";
    }
}

MotorSteering::~MotorSteering()
{
    gpio_write(pi, inLeft1, 0);
    gpio_write(pi, inLeft2, 0);

    gpio_write(pi, inRight3, 0);
    gpio_write(pi, inRight4, 0);
}

void MotorSteering::stopCar()
{
    speedLevelLeft = 0;
    speedLevelRight = 0;
    updatePWM();

    gpio_write(pi, inLeft1, 0);
    gpio_write(pi, inLeft2, 0);

    gpio_write(pi, inRight3, 0);
    gpio_write(pi, inRight4, 0);
}

void MotorSteering::calculateSpeedFromXY()
{
    if ((pow(cardata.xAxis, 2) + pow(cardata.yAxis, 2)) < DEAD_RANGE){
        cardata.xAxis = 0; cardata.yAxis = 0;
    }
    if (cardata.xAxis != 0 || cardata.yAxis !=0){
        if (sgn(cardata.yAxis)>=0){
            speedLevelLeftTrue = cardata.yAxis + cardata.xAxis/2;
            speedLevelRightTrue = cardata.yAxis - cardata.xAxis/2;
            if (speedLevelLeftTrue > MAX_CAR_INPUT_SPEED) speedLevelLeftTrue = MAX_CAR_INPUT_SPEED;
            if (speedLevelRightTrue > MAX_CAR_INPUT_SPEED) speedLevelRightTrue = MAX_CAR_INPUT_SPEED;
        }
        else if (sgn(cardata.yAxis) < 0) {
            speedLevelLeftTrue = cardata.yAxis - cardata.xAxis/2;
            speedLevelRightTrue = cardata.yAxis + cardata.xAxis/2;
            if (speedLevelLeftTrue < -MAX_CAR_INPUT_SPEED) speedLevelLeftTrue = -MAX_CAR_INPUT_SPEED;
            if (speedLevelRightTrue < -MAX_CAR_INPUT_SPEED) speedLevelRightTrue = -MAX_CAR_INPUT_SPEED;
        }
        // otrzymuje zakres -1000 do 100, nalezy teraz pamietac ze podac na pwm mozna 0-255
        if (cardata.frontLeftIFR &&  speedLevelLeftTrue > 0) speedLevelLeftTrue = 0;
        if (cardata.frontRightIFR &&  speedLevelRightTrue > 0) speedLevelRightTrue = 0;
        if (cardata.rearLeftIFR &&  (speedLevelLeftTrue < 0 || speedLevelRightTrue < 0)) speedLevelLeftTrue = 0;
        if (cardata.rearRightIFR &&  (speedLevelLeftTrue < 0 || speedLevelRightTrue < 0)) speedLevelRightTrue = 0;
        if (debugOutputs) qDebug() << "FL:" << cardata.frontLeftIFR << "FR:" << cardata.frontRightIFR << "RL:" << cardata.rearLeftIFR << "RR:" << cardata.rearRightIFR;
        setDrivePins();
    }
    else stopCar();
}

void MotorSteering::updatePWM()
{
    set_PWM_dutycycle(pi, pwmLeft, speedLevelLeft);
    set_PWM_dutycycle(pi, pwmRight, speedLevelRight);
}

void MotorSteering::emergencyStop()
{
    gpio_write(pi, inLeft1, 1);
    gpio_write(pi, inLeft2, 1);

    gpio_write(pi, inRight3, 1);
    gpio_write(pi, inRight4, 1);
    speedLevelLeft = 0;
    speedLevelRight = 0;
    updatePWM();
    time_sleep(0.2);

    stopCar();
}

void MotorSteering::setDrivePins()
{
    if (speedLevelLeftTrue <0){
        gpio_write(pi, inLeft1, 1);
        gpio_write(pi, inLeft2, 0);
        speedLevelLeft = linearizationOfPWM(speedLevelLeftTrue, -1);
    } else {
        gpio_write(pi, inLeft1, 0);
        gpio_write(pi, inLeft2, 1);
        speedLevelLeft = linearizationOfPWM(speedLevelLeftTrue, 1);
    }
    if (speedLevelRightTrue <0){
        gpio_write(pi, inRight3, 0);
        gpio_write(pi, inRight4, 1);
        speedLevelRight = linearizationOfPWM(speedLevelRightTrue, -1);
    } else {
        gpio_write(pi, inRight3, 1);
        gpio_write(pi, inRight4, 0);
        speedLevelRight = linearizationOfPWM(speedLevelRightTrue, 1);
    }
    updatePWM();
    if (debugOutputs) qDebug() << "LEFT: " << speedLevelLeftTrue << "PWM: " << speedLevelLeft << "RIGHT: " << speedLevelRightTrue << "PWM: " << speedLevelRight;

}
