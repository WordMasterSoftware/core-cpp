#pragma once

#include "wmnext/mq/message_queue.hpp"

namespace wmnext::ws {

class WebSocketMessageQueueServer {
public:
    WebSocketMessageQueueServer() = default;
    ~WebSocketMessageQueueServer();

    WebSocketMessageQueueServer(const WebSocketMessageQueueServer&) = delete;
    WebSocketMessageQueueServer& operator=(const WebSocketMessageQueueServer&) = delete;

    [[nodiscard]] bool start(const std::string& host, std::uint16_t port, mq::MessageQueue& message_queue);

    void stop();

private:
    using WebSocketServer = websocketpp::server<websocketpp::config::asio>;
    using ConnectionHandle = websocketpp::connection_hdl;
    using ConnectionHandleSet = std::set<ConnectionHandle, std::owner_less<ConnectionHandle>>;

    [[nodiscard]] http::Json make_envelope(const mq::QueueMessage& message) const;
    void dispatch_messages(mq::MessageQueue& message_queue);
    void send_to_all(const std::string& payload);

    WebSocketServer server_;
    std::mutex connections_mutex_;
    ConnectionHandleSet connections_;
    std::thread server_thread_;
    std::thread dispatcher_thread_;
    bool is_running_ = false;
};

}  // namespace wmnext::ws
