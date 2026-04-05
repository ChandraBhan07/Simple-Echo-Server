#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>

int main()
{
    // --------------------- Network Stack ---------------------
    // Winsock 101 (Windows Sockets API)
    // Windows doesn’t allow sockets to work “out of the box” like Linux. So initialize the networking stack first.

    // Purpose: Initialize Windows Sockets DLL
    // Parameters:
    //  MAKEWORD(2, 2) → Request Winsock 2.2
    //  wsa → receives info about the implementation
    //  Return value:
    //      0 → success
    //      non-zero → failure

    // Without it, all socket calls fail (socket(), bind(), accept(), connect()…)
    // Common error: accept() immediately returns INVALID_SOCKET

    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (result != 0)
    {
        std::cout << "WSAStartup failed with error: " << result << "\n";
        return 1;
    }

    // On Windows, standard errno does not work for sockets. Instead, use:

    // int err = WSAGetLastError();
    // std::cout << "Error code: " << err << "\n";
    // Returns Winsock-specific error code
    // Examples:
    // Code	Meaning
    // 10022	Invalid argument
    // 10048	Address already in use
    // 10054	Connection reset by peer
    // 10061	Connection refused

    // ----------------------- SOCKET -------------------------
    // socket is a handle to a kernel socket object
    // socket() → bind() → listen() → accept()
    SOCKET serverSocket;

    // socket states
    // After socket() → uninitialized / inactive
    // After listen() serverSocket → passive (listening)
    // After accept() clientSocket → active

    // Step 1 We decide address family (domain)
    // AF_INET → IPv4   |   AF_INET6 → IPv6  |   AF_UNIX → local machine (IPC)
    int af = AF_INET;

    // Step 2 We choose socket type
    // SOCK_STREAM → TCP   |   SOCK_DGRAM → UDP
    int type = SOCK_STREAM;

    // Step 3 Choose protocol
    // 0 for default protocol with this combination
    int protocol = 0;

    serverSocket = socket(af, type, protocol);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed, error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    // int bind(socket, address, size)
    // bind() registers socket with the OS networking system OR
    // bind() connects socket to the network routing table inside OS
    // address is a pointer to an address structure and expects a generic address type
    // socket contains address structure but we need to pass it explictely
    // size is how many bytes to read
    // OS does NOT trust the pointer and needs size to interpret memory safely

    // for ipv4, address str is [ family | port | ip (32-bit) | padding ]

    // Step 1 Create ipv4 specific structure and zero initialize
    struct sockaddr_in serverAddr = {};

    // Step 2 Set family
    serverAddr.sin_family = AF_INET;

    // Step 3 Set port (convert to network byte order)
    serverAddr.sin_port = htons(5347);

    // Step 4 Set ip
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Step 5 Create pointer for bind
    const sockaddr *bindPtr = (const struct sockaddr *)&serverAddr;

    int size = sizeof(serverAddr);

    int bindingSocket = bind(serverSocket, bindPtr, size);
    if (bindingSocket == SOCKET_ERROR)
    {
        std::cout << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // int listen(SOCKET s, int backlog)
    // OS may ignore exact backlog
    // Linux: may silently raise it to SOMAXCONN (default max)
    // Windows: may have its own limit

    // ** Backlog ≠ number of clients you can accept in total
    // Only number waiting to be accept()ed

    // listen(serverSocket, 2)
    // Only 2 clients can wait in queue
    // 3rd client → connection refused

    int listingSocket = listen(serverSocket, 10);
    if (listingSocket == SOCKET_ERROR)
    {
        std::cout << "Listen failed: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // accept() drains the queue
    // what accept does
    // 1. waits for client
    // 2. creates new socket
    // 3. gives client info

    // sockaddr_in clientAddr;
    // int clientSize = sizeof(clientAddr);

    // SOCKET clientSocket;

    // After accept()
    // serverSocket  → still listening
    // clientSocket  → communication socket

    while (true)
    {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cout << "Accept failed\n";
            continue;
        }

        // listening client - receive
        char buffer[1024] = {};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            std::cout << "Client disconnected or error\n";
            closesocket(clientSocket);
            continue;
        }
        std::cout << "Client says: " << buffer << "\n";

        const char *msg = "Hello client, greetings received.";
        send(clientSocket, msg, strlen(msg), 0);
        closesocket(clientSocket);
    }

    closesocket(serverSocket);

    WSACleanup();
    // Cleans up Winsock when program ends
    // Always call after closing sockets
    // Can be called multiple times if WSAStartup() is called multiple times

    return 0;
}

// server flow
// socket()
// bind()
// listen()
// accept()
// recv()/send()

// while (true)
// {
//     wait for a client → accept()
//     talk to client → recv/send
//     close client socket
// }
