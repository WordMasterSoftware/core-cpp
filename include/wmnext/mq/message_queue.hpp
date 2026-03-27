#pragma once

#include "wmnext/http/json_response.hpp"

namespace wmnext::mq {

struct QueueMessage {
    std::uint64_t id;
    std::string event;
    http::Json payload;
};

class MessageQueue {
public:
    void publish(std::string event, http::Json payload);

    [[nodiscard]] bool wait_and_pop(QueueMessage& message);

    void shutdown();

private:
    std::mutex mutex_;
    std::condition_variable condition_variable_;
    std::queue<QueueMessage> messages_;
    std::uint64_t next_message_id_ = 1;
    bool is_shutdown_ = false;
};

}  // namespace wmnext::mq
