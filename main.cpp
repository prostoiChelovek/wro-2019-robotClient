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

string serverAddress = "http://niks.duckdns.org:8080";
int port = 1234;
int cameraNum = 2;
Size frameSize = Size(640, 480);

vector<VideoCapture> openCaptures(int num) {
    vector<VideoCapture> res;
    for (int i = 0; i < num; i++) {
        VideoCapture cap;
        if (!cap.open(i))
            log(ERROR, "Cannot open video capture with index", i);
        else
            res.emplace_back(cap);
    }

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

    SpeechRecognition::Recognition rec("../modules/speech/data/zero_ru_cont_8k_v3/zero_ru.cd_cont_4000",
                                       "../new.dic", "../gram.jsgf", "../kws.kwlist");
    DataCollector dc(rec);

    RHSpeaker speaker_rh("anna");
    Speaker speaker(speaker_rh);

    CommandProcessor cmdProc(speaker, dc);

    speaker.callbacks["cmd"] = SpeakerCallbackFn([&](string args) {
        cmdProc.processCmd(args);
    });
    dc.speechRecognizer.onKw = SpeechRecognition::CallbackFn([&](string str) {
        speaker.say("Слушаю...");
    });

    //system("amixer -c 1 set \"Mic Boost\" 10%");

    Mat img, imgGray, all;
    while (true) {
        vector<Mat> frames = captureFrames(caps);

        hconcat(frames, all);
        cvtColor(all, imgGray, COLOR_BGR2GRAY);
        vector<uchar> compressed;
        imencode(".jpeg", imgGray, compressed);

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