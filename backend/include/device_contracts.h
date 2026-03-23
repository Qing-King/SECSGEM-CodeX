#pragma once

#include <string>
#include <vector>

namespace secsgem {

enum class DeviceMode {
    Active,
    Passive
};

enum class DeviceStatus {
    Connected,
    Connecting,
    Disconnected
};

struct DeviceSummary {
    std::string id;
    std::string name;
    std::string ip;
    int port {0};
    DeviceMode mode {DeviceMode::Active};
    DeviceStatus status {DeviceStatus::Disconnected};
    bool selected {false};
};

struct MessageEvent {
    std::string id;
    std::string timestamp;
    std::string direction;
    int stream {0};
    int function {0};
    std::string note;
};

struct DeviceListResponse {
    std::vector<DeviceSummary> items;
};

struct WsDeviceEvent {
    std::string type;
    std::string deviceId;
    std::string timestamp;
    std::string payloadJson;
};

}  // namespace secsgem
