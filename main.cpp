#include <iostream>
#include <vector>
#include <map>

#include "opencv2/opencv.hpp"

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "utils.hpp"
#include "commandProcessing.hpp"

using namespace std;
using namespace cv;
using namespace boost::asio;
using ip::udp;

string serverAddress = "127.0.0.1";
int port = 1234;
int cameraNum = 2;
Size frameSize = Size(640, 480);

const int arduino_i2c_addr = 8;

enum Lang {
    RUSSIAN, ENGLISH
} lang = ENGLISH;

const string dataDir = "../data";
const string speechModule_data = "../modules/speech/data";
const string ru_model = speechModule_data + "/zero_ru_cont_8k_v3/zero_ru.cd_cont_4000";
const string ru_dict = dataDir + "/ru.dic";
const string ru_gram = dataDir + "/ru.jsgf";
const string ru_kws = dataDir + "/ru.kwlist";
const string ru_voice = "irina";
const string ru_listen_phrase = "Слушаю...";
const string ru_hello_phrase = "Привет, мир!";

const string en_model = speechModule_data + "/cmusphinx-en-us-5.2";
const string en_dict = dataDir + "/en.dic";
const string en_gram = dataDir + "/en.jsgf";
const string en_kws = dataDir + "/en.kwlist";
const string en_voice = "Slt";
const string en_listen_phrase = "Listening...";
const string en_hello_phrase = "Hello, world!";

bool show_ps_log = true;

vector<VideoCapture> openCaptures(int num) {
    vector<VideoCapture> res;
    for (int i = 0; i < num; i++) {
        VideoCapture cap;
        if (!cap.open(i))
            log(ERROR, "Cannot open video capture with index", i);
        else
            res.emplace_back(cap);
    }
    //res.emplace_back("camR.avi");
    //res.emplace_back("camL.avi");

    return res;
}

vector<Mat> captureFrames(vector<VideoCapture> caps) {
    vector<Mat> res;

    for (VideoCapture &cap : caps) {
        Mat frame;
        if (!cap.read(frame)) {
            log(ERROR, "Cannot read frame from camera");
            continue;
        }
        resize(frame, frame, frameSize);
        res.emplace_back(frame);
    }
    return res;
}

int main(int argc, char **argv) {
    if (argc > 1)
        serverAddress = argv[1];
    if (argc > 2)
        port = stoi(argv[2]);
    if (argc > 3)
        cameraNum = stoi(argv[3]);

    vector<VideoCapture> caps = openCaptures(cameraNum);
    if (caps.empty()) {
        log(ERROR, "Cannot open video capture");
        return EXIT_FAILURE;
    }
    if (caps.size() != cameraNum)
        log(WARNING, "Using", caps.size(), "cameras");

    io_service ios;
    ip::tcp::socket socket(ios);
    try {
        socket.connect(ip::tcp::endpoint(ip::address::from_string(serverAddress), port));
        log(INFO, "Connected to server", serverAddress, ":", port);
    } catch (exception &e) {
        log(ERROR, "Cannot connect to server:", e.what());
        return EXIT_FAILURE;
    }

    string current_model, current_dict, current_gram, current_kws;
    string current_voice, current_listen_phrase, current_hello_phrase;
    if (lang == RUSSIAN) {
        current_model = ru_model;
        current_dict = ru_dict;
        current_gram = ru_gram;
        current_kws = ru_kws;

        current_voice = ru_voice;
        current_listen_phrase = ru_listen_phrase;
        current_hello_phrase = ru_hello_phrase;
    } else if (lang == ENGLISH) {
        current_model = en_model;
        current_dict = en_dict;
        current_gram = en_gram;
        current_kws = en_kws;

        current_voice = en_voice;
        current_listen_phrase = en_listen_phrase;
        current_hello_phrase = en_hello_phrase;
    }

    SpeechRecognition::Recognition rec(current_model, current_dict,
                                       current_gram, current_kws, !show_ps_log);
    DataCollector dc(rec);

    RHSpeaker speaker_rh(current_voice);
    Speaker speaker(speaker_rh);

#ifdef IS_PI
    Arduino arduino(arduino_i2c_addr);

    dc.set_arduino(arduino);
    CommandProcessor cmdProc(speaker, dc, arduino);
#else
    CommandProcessor cmdProc(speaker, dc);
#endif

    speaker.callbacks["cmd"] = SpeakerCallbackFn([&](string args) {
        cmdProc.processCmd(args);
    });
    speaker.callbacks["speechStart"] = SpeakerCallbackFn([&](string args) {
        dc.speechRecognizer.pause = true;
    });
    speaker.callbacks["speechEnd"] = SpeakerCallbackFn([&](string args) {
        dc.speechRecognizer.pause = false;
    });

    dc.speechRecognizer.onKw = SpeechRecognition::CallbackFn([&](string str) {
        speaker.say(current_listen_phrase);
    });

    speaker.say(current_hello_phrase);

    //system("amixer -c 1 set \"Mic Boost\" 10%");

    Mat img, imgGray, all;
    // Send images to server and receive command
    while (true) {
        vector<Mat> frames = captureFrames(caps);

        hconcat(frames, all);
        cvtColor(all, imgGray, COLOR_BGR2GRAY);
        vector<uchar> compressed;
        imencode(".jpeg", all, compressed);

        boost::asio::streambuf buf;
        try {
            string sizeStr = to_string(compressed.size());
            sizeStr.resize(8);
            write(socket, buffer(sizeStr), transfer_all());

            socket.send(buffer(compressed));

            map<string, string> data = dc.collectData();
            string dataStr = serealizeData(data);
            socket.send(buffer(dataStr + "\n"));

            boost::asio::streambuf cmdBuf;
            read_until(socket, cmdBuf, "\n");
            string cmdStr = buffer_cast<const char *>(cmdBuf.data());
            cmdStr.pop_back();
            map<string, string> cmd = cmdProc.parseCmd(cmdStr);
            cmdProc.processCmd(cmd);
        } catch (boost::exception &err) {
            log(ERROR, "Cannot send image to server");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}