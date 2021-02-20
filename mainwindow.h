#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <pigpiod_if2.h>
#include <stdio.h>
#include <QString>
#include "QObject"
#include <QDateTime>
#include <QTimer>
#include <QKeyEvent>
#include <QEvent>
#include "sonar.h"
#include "ifrsensor.h"

struct carConfig {
    unsigned char rearSonarTriggerPin = 26;
    unsigned char frontSonarTriggerPin = 13;
    unsigned char rearSonarEchoPin = 24;
    unsigned char frontSonarEchoPin = 5;

    unsigned char rearLeftIRF = 16;
    unsigned char rearRightIFR = 9;
    unsigned char frontLeftIRF = 23;
    unsigned char frontRightIRF = 6;

    unsigned char leftEncoder = 25;
    unsigned char rightEncoder = 10;

};

struct carData {
    int xAxis = 0;
    int yAxis = 0;
    float frontRadar = 0.0;
    float rearRadar = 0.0;
    bool frontLeftIFR = 0;
    bool frontRightIFR = 0;
    bool rearLeftIFR = 0;
    bool rearRightIFR = 0;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addToLogs(QString message);
    Sonar* rearSonar;
    Sonar* frontSonar;
    IFRSensor* frontLeftIFR;
    QTimer* timer1;
    float rearDist, frontDist;
    carConfig* conf;
    carData* data;

private slots:
    void on_ClearLogsMonitor_clicked();
    void on_PigpioConnectionManager_clicked();


public slots:
    void triggerSonars();
    void collisionDetected(int, bool);
    void getFrontDistance(float value);
    void getRearDistance(float value);
    void printMessage(QString message) { this->addToLogs(message);}

private:
    Ui::MainWindow *ui;
    int connectToPigpio(char *addrStr, char *portStr);
    void disconnectFromPigpio();
    int pigpioHost = -1;
    int errorPigpio = 0;
};
#endif // MAINWINDOW_H
