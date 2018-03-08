#include <iostream>
#include <csignal>

#include "Network.h"

Net::Server server(Net::Datagram::TCP);

void signalHandler(int) {
    server.stop();
    std::cout << "\nShutting down...\n";
    exit(0);
}

int main() {
    std::string input;

    signal(SIGINT, signalHandler);

    std::string delimiter = ".";
    Net::Protocol protocol(delimiter);

    try {
        server.start(8080);
        if (server.getDatagram() == Net::Datagram::TCP) {
            server.setListen(2);

            while (true) {
                server.openConnection();
                std::cout << "Client connected.\n";
                while (true) {
                    try {
                        server.receiveMessage(protocol);
                        protocol.front();
                        protocol.pop();
                        std::cout << "RECV: " << protocol.getCurrentMessage() << std::endl;
                        server.sendMessage(protocol.getCurrentMessage());
                    } catch (Net::NetworkException &e) {
                        std::cerr << e.what() << std::endl;
                        break;
                    }
                }
                std::cout << "Connection closed.\n";
            }
        }
    } catch (Net::NetworkException &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}