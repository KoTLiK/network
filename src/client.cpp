#include <iostream>

#include "Network.h"

int main() {
    std::string input;

    Net::Client client(Net::Datagram::TCP);

    std::string delimiter = ".";
    Net::Protocol protocol(delimiter);

    try {
        client.setConnection("localhost", "8080");
        client.connectToServer();

        while (true) {
            if (protocol.empty()) {
                while (true) {
                    std::getline(std::cin, input);
                    client.sendMessage(input);
                    if (input.find(delimiter) != std::string::npos)
                        break;
                }
            }
            if (!client.receiveMessage(protocol))
                break;
            protocol.front();
            protocol.pop();
            std::cout << "RECV: " << protocol.getCurrentMessage() << std::endl; // Serve
        }

    } catch (Net::NetworkException& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}