//
// Created by prostoichelovek on 06.06.19.
//

#ifndef VIDEOTRANS_UTILS_HPP
#define VIDEOTRANS_UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

#define ifKeyIs(x) if(key == (x))

enum LogType {
    INFO, WARNING, ERROR
};

template<typename T>
void coutMany(T t) {
    std::cout << t << " " << std::endl;
}

template<typename T, typename... Args>
void coutMany(T t, Args... args) { // recursive variadic function
    std::cout << t << " " << std::flush;
    coutMany(args...);
}

template<typename T>
void cerrMany(T t) {
    std::cerr << t << " " << std::endl;
}

template<typename T, typename... Args>
void cerrMany(T t, Args... args) { // recursive variadic function
    std::cerr << t << " " << std::flush;
    cerrMany(args...);
}

template<typename T, typename... Args>
void log(int type, T t, Args... args) {
    switch (type) {
        case INFO:
            std::cout << "\033[;32mINFO:\033[0m ";
            coutMany(t, args...);
            break;
        case WARNING:
            std::cout << "\033[1;33mWARNING:\033[0m ";
            coutMany(t, args...);
            break;
        case ERROR:
            std::cerr << "\033[1;31mERROR:\033[0m ";
            cerrMany(t, args...);
            break;
    }
}

vector<string> split(const string &str, const string &delim) {
    vector<string> parts;
    size_t start, end = 0;
    while (end < str.size()) {
        start = end;
        while (start < str.size() && (delim.find(str[start]) != string::npos)) {
            start++;  // skip initial whitespace
        }
        end = start;
        while (end < str.size() && (delim.find(str[end]) == string::npos)) {
            end++; // skip to end of word
        }
        if (end - start != 0) {  // just ignore zero-length strings.
            parts.emplace_back(str, start, end - start);
        }
    }
    return parts;
}

#endif //VIDEOTRANS_UTILS_HPP
