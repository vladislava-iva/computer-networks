#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include <sstream>

using namespace std;

struct Request {
    char surname[50];
    int h;
    int w;
};

struct Response {
    char result[20];
};

string zapros(Request req) {
    int ideal = req.h - 100;
    if (req.w >= ideal - 5 && req.w <= ideal + 5) {
        return "нормально";
    }
    else if (req.w < ideal - 5) {
        return "нехватка веса";
    }
    else {
        return "избыток веса";
    }
}

int main() {
    streampos old_pos = 0;

    while (true) {
        ifstream file1("f1.bin", ios::binary);
        if (!file1.is_open()) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        file1.seekg(0, ios::end);
        streampos cur_pos = file1.tellg();
        file1.close();

        if (cur_pos > old_pos) {
            ifstream file2("f1.bin", ios::binary);
            file2.seekg(old_pos, ios::beg);

            Request req;
            while (file2.read((char*)&req, sizeof(req))) {
                string result = zapros(req);

                string name = req.surname;
                string fname = "f2_" + name + ".bin";

                Response resp;
                strncpy(resp.result, result.c_str(), sizeof(resp.result) - 1);
                resp.result[sizeof(resp.result) - 1] = '\0';

                ofstream file3(fname, ios::binary | ios::app);
                if (file3.is_open()) {
                    file3.write((char*)&resp, sizeof(resp));
                    file3.flush();
                    cout << "записан ответ\n";
                }
                else {
                    cout << "ошибка записи в " << fname << endl;
                }
                cout << "ответ: " << result << endl;
            }

            file2.close();
            old_pos = cur_pos;
        }

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}