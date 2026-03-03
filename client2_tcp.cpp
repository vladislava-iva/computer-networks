#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 54000
#define MAGIC "DATA"
#define BUF_SIZE 1024

#define IP "127.0.0.1"

struct Data {
    int   id;
    char  name[20];
    float value;
};

bool func(SOCKET s, char* buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int r = recv(s, buf + total, len - total, 0);
        if (r == 0)return false;
        if (r == SOCKET_ERROR) return false;
        total += r;
    }
    return true;
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        cerr  << WSAGetLastError() << endl;
        WSACleanup(); return 1;
    }

    //тайм-аут для приёма
    DWORD timeout = 3000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // тайм-аут для отправки
    DWORD timeoutSnd = 500;
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeoutSnd, sizeof(timeoutSnd));

    BOOL ka = TRUE;
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&ka, sizeof(ka));

    sockaddr_in server{};
    server.sin_family= AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);

    if (connect(s, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        cerr << WSAGetLastError() << endl;
        closesocket(s); WSACleanup(); return 1;
    }

    cout << "подключено к " << IP << ":" << PORT << endl;

    string input;
    int msgId = 1;

while (true)
{
    cout << "\nВведите exit для выхода или 'id name value' для отправки структуры: ";
    getline(cin, input);
    if (input == "exit") break;

    stringstream ss(input);
    string n_s;
    float val;
    int id;

    if (!(ss >> id >> n_s >> val)) {
        cerr << "неверный формат ввода. используйте 'id name value'.\n";
        continue; // пропускаем итерацию, если парсинг неудачный
    }

    // формируем структуру на основе ввода
    Data data{};
    data.id = id;
    strncpy(data.name, n_s.c_str(), sizeof(data.name) - 1); // копируем name, обрезая до 19 символов
    data.name[sizeof(data.name) - 1] = '\0'; // гарантируем null-терминатор
    data.value = val;

    int payloadSize = sizeof(Data);

    // формируем пакет: MAGIC(4) + size(4) + структура
    char pkt[8 + sizeof(Data)];
    memcpy(pkt,     MAGIC,       4);
    memcpy(pkt + 4, &payloadSize, 4);
    memcpy(pkt + 8, &data,        payloadSize);

    int totalSend = 8 + payloadSize;

    // отправляем
    if (send(s, pkt, totalSend, 0) == SOCKET_ERROR)
    {
        cerr << "ошибка отправки: " << WSAGetLastError() << endl;
        break;
    }
    cout << "отправлено " << totalSend << " байт: " << "id=" << data.id << ", name=" << data.name << ", value=" << data.value << endl;
}

    shutdown(s, SD_BOTH);
    closesocket(s);
    WSACleanup();
    cout << "отключено." << endl;
    return 0;
}
