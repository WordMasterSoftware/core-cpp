#include "wmnext/mq/message_queue.hpp"

namespace wmnext::mq {

void MessageQueue::publish(std::string event, http::Json payload) {
    {
        std::lock_guard lock(mutex_);

        if (is_shutdown_) {
            return;
        }

        messages_.push(QueueMessage{
            .id = next_message_id_++,
            .event = std::move(event),
            .payload = std::move(payload)
        });
    }

    condition_variable_.notify_one();
}

bool MessageQueue::wait_and_pop(QueueMessage& message) {
    std::unique_lock lock(mutex_);

    condition_variable_.wait(lock, [this] {
        return is_shutdown_ || !messages_.empty();
    });

    if (messages_.empty()) {
        return false;
    }

    message = std::move(messages_.front());
    messages_.pop();
    return true;
}

void MessageQueue::shutdown() {
    {
        std::lock_guard lock(mutex_);
        is_shutdown_ = true;
    }

    condition_variable_.notify_all();
}

}  // namespace wmnext::mq
