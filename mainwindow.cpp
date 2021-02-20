#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->LogsMonitor->append("Started car controller");
    errorPigpio = this->connectToPigpio(NULL, NULL);
    conf = new carConfig;
    data = new carData;
    if (errorPigpio > 0) {
        this->addToLogs("Initialization completed!");
        rearSonar = new Sonar(this->pigpioHost, conf->rearSonarTriggerPin, conf->rearSonarEchoPin, QString::fromLatin1("rear"), this);
        frontSonar = new Sonar(this->pigpioHost, conf->frontSonarTriggerPin, conf->frontSonarEchoPin, QString::fromLatin1("front"), this);
        if (frontSonar && rearSonar) {
            this->addToLogs("Created sonars!");
            timer1 = new QTimer(this);
            timer1->setInterval(100);
            timer1->start();
            connect(timer1, SIGNAL(timeout()), this, SLOT(triggerSonars()));
        }
        frontLeftIFR = new IFRSensor(this->pigpioHost, conf->frontLeftIRF, 0, this);


    }
    connect(ui->joypad, &JoyPad::xChanged, this, [this](float x){
        this->data->xAxis = int(x*255);
        ui->xAxisValue->display(this->data->xAxis);
    });

    connect(ui->joypad, &JoyPad::yChanged, this, [this](float y){
        this->data->yAxis = int(y*255);
        ui->yAxisValue->display(this->data->yAxis);
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

void MainWindow::getFrontDistance(float value)
{
    this->data->frontRadar = value;
    ui->frontSonarLCD->display(value);

}

void MainWindow::getRearDistance(float value)
{
    this->data->rearRadar = value;
    ui->rearSonarLCD->display(value);
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
}

void MainWindow::collisionDetected(int side, bool level)
{
    level = !level; // change logic as 5V is when no detection
    if (side == 0){
        this->data->frontLeftIFR = level;
        if (level) ui->frontLeftIFR->setChecked(true);
        else ui->frontLeftIFR->setChecked(false);
    } else if (side == 1) {
        this->data->frontRightIFR = level;
    } else if (side == 2) {
        this->data->rearLeftIFR = level;
    } else if (side == 3){
        this->data->rearRightIFR = level;
    } else {
        this->addToLogs("Error at collision detected. Value: " + QString::number(side));
    }
}


