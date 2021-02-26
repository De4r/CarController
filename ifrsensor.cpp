#include "ifrsensor.h"
#include <QString>
#include <QDebug>
#include "common.h"

IFRSensor::IFRSensor(int pi, char pin, int side, QObject *parent) : QObject(parent)
{
    int error = 0;
    if (parent){
        this->pi = pi;
        this->sensorPin = pin;
        this->side = side;
        this->_pth = NULL;

        connect(this, SIGNAL(sendMessage(QString)), parent, SLOT(printMessage(QString)));
        connect(this, SIGNAL(collisionDetectedSignal()), parent, SLOT(collisionDetected()));
        set_mode(pi, sensorPin, PI_INPUT);
        set_pull_up_down(pi, sensorPin, PI_PUD_UP);

        this->_cb_id_rising = callback_ex(pi, pin, RISING_EDGE, cbEx, this);
        this->_cb_id_falling = callback_ex(pi, pin, FALLING_EDGE, cbEx, this);
        set_glitch_filter(pi, pin, this->_glitch_ms);
    }
    else {
        error = -1;
    }

    if (!error) {
        emit sendMessage("Inicjalizacja IFR: " + QString::number(side) + " cb_id_: " + QString::number(_cb_id_rising) + " cb_id_: " + QString::number(_cb_id_falling));
    } else {
        if (debugOutputs) qDebug() << "Blad inicjalizacji IRF: " << QString::number(side) << " Error: " << QString::number(error);
    }
}

IFRSensor::~IFRSensor()
{
    this->cbCancel();
}

void IFRSensor::cbCancel()
{
    callback_cancel(_cb_id_rising);
    callback_cancel(_cb_id_falling);
    if (debugOutputs) qDebug() << "Kasowanie callbackow. IFR cb id: " << QString::number(_cb_id_rising) << " i " << QString::number(_cb_id_rising);
}

void IFRSensor::cb(int pi, unsigned gpio, unsigned level, unsigned int tick)
{
    if (debugOutputs) qDebug() << "IFR: " << this->side << "level: " << level;
    if (gpio == sensorPin){
        emit collisionDetectedSignal();
        level = !level;

        if (this->side == 0) cardata.frontLeftIFR = level;
        if (this->side == 1) cardata.frontRightIFR = level;
        if (this->side == 2) cardata.rearLeftIFR = level;
        if (this->side == 3) cardata.rearRightIFR = level;


    }
}

void IFRSensor::cbEx(int pi, unsigned gpio, unsigned level, unsigned int tick, void *user)
{
    IFRSensor *mySelf = (IFRSensor *) user;
    mySelf->cb(pi, gpio, level, tick);
}
