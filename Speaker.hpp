//
// Created by prostoichelovek on 18.06.19.
//

#ifndef VIDEOTRANS_SPEAKER_HPP
#define VIDEOTRANS_SPEAKER_HPP

#include <string>
#include <map>
#include <tuple>
#include <queue>
#include <thread>

#include "modules/speech/speaking.h"

#include "utils.hpp"

using namespace std;

// args
using SpeakerCallbackFn = function<void(string)>;
// text, callback name, args
using SpeakerToSayType = tuple<string, string, string>;

class Speaker {
public:
    RHSpeaker &speaker;

    map<string, SpeakerCallbackFn> callbacks;

    explicit Speaker(RHSpeaker &speaker) : speaker(speaker) {
        thread(&Speaker::speakerTh, this).detach();
    }

    void say(const string &text) {
        parseText(text);
    }

private:
    queue<SpeakerToSayType> toSay;

    void callCb(const string &name, const string &args) {
        if (callbacks.count(name) == 0)
            return;
        SpeakerCallbackFn &cb = callbacks[name];
        thread(cb, args).detach();
    }

    void parseText(const string &text) {
        string temp = text;
        size_t startPos = 0, endPos = 0;
        while (startPos != string::npos && endPos != string::npos) {
            startPos = temp.find('<');
            endPos = temp.find('>');
            if (startPos != string::npos && endPos != string::npos) {
                string speechStr = temp.substr(0, startPos);
                string cmdStr = temp.substr(startPos + 1, endPos - startPos - 1);

                if (!speechStr.empty()) {
                    if (speechStr[0] == ' ')
                        speechStr.erase(0, 1);
                }
                if (speechStr[speechStr.size() - 1] == ' ' && !speechStr.empty())
                    speechStr.pop_back();

                vector<string> cmd = split(cmdStr, ":");
                if (cmd.size() < 2) {
                    cmd[0] = cmd[1] = "";
                }
                toSay.emplace(speechStr, cmd[0], cmd[1]);

                temp.erase(0, endPos + 1);
            }
        }
        if (!temp.empty())
            toSay.emplace(temp, "", "");
    }

    void speakerTh() {
        while (true) {
            if (toSay.empty())
                continue;

            SpeakerToSayType &sayNow = toSay.front();
            string text, cmdName, args;
            tie(text, cmdName, args) = sayNow;

            if (!text.empty()) {
                callCb("speechStart", text);
                try {
                    speaker.say(text);
                } catch (exception &e) {
                    log(ERROR, "Cannot speak '", text, "':", e.what());
                }
                callCb("speechEnd", text);
            }
            if (!cmdName.empty()) {
                callCb("cmd", cmdName + ":" + args + ";");
                callCb(cmdName, args);
                if (callbacks.count("cmd") == 0 && callbacks.count("cmd") == 0)
                    log(WARNING, "callback on speech command", cmdName, "not exists");
            }

            toSay.pop();
        }
    }

};


#endif //VIDEOTRANS_SPEAKER_HPP
