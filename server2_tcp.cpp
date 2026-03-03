#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 54000
#define BUF_SIZE 1024
#define MAGIC "DATA"

bool rx(SOCKET sock, char* buf, int len)
{
    int total = 0;
    while (total < len)
    {
        int r = recv(sock, buf + total, len - total, 0);
        if (r == 0) return false; // клиент закрыл соединение
        if (r == SOCKET_ERROR) return false; // ошибка или тайм-аут
        total += r;
    }
    return true;
}

//для одного клиента
void h(SOCKET client)
{
    //тайм-аут для приёма данных
    DWORD t_idle= 30000; //след запрос
    DWORD t_tr = 500; //передача

    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&t_idle, sizeof(t_idle));

    //тайм-аут для отправки данных 
    setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, (char*)&t_tr, sizeof(t_tr));

    BOOL ka = TRUE;
    setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, (char*)&ka, sizeof(ka));

    linger lin;
    lin.l_onoff  = 1;
    lin.l_linger = 2;
    setsockopt(client, SOL_SOCKET, SO_LINGER, (char*)&lin, sizeof(lin));

    cout << "клиент подключён" << endl;

    char header[8];

    // конечный автомат
    while (true)
    {
        if (!rx(client, header, 8))
        {
            cout << "клиент отключился или тайм-аут" << endl;
            break;
        }

        // проверяем MAGIC (не доверяем клиенту )
        if (memcmp(header, MAGIC, 4) != 0)
        {
            cout << "неверный MAGIC " << endl;
            break;
        }

        int size = 0;
        memcpy(&size, header + 4, 4);

        // проверяем длину (не доверяем клиенту )
        if (size <= 0 || size > BUF_SIZE)
        {
            cout << "неверный размер payload: " << size  << endl;
            break;
        }
        
        //короткий таймаут перед чтением тела
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&t_tr, sizeof(t_tr));

        // читаем  ровно size байт
        char body[BUF_SIZE];
        if (!rx(client, body, size))
        {
            cout << "клиент отключился или тайм-аут при чтении тела" << endl;
            break;
        }

        cout << "получено " << size << " байт данных, отправляем эхо" << endl;

        // ждём следующий запрос
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&t_idle, sizeof(t_idle));

        // отправляем эхо
        if (send(client, header, 8,    0) == SOCKET_ERROR ||
            send(client, body,   size, 0) == SOCKET_ERROR)
        {
            cout << "ошибка отправки: " << WSAGetLastError() << endl;
            break;
        }
    }

    shutdown(client, SD_BOTH);
    closesocket(client);
    cout << "соединение закрыто" << endl;
    //поток завершается — не зомби (detach в main)
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        cerr  << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // переиспользование порта после перезапуска
    BOOL ru = TRUE;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*)&ru, sizeof(ru));

    sockaddr_in addr{};
    addr.sin_family= AF_INET;
    addr.sin_port= htons(PORT);
    addr.sin_addr.s_addr= INADDR_ANY;

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << WSAGetLastError() << endl;
        closesocket(server); WSACleanup(); return 1;
    }

    //слушаемс...
    listen(server, SOMAXCONN);
    cout << "TCP сервер запущен на порту " << PORT << endl;

    // бесконечный цикл тк сервер не убиваем ни при каких ошибках клиента
    while (true)
    {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET)
        {
            cerr << WSAGetLastError()  << endl;
            continue; // сервер не падает
        }

        // detach() (поток не остаётся в зомби-состоянии)
        thread(h, client).detach();
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
