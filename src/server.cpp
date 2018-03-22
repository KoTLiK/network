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
                        if (!server.receiveMessage(protocol))
                            break;
                        protocol.front();
                        protocol.pop();
                        std::cout << "RECV: " << protocol.getCurrentMessage() << std::endl; // Serve
                        server.sendMessage(protocol.getCurrentMessage());
                    } catch (Net::NetworkException& e) {
                        std::cerr << e.what() << std::endl;
                        break;
                    }
                }
                server.closeConnection();
                std::cout << "Connection closed.\n";
            }
        } else { // Net::Datagram::UDP
            while (true) {
                try {
                    if (!server.receiveMessage(protocol))
                        break;
                    protocol.front();
                    protocol.pop();
                    std::cout << "RECV: " << protocol.getCurrentMessage() << std::endl; // Serve
                    server.sendMessage(protocol.getCurrentMessage());
                } catch (Net::NetworkException& e) {
                    std::cerr << e.what() << std::endl;
                    break;
                }
            }
        }
    } catch (Net::NetworkException& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}