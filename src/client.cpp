#include <iostream>

#include "Network.h"

int main() {
    std::string input;
    std::cin >> input;

    Net::Client client(Net::Datagram::TCP);

    std::string delimiter = "\n";
    Net::Protocol protocol(delimiter);

    try {
        client.setConnection("localhost", "6667");
        client.connectToServer();
        std::cout << "Connected.\n";

        while (true) {
            if (protocol.empty()) {
                std::cin >> input;
                client.sendMessage(input);
            }
            client.receiveMessage(protocol);
            protocol.front();
            protocol.pop();
            std::cout << "RECV: " << protocol.getCurrentMessage() << std::endl;
        }

    } catch (Net::NetworkException &e) {
        std::cerr << e.what() << std::endl;
    }


    return 0;
}