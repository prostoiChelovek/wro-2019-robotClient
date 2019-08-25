//
// Created by prostoichelovek on 06.06.19.
//

#ifndef VIDEOTRANS_COMMANDPROCESSING_HPP
#define VIDEOTRANS_COMMANDPROCESSING_HPP

#include "utils.hpp"

#include "dataCollector.hpp"

#include "modules/speech/speaking.h"
#include "modules/speech/Recognition.h"

#include "Speaker.hpp"

#ifdef IS_PI
#include "arduino.hpp"
#endif

class CommandProcessor {
public:
    DataCollector &dataColl;

#ifdef IS_PI
    Arduino &arduino;
#endif

    int default_motor_speed = 200;

#ifdef IS_PI
    CommandProcessor(Speaker &speaker, DataCollector &comProc, Arduino &arduino)
            : speaker(speaker), dataColl(comProc), arduino(arduino) {}
#else
    CommandProcessor(Speaker &speaker, DataCollector &comProc)
            : speaker(speaker), dataColl(comProc) {}

#endif

    map<string, string> parseCmd(const string &cmd) {
        map<string, string> res;
        for (const string &part : split(cmd, ";")) {
            vector<string> data = split(part, ":");
            if (data.empty())
                continue;
            if (data.size() != 2) {
                log(ERROR, "Incorrect argument in command: '", part, "'");
                continue;
            }
            res[data[0]] = data[1];
        }
        return res;
    }

    void processCmd(map<string, string> cmd) {
        try {
            for (auto &cm : cmd) {
                string key = cm.first;
                string val = cm.second;
                ifKeyIs("move") {

                }
                ifKeyIs("say") {
                    log(INFO, "Speaking:", val);
                    speaker.say(val);
                }
                ifKeyIs("head") {
                    head(stoi(val));
                }
                ifKeyIs("right") {
                    right_hand(stoi(val));
                }
                ifKeyIs("left") {
                    left_hand(stoi(val));
                }
                ifKeyIs("command") {
                    log(INFO, "Command:", val);
                    if (val == "dance") {
                        thread([&]() {
                            command_dance();
                        }).detach();
                    }
                }
            }
        } catch (exception &e) {
            log(ERROR, "Cannot process command:", e.what());
        }
    }

    void processCmd(const string &cmd) {
        map<string, string> parsed = parseCmd(cmd);
        processCmd(parsed);
    }

private:
    Speaker &speaker;

    void left_hand(int val) {
#ifdef IS_PI
        arduino.set_cmd(0, val);
#endif
    }

    void right_hand(int val) {
#ifdef IS_PI
        arduino.set_cmd(1, val);
#endif
    }

    void head(int val) {
#ifdef IS_PI
        arduino.set_cmd(2, val);
#endif
    }

    void left_motor(int speed, int direction) {
#ifdef IS_PI
        arduino.set_cmd(3, speed);
        arduino.set_cmd(5, direction);
#endif
    }

    void right_motor(int speed, int direction) {
#ifdef IS_PI
        arduino.set_cmd(4, speed);
        arduino.set_cmd(6, direction);
#endif
    }

    void command_dance() {
        for (int n = 0; n < 5; n++) {
            left_motor(default_motor_speed, 1);
            right_motor(default_motor_speed, 0);

            std::future<void> left_hand_f = std::async(std::launch::async, [&] {
                for (int i = 10; i < 170; i++) {
                    left_hand(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            std::future<void> right_hand_f = std::async(std::launch::async, [&] {
                for (int i = 170; i > 10; i--) {
                    right_hand(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            std::future<void> head_f = std::async(std::launch::async, [&] {
                for (int i = 10; i < 170; i++) {
                    head(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            left_hand_f.wait();
            right_hand_f.wait();
            head_f.wait();

            left_motor(default_motor_speed, 0);
            right_motor(default_motor_speed, 1);

            left_hand_f = std::async(std::launch::async, [&] {
                for (int i = 170; i > 10; i--) {
                    left_hand(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            right_hand_f = std::async(std::launch::async, [&] {
                for (int i = 10; i < 170; i++) {
                    right_hand(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            head_f = std::async(std::launch::async, [&] {
                for (int i = 170; i > 10; i--) {
                    head(i);
                    this_thread::sleep_for(chrono::milliseconds(5));
                }
            });
            left_hand_f.wait();
            right_hand_f.wait();
            head_f.wait();
        }

        left_motor(0, 0);
        right_motor(0, 0);
    }

};

#endif //VIDEOTRANS_COMMANDPROCESSING_HPP
