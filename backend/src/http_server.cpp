#include "http_server.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

#if defined(SECSGEM_POSIX_SERVER)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

std::string device_mode_to_string(const secsgem::DeviceMode mode) {
    return mode == secsgem::DeviceMode::Active ? "active" : "passive";
}

std::string device_status_to_string(const secsgem::DeviceStatus status) {
    switch (status) {
    case secsgem::DeviceStatus::Connected:
        return "connected";
    case secsgem::DeviceStatus::Connecting:
        return "connecting";
    case secsgem::DeviceStatus::Disconnected:
    default:
        return "disconnected";
    }
}

std::string json_escape(const std::string& value) {
    std::ostringstream oss;
    for (const char ch : value) {
        switch (ch) {
        case '"':
            oss << "\\\"";
            break;
        case '\\':
            oss << "\\\\";
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\r':
            oss << "\\r";
            break;
        case '\t':
            oss << "\\t";
            break;
        default:
            oss << ch;
            break;
        }
    }
    return oss.str();
}

std::string message_to_json(const secsgem::MessageEvent& message) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":\"" << json_escape(message.id) << "\","
        << "\"timestamp\":\"" << json_escape(message.timestamp) << "\","
        << "\"direction\":\"" << json_escape(message.direction) << "\","
        << "\"stream\":" << message.stream << ","
        << "\"function\":" << message.function << ","
        << "\"note\":\"" << json_escape(message.note) << "\""
        << "}";
    return oss.str();
}

std::string make_timestamp() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto time = clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

std::string extract_method(const std::string& request) {
    const auto first_space = request.find(' ');
    return first_space == std::string::npos ? "GET" : request.substr(0, first_space);
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

std::string extract_header(const std::string& request, const std::string& header_name) {
    const std::string pattern = header_name + ":";
    const auto pos = request.find(pattern);
    if (pos == std::string::npos) {
        return "";
    }

    auto value_start = pos + pattern.size();
    while (value_start < request.size() && (request[value_start] == ' ' || request[value_start] == '\t')) {
        ++value_start;
    }

    auto value_end = request.find("\r\n", value_start);
    if (value_end == std::string::npos) {
        value_end = request.size();
    }
    return request.substr(value_start, value_end - value_start);
}

std::array<std::uint32_t, 5> sha1_digest(const std::string& input) {
    std::vector<std::uint8_t> data(input.begin(), input.end());
    const auto original_bits = static_cast<std::uint64_t>(data.size()) * 8ULL;
    data.push_back(0x80);
    while ((data.size() % 64U) != 56U) {
        data.push_back(0x00);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        data.push_back(static_cast<std::uint8_t>((original_bits >> shift) & 0xffU));
    }

    std::uint32_t h0 = 0x67452301U;
    std::uint32_t h1 = 0xEFCDAB89U;
    std::uint32_t h2 = 0x98BADCFEU;
    std::uint32_t h3 = 0x10325476U;
    std::uint32_t h4 = 0xC3D2E1F0U;

    for (std::size_t offset = 0; offset < data.size(); offset += 64U) {
        std::array<std::uint32_t, 80> words {};
        for (std::size_t i = 0; i < 16U; ++i) {
            const std::size_t idx = offset + i * 4U;
            words[i] = (static_cast<std::uint32_t>(data[idx]) << 24U)
                | (static_cast<std::uint32_t>(data[idx + 1U]) << 16U)
                | (static_cast<std::uint32_t>(data[idx + 2U]) << 8U)
                | static_cast<std::uint32_t>(data[idx + 3U]);
        }
        for (std::size_t i = 16U; i < 80U; ++i) {
            const auto value = words[i - 3U] ^ words[i - 8U] ^ words[i - 14U] ^ words[i - 16U];
            words[i] = (value << 1U) | (value >> 31U);
        }

        std::uint32_t a = h0;
        std::uint32_t b = h1;
        std::uint32_t c = h2;
        std::uint32_t d = h3;
        std::uint32_t e = h4;

        for (std::size_t i = 0; i < 80U; ++i) {
            std::uint32_t f = 0;
            std::uint32_t k = 0;

            if (i < 20U) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999U;
            } else if (i < 40U) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1U;
            } else if (i < 60U) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDCU;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6U;
            }

            const auto rotate_a = (a << 5U) | (a >> 27U);
            const auto rotate_b = (b << 30U) | (b >> 2U);
            const auto temp = rotate_a + f + e + k + words[i];
            e = d;
            d = c;
            c = rotate_b;
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    return {h0, h1, h2, h3, h4};
}

