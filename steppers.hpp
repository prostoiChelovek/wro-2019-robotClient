//
// Created by prostoichelovek on 16.06.19.
//

#ifndef VIDEOTRANS_STEPPERS_H
#define VIDEOTRANS_STEPPERS_H

#ifdef IS_PI

#include <wiringPi.h>
#include <softTone.h>

#include <thread>

using namespace std;

class steppers {
public:
    int direction1 = -1, direction2 = -1;

    steppers(int step1, int dir1, int step2, int dir2)
            : step1_pin(step1), dir1_pin(dir1), step2_pin(step2), dir2_pin(dir2) {
        wiringPiSetup () ;
        pinMode(dir1, OUTPUT);
        pinMode(dir2, OUTPUT);
        digitalWrite(dir1, HIGH);
        digitalWrite(dir2, LOW);
        softToneCreate(step1);
        softToneCreate(step2);

        ctlTh = thread([&](){
            while(true) {
                loop();
            }
        });
    }

    void setDir(int dir1, int dir2) {
        direction1 = dir1;
        direction2 = dir2;
    }

private:
    int step1_pin;
    int dir1_pin;
    int step2_pin;
    int dir2_pin;

    thread ctlTh;

    void loop() {
        if(direction1 == -1)
            softToneWrite(step1_pin, 0);
        else
            softToneWrite(step1_pin, 5000);
        if(direction2 == -1)
            softToneWrite(step2_pin, 0);
        else
            softToneWrite(step2_pin, 5000);

        if(direction1 == 1)
            digitalWrite(dir1_pin, HIGH);
        else
            digitalWrite(dir1_pin, LOW);
        if(direction2 == 1)
            digitalWrite(dir2_pin, HIGH);
        else
            digitalWrite(dir2_pin, LOW);
    }
};


#endif //VIDEOTRANS_STEPPERS_H

#endif // IS_PI