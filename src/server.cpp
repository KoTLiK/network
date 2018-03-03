#include <iostream>

#include "Network.h"

int main() {

    Net::Server server(Net::Datagram::TCP);
    std::string delimiter = ".";
    Net::Protocol protocol(delimiter);
    server.receiveMessage(protocol, 2);

    return 0;
}