std::string base64_encode(const std::uint8_t* data, const std::size_t length) {
    static constexpr char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string output;
    output.reserve(((length + 2U) / 3U) * 4U);

    for (std::size_t i = 0; i < length; i += 3U) {
        const std::uint32_t octet_a = data[i];
        const std::uint32_t octet_b = (i + 1U < length) ? data[i + 1U] : 0U;
        const std::uint32_t octet_c = (i + 2U < length) ? data[i + 2U] : 0U;
        const std::uint32_t triple = (octet_a << 16U) | (octet_b << 8U) | octet_c;

        output.push_back(alphabet[(triple >> 18U) & 0x3fU]);
        output.push_back(alphabet[(triple >> 12U) & 0x3fU]);
        output.push_back(i + 1U < length ? alphabet[(triple >> 6U) & 0x3fU] : '=');
        output.push_back(i + 2U < length ? alphabet[triple & 0x3fU] : '=');
    }

    return output;
}

std::string build_websocket_accept_key(const std::string& client_key) {
    const auto digest = sha1_digest(client_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    std::array<std::uint8_t, 20> bytes {};

    for (std::size_t i = 0; i < digest.size(); ++i) {
        bytes[i * 4U] = static_cast<std::uint8_t>((digest[i] >> 24U) & 0xffU);
        bytes[i * 4U + 1U] = static_cast<std::uint8_t>((digest[i] >> 16U) & 0xffU);
        bytes[i * 4U + 2U] = static_cast<std::uint8_t>((digest[i] >> 8U) & 0xffU);
        bytes[i * 4U + 3U] = static_cast<std::uint8_t>(digest[i] & 0xffU);
    }

    return base64_encode(bytes.data(), bytes.size());
}

std::string build_http_response(const std::string& body, int status_code, const std::string& status_text) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        << "Access-Control-Allow-Headers: Content-Type, Accept\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

std::string build_websocket_handshake_response(const std::string& accept_key) {
    std::ostringstream oss;
    oss << "HTTP/1.1 101 Switching Protocols\r\n"
        << "Upgrade: websocket\r\n"
        << "Connection: Upgrade\r\n"
        << "Sec-WebSocket-Accept: " << accept_key << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n\r\n";
    return oss.str();
}

#if defined(SECSGEM_POSIX_SERVER)
bool send_all(const int client_fd, const std::string& payload) {
    std::size_t sent_total = 0;
    while (sent_total < payload.size()) {
        int flags = 0;
#if defined(MSG_NOSIGNAL)
        flags = MSG_NOSIGNAL;
#endif
        const auto sent = ::send(
            client_fd,
            payload.data() + sent_total,
            payload.size() - sent_total,
            flags);
        if (sent <= 0) {
            return false;
        }
        sent_total += static_cast<std::size_t>(sent);
    }
    return true;
}

bool send_websocket_text_frame(const int client_fd, const std::string& text) {
    std::string frame;
    frame.reserve(text.size() + 10U);
    frame.push_back(static_cast<char>(0x81));

    if (text.size() < 126U) {
        frame.push_back(static_cast<char>(text.size()));
    } else if (text.size() <= 0xffffU) {
        frame.push_back(static_cast<char>(126U));
        frame.push_back(static_cast<char>((text.size() >> 8U) & 0xffU));
        frame.push_back(static_cast<char>(text.size() & 0xffU));
    } else {
        frame.push_back(static_cast<char>(127U));
        for (int shift = 56; shift >= 0; shift -= 8) {
            frame.push_back(static_cast<char>((text.size() >> shift) & 0xffU));
        }
    }

    frame += text;
    return send_all(client_fd, frame);
}
#endif

}  // namespace

