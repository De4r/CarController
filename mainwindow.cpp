#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QDebug>
#include "common.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->LogsMonitor->append("Started car controller");
    errorPigpio = this->connectToPigpio(NULL, NULL);

    if (errorPigpio > 0) {
        this->addToLogs("Initialization completed!");
        rearSonar = new Sonar(this->pigpioHost, conf.rearSonarTriggerPin, conf.rearSonarEchoPin, QString::fromLatin1("rear"), this);
        frontSonar = new Sonar(this->pigpioHost, conf.frontSonarTriggerPin, conf.frontSonarEchoPin, QString::fromLatin1("front"), this);
        if (frontSonar && rearSonar) {
            this->addToLogs("Created sonars!");
            timer1 = new QTimer(this);
            timer1->setInterval(100);
            timer1->start();
            connect(timer1, SIGNAL(timeout()), this, SLOT(triggerSonars()));
            connect(timer1, SIGNAL(timeout()), this, SLOT(updateMotorSteering()));
        }
        frontLeftIFR = new IFRSensor(this->pigpioHost, conf.frontLeftIRF, 0, this);
        frontRightIFR = new IFRSensor(this->pigpioHost, conf.frontRightIRF, 1, this);
        rearLeftIFR = new IFRSensor(this->pigpioHost, conf.rearLeftIRF, 2, this);
        rearRightIFR = new IFRSensor(this->pigpioHost, conf.rearRightIFR, 3, this);

        motorStering = new MotorSteering(this->pigpioHost, conf.pwmLeft, conf.pwmRight, conf.inLeft1, conf.inLeft2, conf.inRight3, conf.inRight4, this);

        timer2 = new QTimer(this);
        timer2->setInterval(200);
        timer2->start();
        connect(timer2, SIGNAL(timeout()), this, SLOT(updateLCDs()));
    }
    connect(ui->joypad, &JoyPad::xChanged, this, [this](float x){
        cardata.xAxis = int(x*1000);
        //updateMotorSteering();
    });

    connect(ui->joypad, &JoyPad::yChanged, this, [this](float y){
        cardata.yAxis = int(y*1000);
        //updateMotorSteering();
    });
}

MainWindow::~MainWindow()
{
    this->disconnectFromPigpio();
    delete ui;
}

void MainWindow::addToLogs(QString message)
{
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    ui->LogsMonitor->append(currentDateTime + "\t" + message);
}

void MainWindow::updateLCDs()
{
    ui->xAxisValue->display(cardata.xAxis);
    ui->yAxisValue->display(cardata.yAxis);
    ui->leftPWMValue->display(this->motorStering->speedLevelLeft);
    ui->rightPWMValue->display(this->motorStering->speedLevelRight);
    ui->frontSonarLCD->display(cardata.frontRadar);
    ui->rearSonarLCD->display(cardata.rearRadar);

}

void MainWindow::updateMotorSteering()
{
    this->motorStering->calculateSpeedFromXY();
}


void MainWindow::on_ClearLogsMonitor_clicked()
{
    ui->LogsMonitor->clear();
}

int MainWindow::connectToPigpio(char *addrStr, char *portStr)
{
    if (this->pigpioHost < 0) {
        this->pigpioHost = pigpio_start(addrStr, portStr);
        if (pigpioHost < 0) {
            this->addToLogs("Failed to create pigpio deamon host. Make sure to run pigpiod before runing program!");
            return 0;
        } else {
            this->addToLogs("Started pigpio deamon host at " + QString::number(pigpioHost));
        }
        return 1;
    } else {
        return this->pigpioHost;
    }
}

void MainWindow::disconnectFromPigpio()
{
    pigpio_stop(this->pigpioHost);
    this->addToLogs("Terminated connection to pigpio!");
    this->pigpioHost = -1;
}


void MainWindow::on_PigpioConnectionManager_clicked()
{
    if (this->pigpioHost >= 0) this->disconnectFromPigpio();
    else this->connectToPigpio(NULL, NULL);
}

void MainWindow::triggerSonars()
{
    if (rearSonar) {
        rearSonar->trigger();
    }
    if (frontSonar) {
        frontSonar->trigger();
    }
    if (firstResetIFR){
        emit ui->resetIFRs->clicked();
        firstResetIFR = false;
    }
}

void MainWindow::collisionDetected()
{

    const char _styleGreen [] = "{background-color: rgb(62, 251, 42); border-radius: 15px;}";
    const char _styleRed [] = "{background-color: rgb(239, 41, 41); border-radius: 15px;}";
    //qDebug() << "Level: " << level << "Side: " << side;
    if (cardata.frontLeftIFR) ui->frontLeftIFR->setStyleSheet(QString::fromUtf8("#frontLeftIFR ") + QString::fromUtf8(_styleRed));
    else ui->frontLeftIFR->setStyleSheet(QString::fromUtf8("#frontLeftIFR ") + QString::fromUtf8(_styleGreen));

    if (cardata.frontRightIFR) ui->frontRightIFR->setStyleSheet(QString::fromUtf8("#frontRightIFR ") + QString::fromUtf8(_styleRed));
    else ui->frontRightIFR->setStyleSheet(QString::fromUtf8("#frontRightIFR ") + QString::fromUtf8(_styleGreen));

    if (cardata.rearLeftIFR) ui->rearLeftIFR->setStyleSheet(QString::fromUtf8("#rearLeftIFR ") + QString::fromUtf8(_styleRed));
    else ui->rearLeftIFR->setStyleSheet(QString::fromUtf8("#rearLeftIFR ") + QString::fromUtf8(_styleGreen));

    if (cardata.rearRightIFR) ui->rearRightIFR->setStyleSheet(QString::fromUtf8("#rearRightIFR ") + QString::fromUtf8(_styleRed));
    else ui->rearRightIFR->setStyleSheet(QString::fromUtf8("#rearRightIFR ") + QString::fromUtf8(_styleGreen));
}



void MainWindow::on_stopCarButton_clicked()
{
    this->motorStering->stopCar();
}


void MainWindow::on_debugOutputs_clicked(bool checked)
{
    this->debugOutputs = checked;
    rearSonar->debugOutputs = frontSonar->debugOutputs = checked;
    frontLeftIFR->debugOutputs = frontRightIFR->debugOutputs = rearLeftIFR->debugOutputs = rearRightIFR->debugOutputs = checked;
    motorStering->debugOutputs = checked;
}

void MainWindow::on_resetIFRs_clicked()
{
    cardata.frontLeftIFR = 0;
    cardata.frontRightIFR = 0;
    cardata.rearLeftIFR = 0;
    cardata.rearRightIFR = 0;
    addToLogs("Zresetowano stany czujnikow IFR");
}
