#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

using namespace std;

string zapros(string q)
{
    stringstream iss(q);
    string name;
    int h = 0;
    int w = 0;

    iss >> name >> h >> w;

    if (name.empty() || h <= 0 || w <= 0) return "ошибка";

    int ideal = h - 100;

    if (w >= ideal - 5 && w <= ideal + 5) return "нормально";
    else if (w < ideal - 5) return "нехватка веса";
    else return "избыток веса";
}

int main()
{
    streampos old_pos = 0;

    while (true)
    {
        ifstream file1("f1.txt");
        if (!file1.is_open())
        {
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }

        file1.seekg(0, ios::end);
        streampos cur_pos = file1.tellg();
        file1.close();

        if (cur_pos > old_pos)
        {
            ifstream file2("f1.txt");
            file2.seekg(old_pos, ios::beg);
            string line;

            while (getline(file2, line))
            {
                if (line.empty()) continue;

                cout << "запрос обрабатывается..." << endl;

                string result = zapros(line);

                stringstream iss(line);
                string name;
                iss >> name;

                string fname = "f2_" + name + ".txt";

                ofstream file3(fname, ios::app);
                if (file3.is_open())
                {
                    file3 << result << endl;
                    cout << "записан ответ: " << result << endl;
                }
                else
                {
                    cout << "ошибка" << endl;
                }
            }

            file2.close();
            old_pos = cur_pos;
        }

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}