namespace secsgem {

HttpServer::HttpServer() {
    DeviceRecord eqp01;
    eqp01.summary = {"eqp01", "EQP-01 Etcher", "10.10.20.15", 5000, DeviceMode::Active, DeviceStatus::Connected, true};
    eqp01.messages = {
        {"eqp01-m1", "2026-03-23T10:00:01+08:00", "send", 1, 13, "Establish Communications Request"},
        {"eqp01-m2", "2026-03-23T10:00:02+08:00", "recv", 1, 14, "Establish Communications Acknowledge"},
        {"eqp01-m3", "2026-03-23T10:00:05+08:00", "recv", 6, 11, "Event Report"}
    };

    DeviceRecord eqp02;
    eqp02.summary = {"eqp02", "EQP-02 Cleaner", "0.0.0.0", 5001, DeviceMode::Passive, DeviceStatus::Disconnected, false};
    eqp02.messages = {
        {"eqp02-m1", "2026-03-23T10:05:10+08:00", "recv", 6, 11, "Waiting for host connect"}
    };

    devices_.emplace(eqp01.summary.id, eqp01);
    devices_.emplace(eqp02.summary.id, eqp02);
    next_message_id_ = 100;
}

std::string HttpServer::build_devices_json() {
    std::ostringstream oss;
    oss << "{\"items\":[";

    bool first = true;
    for (const auto& [device_id, device] : devices_) {
        (void)device_id;
        if (!first) {
            oss << ",";
        }
        first = false;

        oss << "{"
            << "\"id\":\"" << json_escape(device.summary.id) << "\","
            << "\"name\":\"" << json_escape(device.summary.name) << "\","
            << "\"ip\":\"" << json_escape(device.summary.ip) << "\","
            << "\"port\":" << device.summary.port << ","
            << "\"mode\":\"" << device_mode_to_string(device.summary.mode) << "\","
            << "\"status\":\"" << device_status_to_string(device.summary.status) << "\","
            << "\"selected\":" << (device.summary.selected ? "true" : "false")
            << "}";
    }

    oss << "]}";
    return oss.str();
}

std::string HttpServer::build_messages_json(const std::string& device_id) {
    const auto found = devices_.find(device_id);
    if (found == devices_.end()) {
        return "[]";
    }

    std::ostringstream oss;
    oss << "[";
    for (std::size_t i = 0; i < found->second.messages.size(); ++i) {
        if (i != 0U) {
            oss << ",";
        }
        oss << message_to_json(found->second.messages[i]);
    }
    oss << "]";
    return oss.str();
}

MessageEvent HttpServer::append_message_locked(
    DeviceRecord& device,
    const std::string& direction,
    const int stream,
    const int function,
    const std::string& note) {
    const auto timestamp = make_timestamp();
    MessageEvent message {
        device.summary.id + "-m" + std::to_string(next_message_id_++),
        timestamp,
        direction,
        stream,
        function,
        note
    };

    device.messages.insert(device.messages.begin(), message);
    if (device.messages.size() > 20U) {
        device.messages.resize(20U);
    }
    return message;
}

void HttpServer::broadcast_device_event(
    const std::string& device_id,
    const std::string& type,
    const std::string& timestamp,
    const std::string& payload_json) {
#if defined(SECSGEM_POSIX_SERVER)
    std::vector<int> clients;
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        const auto found = ws_clients_.find(device_id);
        if (found == ws_clients_.end()) {
            return;
        }
        clients = found->second;
    }

    const std::string event_json =
        "{\"type\":\"" + json_escape(type) + "\","
        "\"deviceId\":\"" + json_escape(device_id) + "\","
        "\"timestamp\":\"" + json_escape(timestamp) + "\","
        "\"payload\":" + payload_json + "}";

