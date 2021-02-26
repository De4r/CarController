#ifndef COMMON_H
#define COMMON_H

struct carConfig {
    unsigned char rearSonarTriggerPin, frontSonarTriggerPin, rearSonarEchoPin, frontSonarEchoPin;

    unsigned char rearLeftIRF, rearRightIFR, frontRightIRF, frontLeftIRF;

    unsigned char leftEncoder, rightEncoder;

    unsigned char pwmLeft, pwmRight, inLeft1, inLeft2, inRight3, inRight4;


};

struct carData {
    int xAxis, yAxis, frontRadar, rearRadar;

    bool frontLeftIFR, frontRightIFR, rearLeftIFR, rearRightIFR;

};

extern struct carConfig conf;
extern struct carData cardata;


#endif // COMMON_H
