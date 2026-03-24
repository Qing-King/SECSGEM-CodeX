#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "device_contracts.h"

namespace secsgem {

class HttpServer {
public:
    HttpServer();
    void run(int port);

private:
    struct DeviceRecord {
        DeviceSummary summary;
        std::vector<MessageEvent> messages;
    };

    std::mutex state_mutex_;
    std::unordered_map<std::string, DeviceRecord> devices_;
    std::unordered_map<std::string, std::vector<int>> ws_clients_;
    std::uint64_t next_message_id_ {1};

    void handle_client(int client_fd);
    void handle_websocket_session(int client_fd, const std::string& device_id);
    std::string handle_route(
        const std::string& method,
        const std::string& path,
        int& status_code,
        std::string& status_text);
    std::string build_devices_json();
    std::string build_messages_json(const std::string& device_id);
    std::string build_action_json(const std::string& device_id, const std::string& action);
    MessageEvent append_message_locked(
        DeviceRecord& device,
        const std::string& direction,
        int stream,
        int function,
        const std::string& note);
    void broadcast_device_event(
        const std::string& device_id,
        const std::string& type,
        const std::string& timestamp,
        const std::string& payload_json);
};

}  // namespace secsgem
