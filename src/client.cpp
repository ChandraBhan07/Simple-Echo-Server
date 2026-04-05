#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>

int main()
{
    // Step 0 Initialize network stack
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (result != 0)
    {
        std::cout << "WSAStartup failed with error: " << result << "\n";
        return 1;
    }

    // Step 1 create client socket - socket = family type protocol
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed, error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // Step 2: Define target SERVER address (not client!)
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5347);

    // If server is on same machine:
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Step 3: Connect to server
    int connectingSocket = connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));

    if (connectingSocket == SOCKET_ERROR)
    {
        std::cout << "Connect failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Step 4: Send data
    const char *msg = "Hello from client";
    int bytesSent = send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cout << "Send failed: " << WSAGetLastError() << "\n";
    }

    // Step 5: Receive response

    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        std::cout << "Recv failed: " << WSAGetLastError() << "\n";
    }
    else if (bytesReceived == 0)
    {
        std::cout << "Server closed connection\n";
    }
    else
    {
        buffer[bytesReceived] = '\0';
        std::cout << "Server says: " << buffer << "\n";
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

// client flow
// socket()
// connect()
// send()/recv()

// FULL FLOW
// Server                         Client

// socket()                       socket()
// bind()
// listen()

// accept()   ◀───────────────    connect()

// recv()     ◀───────────────    send()
// send()     ───────────────▶    recv()

// Client size common wsa error codes
// Code	    Meaning
// 10061	Connection refused (server not listening)
// 10060	Connection timed out (server unreachable)
// 10049	Invalid address (wrong IP/port)
