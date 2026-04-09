#pragma once

#include "wmnext/http/json_response.hpp"

namespace wmnext::ws {

class WebSocketNotifier {
public:
    WebSocketNotifier() = default;
    ~WebSocketNotifier();

    WebSocketNotifier(const WebSocketNotifier&) = delete;
    WebSocketNotifier& operator=(const WebSocketNotifier&) = delete;

    [[nodiscard]] bool start(const std::string& host, std::uint16_t port);

    void stop();

    void notify(std::string event, http::Json payload);
    void notify(http::Json message);

private:
    using WebSocketServer = websocketpp::server<websocketpp::config::asio>;
    using ConnectionHandle = websocketpp::connection_hdl;
    using ConnectionHandleSet = std::set<ConnectionHandle, std::owner_less<ConnectionHandle>>;

    [[nodiscard]] http::Json make_message(std::string event, http::Json payload) const;
    void send_to_all(std::string payload);

    WebSocketServer server_;
    std::mutex connections_mutex_;
    ConnectionHandleSet connections_;
    std::thread server_thread_;
    bool is_running_ = false;
};

}  // namespace wmnext::ws
