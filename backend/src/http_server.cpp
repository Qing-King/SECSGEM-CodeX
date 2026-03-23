#include "http_server.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(SECSGEM_POSIX_SERVER)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

std::string build_devices_json() {
    return R"({"items":[{"id":"eqp01","name":"EQP-01 Etcher","ip":"10.10.20.15","port":5000,"mode":"active","status":"connected","selected":true},{"id":"eqp02","name":"EQP-02 Cleaner","ip":"0.0.0.0","port":5001,"mode":"passive","status":"disconnected","selected":false}]})";
}

std::string build_messages_json(const std::string& device_id) {
    std::ostringstream oss;
    oss << "["
        << "{\"id\":\"" << device_id << "-m1\",\"timestamp\":\"2026-03-23T10:00:01+08:00\",\"direction\":\"send\",\"stream\":1,\"function\":13,\"note\":\"Establish Communications Request\"},"
        << "{\"id\":\"" << device_id << "-m2\",\"timestamp\":\"2026-03-23T10:00:02+08:00\",\"direction\":\"recv\",\"stream\":1,\"function\":14,\"note\":\"Establish Communications Acknowledge\"},"
        << "{\"id\":\"" << device_id << "-m3\",\"timestamp\":\"2026-03-23T10:00:05+08:00\",\"direction\":\"recv\",\"stream\":6,\"function\":11,\"note\":\"Event Report\"}"
        << "]";
    return oss.str();
}

std::string extract_path(const std::string& request) {
    const auto first_space = request.find(' ');
    if (first_space == std::string::npos) {
        return "/";
    }
    const auto second_space = request.find(' ', first_space + 1);
    if (second_space == std::string::npos) {
        return "/";
    }
    return request.substr(first_space + 1, second_space - first_space - 1);
}

std::string response_for_path(const std::string& path, int& status_code, std::string& status_text) {
    if (path == "/api/devices") {
        status_code = 200;
        status_text = "OK";
        return build_devices_json();
    }

    const std::string prefix = "/api/devices/";
    const std::string suffix = "/messages";
    if (path.rfind(prefix, 0) == 0 && path.size() > prefix.size() + suffix.size()) {
        const auto suffix_pos = path.rfind(suffix);
        if (suffix_pos != std::string::npos && suffix_pos + suffix.size() == path.size()) {
            const auto device_id = path.substr(prefix.size(), suffix_pos - prefix.size());
            status_code = 200;
            status_text = "OK";
            return build_messages_json(device_id);
        }
    }

    status_code = 404;
    status_text = "Not Found";
    return R"({"error":"route_not_found"})";
}

std::string build_http_response(const std::string& body, int status_code, const std::string& status_text) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

}  // namespace

namespace secsgem {

void HttpServer::run(int port) {
#if defined(SECSGEM_POSIX_SERVER)
    const int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("failed to create socket");
    }

    int opt = 1;
    ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(server_fd);
        throw std::runtime_error("failed to bind socket");
    }

    if (::listen(server_fd, 16) < 0) {
        ::close(server_fd);
        throw std::runtime_error("failed to listen");
    }

    std::cout << "HTTP server listening on 0.0.0.0:" << port << std::endl;

    while (true) {
      sockaddr_in client_address {};
      socklen_t client_size = sizeof(client_address);
      const int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_address), &client_size);
      if (client_fd < 0) {
          continue;
      }

      char buffer[4096] = {0};
      const auto bytes = ::read(client_fd, buffer, sizeof(buffer) - 1);
      if (bytes > 0) {
          const std::string request(buffer, static_cast<std::size_t>(bytes));
          const auto path = extract_path(request);
          int status_code = 200;
          std::string status_text = "OK";
          const auto body = response_for_path(path, status_code, status_text);
          const auto response = build_http_response(body, status_code, status_text);
          ::send(client_fd, response.c_str(), response.size(), 0);
      }

      ::close(client_fd);
    }
#else
    (void)port;
    throw std::runtime_error("minimal HTTP server is implemented for Linux deployment only");
#endif
}

}  // namespace secsgem

