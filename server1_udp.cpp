#include <winsock2.h>
#include <windows.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 54001
#define MAGIC "DATA"

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

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
    {
        cerr << WSAGetLastError() << endl;
        WSACleanup(); 
        return 1;
    }

    //тайм-аут 1 для приема
    DWORD t = 500;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&t, sizeof(t));

    //тайм-аут для отправки
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&t, sizeof(t));

    BOOL ka = TRUE;
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&ka, sizeof(ka));

    BOOL reuse = TRUE;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family= AF_INET;
    addr.sin_port= htons(PORT);
    addr.sin_addr.s_addr =INADDR_ANY;

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << WSAGetLastError() << endl;
        closesocket(s); 
        WSACleanup(); 
        return 1;
    }

    cout << "эхо-сервер запущен на порту " << PORT
         << " (ожидаем пакет " << PKT_SIZE << " байт)" << endl;

    char buff[PKT_SIZE + 32];

    while (true)
    {
        sockaddr_in client{};
        int clientLen = sizeof(client);

        int n = recvfrom(s, buff, sizeof(buff), 0,
                                (sockaddr*)&client, &clientLen);

        if (n == SOCKET_ERROR)
        {
            int e = WSAGetLastError();
            if (e == WSAETIMEDOUT) continue; // тайм-аут
            cerr << e << endl;
            continue;
        }

        char ip[16];
        strncpy(ip, inet_ntoa(client.sin_addr), 15);

        //проверяем размер пакета (не доверяем клиенту)
        if (n != PKT_SIZE)
        {
            cout << "[UDP-S] Неверный размер от " << ip
                 << ": " << n << " байт (ожидалось " << PKT_SIZE << ") — drop" << endl;
            continue;
        }

        //проверяем MAGIC (не доверяем клиенту)
        if (memcmp(buff, MAGIC, 4) != 0)
        {
            cout << "неверный MAGIC от " << ip  << endl;
            continue;
        }

        int sz = 0;
        memcpy(&sz, buff + 4, 4);

        if (sz != (int)sizeof(Data))
        {
            cout << "неверный payload size " << sz
                 << " от " << ip << endl;
            continue;
        }

        Data st{};
        memcpy(&st, buff + 8, sizeof(Data));
        st.name[19] = '\0';

        cout << "от " << ip << ": "
             << "id=" << st.id << ", name=" << st.name << ", value=" << st.value << endl;

        //отправляем эхо обратно
        if (sendto(s, buff, n, 0, (sockaddr*)&client, clientLen) == SOCKET_ERROR)
            cerr <<  WSAGetLastError() << endl;
        else
            cout << "эхо отправлено к " << ip << endl;
    }

    closesocket(s);
    WSACleanup();
    return 0;
}