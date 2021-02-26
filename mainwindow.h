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
#include "motorsteering.h"
#include "common.h"


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
    IFRSensor* frontRightIFR;
    IFRSensor* rearLeftIFR;
    IFRSensor* rearRightIFR;
    QTimer* timer1;
    QTimer* timer2;
    MotorSteering* motorStering;
    bool debugOutputs = 0;
    bool firstResetIFR = 1;


private slots:
    void on_ClearLogsMonitor_clicked();
    void on_PigpioConnectionManager_clicked();
    void on_stopCarButton_clicked();
    void on_debugOutputs_clicked(bool checked);
    void on_resetIFRs_clicked();

public slots:
    void updateLCDs();
    void triggerSonars();
    void collisionDetected();
    void printMessage(QString message) { this->addToLogs(message);}
    void updateMotorSteering();

private:
    Ui::MainWindow *ui;
    int connectToPigpio(char *addrStr, char *portStr);
    void disconnectFromPigpio();
    int pigpioHost = -1;
    int errorPigpio = 0;
};
#endif // MAINWINDOW_H