    std::vector<int> dead_clients;
    for (const int client_fd : clients) {
        if (!send_websocket_text_frame(client_fd, event_json)) {
            dead_clients.push_back(client_fd);
        }
    }

    if (!dead_clients.empty()) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        auto& subscriptions = ws_clients_[device_id];
        for (const int client_fd : dead_clients) {
            subscriptions.erase(
                std::remove(subscriptions.begin(), subscriptions.end(), client_fd),
                subscriptions.end());
        }
    }
#else
    (void)device_id;
    (void)type;
    (void)timestamp;
    (void)payload_json;
#endif
}

std::string HttpServer::build_action_json(const std::string& device_id, const std::string& action) {
    std::string timestamp;
    std::string payload_json;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        auto found = devices_.find(device_id);
        if (found == devices_.end()) {
            return R"({"ok":false,"message":"unknown_device"})";
        }

        auto& device = found->second;
        timestamp = make_timestamp();

        if (action == "connect") {
            device.summary.status = DeviceStatus::Connected;
            const auto message = append_message_locked(device, "send", 1, 13, "Connect command accepted");
            payload_json =
                "{\"status\":\"connected\",\"message\":" + message_to_json(message) + "}";
        } else if (action == "disconnect") {
            device.summary.status = DeviceStatus::Disconnected;
            payload_json = "{\"status\":\"disconnected\",\"note\":\"Disconnect command accepted\"}";
        } else if (action == "linktest") {
            const auto message = append_message_locked(device, "send", 0, 0, "Linktest heartbeat");
            payload_json = message_to_json(message);
        }
    }

    if (action == "linktest") {
        broadcast_device_event(device_id, "secs_message", timestamp, payload_json);
    } else {
        broadcast_device_event(device_id, "session_state", timestamp, payload_json);
    }

    std::ostringstream oss;
    oss << "{"
        << "\"ok\":true,"
        << "\"action\":\"" << action << "\","
        << "\"deviceId\":\"" << device_id << "\","
        << "\"message\":\"" << action << " accepted for " << device_id << "\""
        << "}";
    return oss.str();
}

std::string HttpServer::handle_route(
    const std::string& method,
    const std::string& path,
    int& status_code,
    std::string& status_text) {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (method == "GET" && path == "/api/devices") {
        status_code = 200;
        status_text = "OK";
        return build_devices_json();
    }

    const std::string device_prefix = "/api/devices/";
    if (path.rfind(device_prefix, 0) == 0) {
        const auto rest = path.substr(device_prefix.size());
        const auto slash_pos = rest.find('/');
        const auto device_id = slash_pos == std::string::npos ? rest : rest.substr(0, slash_pos);
        const auto action = slash_pos == std::string::npos ? "" : rest.substr(slash_pos + 1);

        if (devices_.find(device_id) == devices_.end()) {
            status_code = 404;
            status_text = "Not Found";
            return R"({"error":"unknown_device"})";
        }

        if (method == "GET" && action == "messages") {
            status_code = 200;
            status_text = "OK";
            return build_messages_json(device_id);
        }
    }

    if (method == "OPTIONS") {
        status_code = 204;
        status_text = "No Content";
        return "";
    }

    status_code = 404;
    status_text = "Not Found";
    return R"({"error":"route_not_found"})";
}

void HttpServer::handle_websocket_session(const int client_fd, const std::string& device_id) {
#if defined(SECSGEM_POSIX_SERVER)
    std::string initial_payload;
    std::string initial_timestamp;
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        auto found = devices_.find(device_id);
        if (found == devices_.end()) {
            return;
        }

        ws_clients_[device_id].push_back(client_fd);
        initial_timestamp = make_timestamp();
        initial_payload =
            "{\"status\":\"" + device_status_to_string(found->second.summary.status) + "\","
            "\"connectedClients\":" + std::to_string(ws_clients_[device_id].size()) + "}";
    }

    broadcast_device_event(device_id, "session_state", initial_timestamp, initial_payload);

    char buffer[1024];
    while (true) {
        const auto received = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            break;
        }

        const unsigned char opcode = static_cast<unsigned char>(buffer[0]) & 0x0fU;
        if (opcode == 0x08U) {
            break;
        }
    }

    std::lock_guard<std::mutex> lock(state_mutex_);
    auto found = ws_clients_.find(device_id);
    if (found != ws_clients_.end()) {
        found->second.erase(
            std::remove(found->second.begin(), found->second.end(), client_fd),
            found->second.end());
    }
