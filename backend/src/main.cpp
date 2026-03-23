#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "http_server.h"

int main() {
    try {
        secsgem::HttpServer server;
        server.run(8080);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "backend startup failed: " << ex.what() << std::endl;
        return 1;
    }
}

