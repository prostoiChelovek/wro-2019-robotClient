//
// Created by prostoichelovek on 08.06.19.
//

#ifndef VIDEOTRANS_DATACOLLECTION_HPP
#define VIDEOTRANS_DATACOLLECTION_HPP

#include <string>
#include <map>

#include "utils.hpp"

#include "modules/speech/Recognition.h"

using namespace std;

class DataCollector {
public:
    bool should_recognizeSpeech = true;

    DataCollector(SpeechRecognition::Recognition &speechRecognizer) : speechRecognizer(speechRecognizer) {
        SpeechRecognition::CallbackFn cb = [&](string str) {
            speech = str;
        };
        speechRecognizer.onRecognize = cb;
        speechRecognizer.start();
    }

    map<string, string> collectData() {
        map<string, string> res;
        if (!speech.empty() && should_recognizeSpeech) {
            log(INFO, "Speech recognized:", speech);
            res["speech"] = speech;
            speech.clear();
        }
        if (!speech.empty() && !should_recognizeSpeech)
            speech.clear();

        res["temp"] = to_string(30);
        return res;
    }

private:
    SpeechRecognition::Recognition &speechRecognizer;
    string speech;
};


string serealizeData(map<string, string> &data) {
    string res;
    for (auto &d : data)
        res += d.first + ":" + d.second + ";";
    return res;
}

#endif //VIDEOTRANS_DATACOLLECTION_HPP