#else
    (void)client_fd;
    (void)device_id;
#endif
}

void HttpServer::handle_client(const int client_fd) {
#if defined(SECSGEM_POSIX_SERVER)
    char buffer[8192] = {0};
    const auto bytes = ::read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes <= 0) {
        ::close(client_fd);
        return;
    }

    const std::string request(buffer, static_cast<std::size_t>(bytes));
    const auto method = extract_method(request);
    const auto path = extract_path(request);
    const auto upgrade = extract_header(request, "Upgrade");

    const std::string ws_prefix = "/ws/devices/";
    if (path.rfind(ws_prefix, 0) == 0 && upgrade == "websocket") {
        const auto device_id = path.substr(ws_prefix.size());
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (devices_.find(device_id) == devices_.end()) {
                const auto response = build_http_response(R"({"error":"unknown_device"})", 404, "Not Found");
                send_all(client_fd, response);
                ::close(client_fd);
                return;
            }
        }

        const auto key = extract_header(request, "Sec-WebSocket-Key");
        if (key.empty()) {
            const auto response = build_http_response(R"({"error":"missing_ws_key"})", 400, "Bad Request");
            send_all(client_fd, response);
            ::close(client_fd);
            return;
        }

        const auto handshake = build_websocket_handshake_response(build_websocket_accept_key(key));
        if (!send_all(client_fd, handshake)) {
            ::close(client_fd);
            return;
        }

        handle_websocket_session(client_fd, device_id);
        ::close(client_fd);
        return;
    }

    if (method == "POST" && path.rfind("/api/devices/", 0) == 0) {
        const std::string device_prefix = "/api/devices/";
        const auto rest = path.substr(device_prefix.size());
        const auto slash_pos = rest.find('/');
        const auto device_id = slash_pos == std::string::npos ? rest : rest.substr(0, slash_pos);
        const auto action = slash_pos == std::string::npos ? "" : rest.substr(slash_pos + 1);

        if (action == "connect" || action == "disconnect" || action == "linktest") {
            int status_code = 200;
            std::string status_text = "OK";
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                if (devices_.find(device_id) == devices_.end()) {
                    status_code = 404;
                    status_text = "Not Found";
                    const auto response = build_http_response(R"({"error":"unknown_device"})", status_code, status_text);
                    send_all(client_fd, response);
                    ::close(client_fd);
                    return;
                }
            }

            const auto response = build_http_response(build_action_json(device_id, action), status_code, status_text);
            send_all(client_fd, response);
            ::close(client_fd);
            return;
        }
    }

    int status_code = 200;
    std::string status_text = "OK";
    const auto body = handle_route(method, path, status_code, status_text);
    const auto response = build_http_response(body, status_code, status_text);
    send_all(client_fd, response);
    ::close(client_fd);
#else
    (void)client_fd;
#endif
}

void HttpServer::run(const int port) {
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

    if (::listen(server_fd, 32) < 0) {
        ::close(server_fd);
        throw std::runtime_error("failed to listen");
    }

    std::cout << "HTTP/WebSocket server listening on 0.0.0.0:" << port << std::endl;

    while (true) {
        sockaddr_in client_address {};
        socklen_t client_size = sizeof(client_address);
        const int client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_address), &client_size);
        if (client_fd < 0) {
            continue;
        }

        std::thread([this, client_fd]() {
            this->handle_client(client_fd);
        }).detach();
    }
#else
    (void)port;
    throw std::runtime_error("minimal HTTP server is implemented for Linux deployment only");
#endif
}

}  // namespace secsgem
