//
// Created by prostoichelovek on 08.06.19.
//

#ifndef VIDEOTRANS_DATACOLLECTION_HPP
#define VIDEOTRANS_DATACOLLECTION_HPP

#include <string>
#include <map>

using namespace std;

map<string, string> collectData() {
    map<string, string> res;
    res["temp"] = to_string(30);
    return res;
}

string serealizeData(map<string, string> &data) {
    string res;
    for (auto &d : data)
        res += d.first + ":" + d.second + ";";
    return res;
}

#endif //VIDEOTRANS_DATACOLLECTION_HPP
