#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>

using namespace std;

struct Request {
    char surname[50];
    int h;
    int w;
};

struct Response {
    char result[20];
};

int main() {
    string name;
    cout << "введите фамилию: ";
    getline(cin, name);

    Request req;
    strncpy(req.surname, name.c_str(), sizeof(req.surname) - 1);
    req.surname[sizeof(req.surname) - 1] = '\0';

    string answer_file = "f2_" + name + ".bin";
    streampos old_pos = 0;

    while (true) {
        cout << "рост вес: ";
        cin >> req.h >> req.w;
        cin.ignore();

        if (req.h == -1) break;

        ofstream file("f1.bin", ios::binary | ios::app);
        if (file.is_open()) {
            file.write((char*)&req, sizeof(req));
            cout << "отправлено" << endl;
            file.close();
        }
        else {
            cout << "ошибка" << endl;
        }

        ifstream ans(answer_file, ios::binary);
        if (!ans.is_open()) {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        ans.seekg(0, ios::end);
        streampos cur = ans.tellg();

        if (cur > old_pos) {
            ans.seekg(old_pos, ios::beg);

            Response resp;
            while (ans.read((char*)&resp, sizeof(resp))) {
                cout << "Ответ: " << resp.result << endl;
            }

            old_pos = cur;
        }

        ans.close();
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}