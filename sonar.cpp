#include "sonar.h"
#include <pigpiod_if2.h>
#include <QDebug>


#define SRTE_GOOD      0
#define SRTE_BAD_RANGE 1
#define SRTE_TIMEOUT   2


void Sonar::trigger()
{
    int i = gpio_trigger(this->pi, this->trig, 11, 1);
    if (i<0) {
        emit sendMessage("Blad w " + this->frontOrRear + "sonar podczas trigger()!");
    }
}

Sonar::Sonar(int pi, char trigPin, char echoPin, QString rearOrFront, QObject *parent) : QObject(parent)
{
    int error = 0;
    if (parent){
        this->frontOrRear = rearOrFront;
        if (rearOrFront == "front"){
            connect(this, SIGNAL(sendValue(float)), parent, SLOT(getFrontDistance(float)));
        }
        else if (rearOrFront == "rear") {
            connect(this, SIGNAL(sendValue(float)), parent, SLOT(getRearDistance(float)));
        }
        else {
            error = 1;
        }
        connect(this, SIGNAL(sendMessage(QString)), parent, SLOT(printMessage(QString)));


        this->pi = pi;
        this->trig = trigPin;
        this->echo = echoPin;
        //this->_cb = callback;

        this->min_cms = 2;
        this->max_cms = 400.0;
        //static this->filter = SMA<10>;
        this->_pth = NULL;
        this->_got_trig = 0;
        this->_got_echo = 0;

        this->_ready = 0;
        this->_new_reading = 0;

        set_mode(pi, trig, PI_OUTPUT);
        set_mode(pi, echo, PI_INPUT);

        this->_cb_id_trig = callback_ex(pi, trig, FALLING_EDGE, cbEx, this);
        this->_cb_id_echo = callback_ex(pi, echo, EITHER_EDGE, cbEx, this);
    }
    else {
        error = -1;
    }

    if (!error) {
        emit sendMessage("Inicjalizacja " + frontOrRear + " sonar. TrigId: " + QString::number(_cb_id_trig) + " EchoId: " + QString::number(_cb_id_echo));
    } else {
        qDebug() <<"Blad inicjalizacji. Sonar: " << frontOrRear << " Error: " << QString::number(error);
    }

}



void Sonar::cbCancel()
{
    callback_cancel(_cb_id_echo);
    callback_cancel(_cb_id_trig);
    qDebug() << "Kasowanie callbackow. TrigId: " << QString::number(_cb_id_trig) << " EchoId: " << QString::number(_cb_id_echo);

}

void Sonar::cb(int pi, unsigned gpio, unsigned level, unsigned int tick)
{
    int round_trip_micros;
    float range_cms;
    int tick_diff;
    if (gpio == this->trig)
    {
        this->_got_trig = 1;
        this->_trig_tick = tick;
        this->_got_echo = 0;
    }
    else if (level == 1)
    {
        if (this->_got_trig)
        {
            /*
          Some devices send a rising edge shortly after the trigger.
          Those rising edges must be ignored.
          */

            tick_diff = tick - this->_trig_tick;

            if (tick_diff > 100)
            {
                this->_got_echo = 1;
                this->_echo_tick = tick;
            }
        }
    }
    else if (level == 0)
    {
        if (this->_got_echo)
        {
            this->_got_echo = 0;
            this->_got_trig = 0;

            round_trip_micros = tick - this->_echo_tick;
            range_cms = round_trip_micros * 0.017015;

            if ((this->min_cms <= range_cms) && (range_cms <= this->max_cms))
            {
                this->range_cms = range_cms;
                this->round_trip_micros = round_trip_micros;
                this->status = SRTE_GOOD;
            }
            else if (range_cms > this->max_cms)
            {
                this->range_cms = this->max_cms;
            }
            else if (range_cms < this->min_cms)
            {
                this->range_cms = this->min_cms;
            }

            this->_ready = 1;
            this->_new_reading = 1;
            int value = int(filter(int(range_cms)*10));
            emit sendValue(value/10.0);
        }
    }
}

void Sonar::cbEx(int pi, unsigned gpio, unsigned level, unsigned int tick, void *user)
{
    Sonar *mySelf = (Sonar *) user;
    mySelf->cb(pi, gpio, level, tick);
}

Sonar::~Sonar()
{
    this->cbCancel();
}

