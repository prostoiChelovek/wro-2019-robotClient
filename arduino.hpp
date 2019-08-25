//
// Created by prostoichelovek on 25.08.19.
//

#ifndef VIDEOTRANS_ARDUINO_HPP
#define VIDEOTRANS_ARDUINO_HPP

#ifdef IS_PI

#include <mutex>
#include <thread>

#include "utils.hpp"

#include "modules/Pi2c/pi2c.h"

using namespace std;

class Arduino {
public:
    int address = 8;

    const int cmd_size = 7;
    const int data_size = 4;

    // 0 - left hand
    // 1 - right hand
    // 2 - head
    // 3 - left motor speed
    // 4 - right motor speed
    // 5 - left motor direction
    // 6 - right motor direction
    char cmd[cmd_size] = {};

    // 0 - distance(cm)
    // 1, 2, 3 - x, y, z from adxl
    char data[data_size] = {};

    bool run_loop = true;

    Arduino(int address = 8) : arduino(Pi2c(address)) {
        loop();
    }

    ~Arduino() {
        run_loop = false;
    }

    void send_cmd() {
        lock_guard<mutex> lg(cmd_m);
        arduino.i2cWrite(cmd, cmd_size);
    }

    void read_data() {
        lock_guard<mutex> lg(data_m);
        arduino.i2cRead(data, data_size);
    }

    char get_data(int i) {
        lock_guard<mutex> lg(data_m);
        if(i >= data_size)
            return -1;
        return data[i];
    }

    void set_cmd(int i, char val) {
        lock_guard<mutex> lg(cmd_m);
        if(i >= cmd_size)
            return;
        cmd[i] = val;
    }

    void loop() {
        thread([&](){
            while(run_loop) {
                read_data();
                this_thread::sleep_for(chrono::milliseconds(2));
                send_cmd();
                this_thread::sleep_for(chrono::milliseconds(2));
            }
        }).detach();
    }

private:
    Pi2c arduino;

    mutex cmd_m;
    mutex data_m;

};

#endif //IS_PI

#endif //VIDEOTRANS_ARDUINO_HPP
