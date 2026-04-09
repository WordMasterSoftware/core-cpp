#include "wmnext/ws/websocket_notifier.hpp"

namespace wmnext::ws {

namespace {

constexpr auto kMessageType = websocketpp::frame::opcode::text;
constexpr std::string_view kEndpoint = "/queue";

}  // namespace

WebSocketNotifier::~WebSocketNotifier() {
    stop();
}

bool WebSocketNotifier::start(const std::string& host, std::uint16_t port) {
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
        return server_connection->get_resource() == kEndpoint;
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
    return true;
}

void WebSocketNotifier::stop() {
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
}

void WebSocketNotifier::notify(std::string event, http::Json payload) {
    notify(make_message(std::move(event), std::move(payload)));
}

void WebSocketNotifier::notify(http::Json message) {
    if (!is_running_) {
        return;
    }

    send_to_all(message.dump());
}

http::Json WebSocketNotifier::make_message(std::string event, http::Json payload) const {
    return http::Json{
        {"event", std::move(event)},
        {"payload", std::move(payload)}
    };
}

void WebSocketNotifier::send_to_all(std::string payload) {
    server_.get_io_service().post([this, payload = std::move(payload)] {
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
