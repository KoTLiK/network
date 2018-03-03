#include <iostream>

#include "Network.h"

int main() {
    std::string input;
    std::cin >> input;

    Net::Server server(Net::Datagram::TCP);

    std::string delimiter = "\n";
    Net::Protocol protocol(delimiter);

    try {
        server.start("localhost", 6667);
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