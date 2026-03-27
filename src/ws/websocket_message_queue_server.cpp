#include "wmnext/ws/websocket_message_queue_server.hpp"

namespace wmnext::ws {

namespace {

constexpr auto kMessageType = websocketpp::frame::opcode::text;
constexpr std::string_view kQueueEndpoint = "/queue";

}  // namespace

WebSocketMessageQueueServer::~WebSocketMessageQueueServer() {
    stop();
}

bool WebSocketMessageQueueServer::start(const std::string& host, std::uint16_t port, mq::MessageQueue& message_queue) {
    if (is_running_) {
        return true;
    }

    server_.clear_access_channels(websocketpp::log::alevel::all);
    server_.clear_error_channels(websocketpp::log::elevel::all);
    server_.init_asio();
    server_.set_reuse_addr(true);

    server_.set_open_handler([this](ConnectionHandle connection) {
        std::lock_guard lock(connections_mutex_);
        connections_.insert(connection);
    });

    server_.set_validate_handler([this](ConnectionHandle connection) {
        const auto server_connection = server_.get_con_from_hdl(connection);
        return server_connection->get_resource() == kQueueEndpoint;
    });

    server_.set_close_handler([this](ConnectionHandle connection) {
        std::lock_guard lock(connections_mutex_);
        connections_.erase(connection);
    });

    server_.set_fail_handler([this](ConnectionHandle connection) {
        std::lock_guard lock(connections_mutex_);
        connections_.erase(connection);
    });

    try {
        server_.listen(asio::ip::tcp::endpoint(asio::ip::make_address(host), port));
        server_.start_accept();
    } catch (const std::exception&) {
        return false;
    }

    is_running_ = true;

    server_thread_ = std::thread([this] {
        server_.run();
    });

    dispatcher_thread_ = std::thread([this, &message_queue] {
        dispatch_messages(message_queue);
    });

    return true;
}

void WebSocketMessageQueueServer::stop() {
    if (!is_running_) {
        return;
    }

    is_running_ = false;

    server_.get_io_service().post([this] {
        std::vector<ConnectionHandle> connections;

        {
            std::lock_guard lock(connections_mutex_);
            connections.reserve(connections_.size());
            for (const auto& connection : connections_) {
                connections.push_back(connection);
            }
            connections_.clear();
        }

        for (const auto& connection : connections) {
            websocketpp::lib::error_code error_code;
            server_.close(connection, websocketpp::close::status::going_away, "Server stopping", error_code);
        }
    });

    websocketpp::lib::error_code error_code;
    server_.stop_listening(error_code);
    server_.stop();

    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    if (dispatcher_thread_.joinable()) {
        dispatcher_thread_.join();
    }
}

http::Json WebSocketMessageQueueServer::make_envelope(const mq::QueueMessage& message) const {
    return http::Json{
        {"message_id", message.id},
        {"event", message.event},
        {"payload", message.payload}
    };
}

void WebSocketMessageQueueServer::dispatch_messages(mq::MessageQueue& message_queue) {
    mq::QueueMessage message{};

    while (message_queue.wait_and_pop(message)) {
        send_to_all(make_envelope(message).dump());
    }
}

void WebSocketMessageQueueServer::send_to_all(const std::string& payload) {
    server_.get_io_service().post([this, payload] {
        std::vector<ConnectionHandle> connections;

        {
            std::lock_guard lock(connections_mutex_);
            connections.reserve(connections_.size());
            for (const auto& connection : connections_) {
                connections.push_back(connection);
            }
        }

        for (const auto& connection : connections) {
            websocketpp::lib::error_code error_code;
            server_.send(connection, payload, kMessageType, error_code);

            if (error_code) {
                std::lock_guard lock(connections_mutex_);
                connections_.erase(connection);
            }
        }
    });
}

}  // namespace wmnext::ws
