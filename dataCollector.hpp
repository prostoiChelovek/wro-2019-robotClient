//
// Created by prostoichelovek on 08.06.19.
//

#ifndef VIDEOTRANS_DATACOLLECTION_HPP
#define VIDEOTRANS_DATACOLLECTION_HPP

#include <string>
#include <map>

#include "utils.hpp"

#ifdef IS_PI
#include "arduino.hpp"
#endif

#include "modules/speech/Recognition.h"

using namespace std;

class DataCollector {
public:
    SpeechRecognition::Recognition &speechRecognizer;

#ifdef IS_PI
    Arduino &arduino;
#endif

    explicit DataCollector(SpeechRecognition::Recognition &speechRecognizer)
            : speechRecognizer(speechRecognizer) {
        SpeechRecognition::CallbackFn cb = [&](string str) {
            speech = str;
        };
        speechRecognizer.onRecognize = cb;
        speechRecognizer.start();
    }

#ifdef IS_PI
    void set_arduino(Arduino &ardu) {
        arduino = ardu;
    }
#endif

    map<string, string> collectData() {
        map<string, string> res;
        if (!speech.empty()) {
            log(INFO, "Speech recognized:", speech);
            res["speech"] = speech;
            speech.clear();
        }

#ifdef IS_PI
        res["dist"] = arduino.get_data(0);
        res["x"] = arduino.get_data(1);
        res["y"] = arduino.get_data(2);
        res["z"] = arduino.get_data(3);
#endif
        return res;
    }

private:
    string speech;
};


string serealizeData(map<string, string> &data) {
    string res;
    for (auto &d : data)
        res += d.first + ":" + d.second + ";";
    return res;
}

#endif //VIDEOTRANS_DATACOLLECTION_HPP
