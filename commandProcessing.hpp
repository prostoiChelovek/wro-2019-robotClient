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
#include <wiringPi.h>
#include <softTone.h>

#define PIN 3
#define PIN2 1
#define DIR_PIN 4
#define DIR_PIN2 0

#include "steppers.h"
#endif


class CommandProcessor {
public:
    DataCollector &dataColl;

#ifdef IS_PI
    steppers stps = steppers(PIN, DIR_PIN, PIN2, DIR_PIN2);
#endif

    CommandProcessor(Speaker &speaker, DataCollector &comProc)
            : speaker(speaker), dataColl(comProc) {

    }

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
                    runSteppers(stoi(val));
                }
                ifKeyIs("say") {
                    log(INFO, "Speaking:", val);
                    speaker.say(val);
                }
                ifKeyIs("head") {
//                    log(INFO, "Rotating head on", val);
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

    void runSteppers(int dir) {
#ifdef IS_PI
        if(dir == 0)
          stps.setDir(-1, -1);
        if(dir == 1)
            stps.setDir(1, 1);
        if(dir == 2)
            stps.setDir(0, 0);
        if(dir == 4)
            stps.setDir(1, 0);
        if(dir == 3)
            stps.setDir(0, 1);
#endif
    }
};

#endif //VIDEOTRANS_COMMANDPROCESSING_HPP
