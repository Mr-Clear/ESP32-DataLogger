#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory>

template <typename T>
class Queue {
public:
    Queue(int size) :
        _queue(xQueueCreate(size, sizeof(T)))
    { }

    ~Queue() {
        vQueueDelete(_queue);
    }

    bool push(const T &data, int timeoutMs = -1) const {
        T *copy = new T(data);
        bool ret;
        if (xPortInIsrContext()) {
            ret = xQueueSendFromISR(_queue, static_cast<void*>(copy), 0) == pdPASS;
        } else {
            const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
            ret = xQueueSend(_queue, static_cast<void*>(copy), timeout) == pdPASS;
        }
        _allocator.deallocate(copy, 1);
        return ret;
    }

    T pop(int timeoutMs = -1) {
        const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
        std::unique_ptr<T> data{_allocator.allocate(1)};
        if (xQueueReceive(_queue, static_cast<void*>(data.get()), timeout) == pdPASS)
            return *data.get();
        return T{};
    }

    unsigned int messagesWaiting() const {
        return uxQueueMessagesWaiting(_queue);
    }

private:
    QueueHandle_t _queue;

    static std::allocator<T> _allocator;
};
