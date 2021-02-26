#ifndef IFRSENSOR_H
#define IFRSENSOR_H

#include <QObject>
#include <QString>
#include <pigpiod_if2.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

typedef void (*sonarCB_t)(float);

class IFRSensor : public QObject
{
    Q_OBJECT
public:
    int pi;
    unsigned char sensorPin;
    int side; // side, 0 for front Left, 1 for front Right, 2 for rear Left, 3 - rear right
    bool detection = 0;
    pthread_t *_pth;
    int _cb_id_rising, _cb_id_falling;
    int _glitch_ms = 300;
    IFRSensor(int pi, char pin, int side, QObject *parent);
    ~IFRSensor();
    void cbCancel(void);
    void cb(int pi, unsigned gpio, unsigned level, __UINT32_TYPE__ tick);
    static void cbEx(int pi, unsigned gpio, unsigned level, __UINT32_TYPE__ tick, void *user);
    bool debugOutputs = 0;

signals:
    void sendMessage(QString message);
    void collisionDetectedSignal();

};


#endif // IFRSENSOR_H
