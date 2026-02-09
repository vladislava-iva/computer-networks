#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    string surname;
    cout << "введите фамилию: ";
    getline(cin, surname);

    if (surname.empty()) surname = "anonym";

    streampos last_pos = 0;

    cout << "введите рост и вес:" << endl;

    while (true) {
        string input;
        getline(cin, input);

        if (input == "exit") {
            break;
        }

        if (input.empty()) continue;

        ofstream file("f1.txt", ios::app);
        if (file.is_open()) {
            file << surname << " " << input << endl;
            cout << "отправлено" << endl;
            file.close();
        } else {
            cout << "ошибка" << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}