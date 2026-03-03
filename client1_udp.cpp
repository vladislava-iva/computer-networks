#include <winsock2.h>
#include <windows.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 54001
#define MAGIC "DATA"

#define IP "127.0.0.1"

struct Data {
    int   id;
    char  name[20];
    float value;
};

static const int PKT_SIZE = 8 + (int)sizeof(Data);

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cerr << "socket() failed: " << WSAGetLastError() << endl;
        WSACleanup(); return 1;
    }

    // Тайм-аут для приёма
    DWORD timeout = 500;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    // Тайм-аут для отправки
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    BOOL ka = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&ka, sizeof(ka));

    sockaddr_in server{};
    server.sin_family=AF_INET;
    server.sin_port= htons(PORT);
    server.sin_addr.s_addr=inet_addr(IP);

    cout << "клиент запущен, сервер: " << IP << ":" << PORT << endl;

    for (int i = 1; i <= 3; ++i)
    {
        Data msg{};
        msg.id    = i;
        snprintf(msg.name, sizeof(msg.name), "Client_%d", i);
        msg.value = 1.1f * i;

        int payloadSize = sizeof(Data);

        char packet[8 + sizeof(Data)];
        memcpy(packet,     MAGIC,        4);
        memcpy(packet + 4, &payloadSize, 4);
        memcpy(packet + 8, &msg,        payloadSize);

        cout << "\nотправляем: id=" << msg.id << ", name=" << msg.name << ", value=" << msg.value << endl;

        if (sendto(sock, packet, PKT_SIZE, 0,
                   (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            cerr << WSAGetLastError() << endl;
            continue;
        }

        char buf[PKT_SIZE + 32];
        sockaddr_in from{};
        int fromLen = sizeof(from);

        int received = recvfrom(sock, buf, sizeof(buf), 0,
                                (sockaddr*)&from, &fromLen);

        if (received == SOCKET_ERROR)
        {
            int e = WSAGetLastError();
            if (e == WSAETIMEDOUT)
                cerr << "тайм-аут ожидания эхо " << endl;
            else
                cerr <<  e << endl;
            continue;
        }

        if (received != PKT_SIZE)
        {
            cerr << "неверный размер эхо: " << received << endl;
            continue;
        }

        if (memcmp(buf, MAGIC, 4) != 0)
        {
            cerr << "неверный MAGIC в эхо" << endl;
            continue;
        }

        Data echo{};
        memcpy(&echo, buf + 8, sizeof(Data));
        echo.name[19] = '\0';

        cout << "эхо получено: id=" << echo.id
             << ", name=" << echo.name << ", value=" << echo.value << endl;
    }

    closesocket(sock);
    WSACleanup();
    cout << "\nготово" << endl;
    return 0;
}