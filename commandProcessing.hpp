//
// Created by prostoichelovek on 06.06.19.
//

#ifndef VIDEOTRANS_COMMANDPROCESSING_HPP
#define VIDEOTRANS_COMMANDPROCESSING_HPP

#include "utils.hpp"

#include "dataCollector.hpp"

#include "modules/speech/speaking.h"
#include "modules/speech/Recognition.h"

class CommandProcessor {
public:
    DataCollector &dataColl;

    CommandProcessor(RHSpeaker &speaker, DataCollector &comProc)
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
                ifKeyIs("fwd") {
                    //log(INFO, "Moving forward on", val, "cm");
                }
                ifKeyIs("say") {
                    log(INFO, "Speaking:", val);
                    dataColl.should_recognizeSpeech = false;
                    thread([val, this]() {
                        try {
                            speaker.say(val);
                            this_thread::sleep_for(chrono::seconds(2));
                            dataColl.should_recognizeSpeech = true;
                        } catch (exception &e) {
                            log(ERROR, "Cannot speak '", val, "':", e.what());
                            this_thread::sleep_for(chrono::seconds(2));
                            dataColl.should_recognizeSpeech = true;
                        }
                    }).detach();
                }
                ifKeyIs("head") {
                    //log(INFO, "Rotating head on", val);
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
    RHSpeaker &speaker;
};

#endif //VIDEOTRANS_COMMANDPROCESSING_HPP
