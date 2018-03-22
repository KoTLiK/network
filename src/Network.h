//
// Created by milan on 3/3/18.
//

#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <exception>
#include <string>
#include <utility>
#include <strings.h>
#include <queue>
#include <cstring>

namespace Net {

static const unsigned BUFFER_SIZE = 1024;

enum class Datagram {
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM
};

enum class Error {
    SEND,
    RECV,
    BIND,
    SOCKET,
    LISTEN,
    ACCEPT,
    ADDR_INFO,
    FAMILY,
    CONNECT,
    RESOLVE,
    CONVERT
};

class NetworkException : public std::exception {
private:
    const std::string s;
    const Error e{};

public:
    explicit NetworkException(std::string e) throw()
        : s{ std::move(e) } {}
    NetworkException(Error error, std::string e) throw()
        : e{ error }, s{ std::move(e) } {}
    ~NetworkException() throw() override = default;
    const char* what() const throw() override { return s.c_str(); }
    const Error which() const throw() { return e; }
};

struct Socket {
    Socket()
        : _length{ sizeof(_socket) } {}
    socklen_t& length() { return _length; }
    socklen_t* lengthPtr() { return &_length; }
    sockaddr* operator&() { return reinterpret_cast<sockaddr*>(&_socket); }
    sockaddr_in& ipv4() { return _socket.si; }
    sockaddr_in6& ipv6() { return _socket.si6; }
    const sockaddr_in& ipv4() const { return _socket.si; }
    const sockaddr_in6& ipv6() const { return _socket.si6; }

    void* addr() {
        switch (family()) {
        case AF_INET:
            return &_socket.si.sin_addr;
        case AF_INET6:
            return &_socket.si6.sin6_addr;
        default:
            throw NetworkException(Error::FAMILY, "Oh crap!");
        }
    }

    socklen_t addrLength() {
        switch (family()) {
        case AF_INET:
            return INET_ADDRSTRLEN;
        case AF_INET6:
            return INET6_ADDRSTRLEN;
        default:
            throw NetworkException(Error::FAMILY, "Oh crap!");
        }
    }
    sa_family_t family() { return _socket.sa.sa_family; }

private:
    union {
        sockaddr sa;
        sockaddr_in si;
        sockaddr_in6 si6;
    } _socket{};
    socklen_t _length{};
};

class Protocol {
    friend class Network;
    friend class Server;

protected:
    std::queue<std::string> container;
    std::string package;
    std::string delimiter;
    std::string currentMessage;
    unsigned long delimiterLength;

    bool checkAndAppend(const char* buffer, unsigned bytes) {
        package += std::string(buffer, bytes);
        return package.find(delimiter.c_str(), 0, delimiterLength) != std::string::npos;
    }

    void split() {
        unsigned long position;
        std::string temp;
        while ((position = package.find(delimiter.c_str(), 0, delimiterLength)) != std::string::npos) {
            temp = package.substr(0, position + delimiterLength);
            package = package.substr(position + delimiterLength);
            container.push(temp);
        }
    }

public:
    explicit Protocol(std::string& delimiter) { setDelimiter(delimiter); }
    Protocol(std::string& delimiter, size_t length) { setDelimiter(delimiter, length); }
    Protocol(const char* delimiter, size_t length) { setDelimiter(delimiter, length); }

    bool empty() const { container.empty(); }

    void setDelimiter(std::string& delimiter) {
        this->delimiter = std::string(delimiter);
        delimiterLength = delimiter.length();
    }

    void setDelimiter(std::string& delimiter, size_t length) {
        this->delimiter = std::string(delimiter, 0, length);
        delimiterLength = length;
    }

    void setDelimiter(const char* delimiter, size_t length) {
        this->delimiter = std::string(delimiter, length);
        delimiterLength = length;
    }

    std::string getCurrentMessage() const { return currentMessage; }

    std::string front() {
        currentMessage = container.empty() ? "" : container.front();
        return currentMessage;
    }

    void pop() {
        if (!container.empty())
            container.pop();
    }
};

class Network {
protected:
    int socketDescriptor{}, connection{}, error{};
    const Datagram datagram;
    const unsigned bufferSize;
    char* buffer;

public:
    Network(Datagram d, unsigned bufferSize)
        : datagram{ d }, bufferSize{ bufferSize } { buffer = new char[bufferSize]; }
    ~Network() {
        close(socketDescriptor);
        delete[] buffer;
    }

    virtual void sendMessage(const std::string& message, int socket) const {
        if (message.empty())
            return;
        if (send(socket, message.c_str(), message.size(), 0) < 0)
            throw NetworkException(Error::SEND, "Cannot send message. " + std::string(strerror(errno)));
    }

    virtual bool receiveMessage(Protocol& protocol, int socket) {
        int i = 0;
        if (protocol.empty()) {
            while (true) {
                bzero(buffer, bufferSize + 1);
                i = static_cast<int>(recv(socket, buffer, bufferSize, 0));
                if (i == 0)
                    break;
                else if (i < 0)
                    throw NetworkException(Error::RECV, "Ooops. Receive goes wrong. " + std::string(strerror(errno)));
                else if (protocol.checkAndAppend(buffer, static_cast<unsigned>(i)))
                    break;
            }
        }
        protocol.split();
        return i != 0;
    }

