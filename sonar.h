#ifndef SONAR_H
#define SONAR_H
#include <pigpiod_if2.h>
#include <stdio.h>
#include <stdlib.h>
#include <sma.h>
#include <QObject>

typedef void (*sonarCB_t)(float);

class Sonar : public QObject
{
    Q_OBJECT
public:
    int pi;
    unsigned char trig, echo;
    int status; /* Good, bad range, or timeout */
    int round_trip_micros; /* micros taken to travel to the object and back */
    float range_cms; /* range in centimetres to the object */
    float min_cms, max_cms;
    int _cb_id_trig, _cb_id_echo;
    QString frontOrRear;
    pthread_t *_pth;
    SMA<int(10)> filter;
    int _in_code;
    int _ready, _new_reading;
    int _got_echo, _got_trig;
    __UINT32_TYPE__ _echo_tick;
    __UINT32_TYPE__ _trig_tick;


    void trigger();
    Sonar(int pi, char trigPin, char echoPin, QString rearOrFront, QObject *parent=NULL);
    ~Sonar();
    void cbCancel(void);
    void cb(int pi, unsigned gpio, unsigned level, __UINT32_TYPE__ tick);
    static void cbEx(int pi, unsigned gpio, unsigned level, __UINT32_TYPE__ tick, void *user);

private:
    int m_parent = 0;

signals:
    void sendValue(float value);
    void sendMessage(QString message);
};

#endif // SONAR_H