    static std::string getCurrentIP(const int descriptor) {
        Socket socket{};

        if (getsockname(descriptor, &socket, socket.lengthPtr()) != 0)
            throw NetworkException(Error::RESOLVE, "Cannot resolve my own address.");

        char addr[socket.addrLength()];

        if (inet_ntop(socket.family(), socket.addr(), addr, socket.addrLength()) == nullptr)
            throw NetworkException(Error::CONVERT, "Unable to convert my binary name.");

        return std::string(addr);
    }

    Datagram getDatagram() const { return datagram; }
};

class Server : public Network {
private:
    Socket server{}, client{};

public:
    explicit Server(Datagram d, unsigned bufferSize = Net::BUFFER_SIZE)
        : Network(d, bufferSize) {}

    void start(uint16_t port) {
        if ((socketDescriptor = socket(PF_INET6, static_cast<int>(datagram), 0)) < 0)
            throw NetworkException(Error::SOCKET, "The socket cannot be created.");

        bzero(&server, server.length());
        server.ipv6().sin6_family = AF_INET6;
        server.ipv6().sin6_addr = in6addr_any;
        server.ipv6().sin6_port = htons(port);

        if (bind(socketDescriptor, &server, server.length()) < 0)
            throw NetworkException(Error::BIND, "Cannot bind...not enough mana.");
    }

    void setListen(const int listenQueue) const {
        if (listen(socketDescriptor, listenQueue) < 0)
            throw NetworkException(Error::LISTEN, "I can't hear yooou...");
    }

    int openConnection() {
        if ((connection = accept(socketDescriptor, &client, client.lengthPtr())) < 0)
            throw NetworkException(Error::ACCEPT, "Challenge NOT accepted...nor the connection...");
        return connection;
    }

    void stop() const { close(socketDescriptor); }
    int getServerSocketDescriptor() const { return socketDescriptor; }
    int getConnectionSocketDescriptor() const { return connection; }
    void closeConnection(int socket) const { close(socket); }
    void closeConnection() const { closeConnection(connection); }

    void sendToMessage(const std::string& message, int socket) {
        if (message.empty())
            return;
        if (sendto(socket, message.c_str(), message.size(), 0, &client, client.length()) < 0)
            throw NetworkException(Error::SEND, "Cannot send message. " + std::string(strerror(errno)));
    }

    void sendMessage(const std::string& message) {
        if (datagram == Datagram::TCP)
            Network::sendMessage(message, connection);
        else
            sendToMessage(message, socketDescriptor);
    }

    bool receiveFromMessage(Protocol& protocol, int socket) {
        int i = 0;
        if (protocol.empty()) {
            while (true) {
                bzero(buffer, bufferSize + 1);
                i = static_cast<int>(recvfrom(socket, buffer, bufferSize, 0, &client, client.lengthPtr()));
                if (i == 0)
                    break;
                else if (i < 0)
                    throw NetworkException(Error::RECV, "Ooops. Receive goes wrong. " + std::string(strerror(errno)));
                else if (protocol.checkAndAppend(buffer, static_cast<unsigned>(i)))
                    break;
            }
        }
        protocol.split();
        return i != 0;
    }

    bool receiveMessage(Protocol& protocol) {
        if (datagram == Datagram::TCP)
            return Network::receiveMessage(protocol, connection);
        else
            return receiveFromMessage(protocol, socketDescriptor);
    }

    void sendMessage(const std::string& message, int socket) const override { Network::sendMessage(message, socket); }
    bool receiveMessage(Protocol& protocol, int socket) override { return Network::receiveMessage(protocol, socket); }
    Socket getClient() const { return client; }
};

class Client : public Network {
private:
    struct addrinfo hints;
    struct addrinfo* result;

public:
    explicit Client(Datagram d, unsigned bufferSize = BUFFER_SIZE)
        : Network(d, bufferSize) {}

    void setConnection(const std::string& server_ip, const std::string& port) {
        bzero(&hints, sizeof(hints));
        hints.ai_flags = AI_NUMERICSERV;                /* Only numeric service (port) */
        hints.ai_family = AF_UNSPEC;                    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = static_cast<int>(datagram); /* Datagram socket */
        hints.ai_protocol = 0;                          /* Any protocol */

        if ((error = getaddrinfo(server_ip.c_str(), port.c_str(), &hints, &result)) != 0)
            throw NetworkException(Error::ADDR_INFO, gai_strerror(error));
    }

    void connectToServer() {
        struct addrinfo* rp = nullptr;
        for (rp = result; rp != nullptr; rp = rp->ai_next) {
            if ((socketDescriptor = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
                continue; // Try next one
            if (connect(socketDescriptor, rp->ai_addr, rp->ai_addrlen) == 0)
                break; // Success
            close(socketDescriptor);
        }

        if (rp == nullptr) {
            if (datagram == Datagram::UDP)
                throw NetworkException(Error::CONNECT, "UDP Service is unavailable.");
            else
                throw NetworkException(Error::CONNECT, "Cannot connect to the server.");
        }

        freeaddrinfo(result); // Free memory
    }

    int getSocketDescriptor() const { return socketDescriptor; }
    void sendMessage(const std::string& message) const { Network::sendMessage(message, socketDescriptor); }
    bool receiveMessage(Protocol& protocol) { return Network::receiveMessage(protocol, socketDescriptor); }
};
} // namespace Net

#endif //NETWORK_NETWORK_H
