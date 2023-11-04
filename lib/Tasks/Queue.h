#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <memory>

namespace {
    TickType_t ticksToMs(int ms) {
        return ms >= 0 ? ms / portTICK_PERIOD_MS : portMAX_DELAY;;
    }
}

template <typename T>
class Queue {
public:
    Queue(int size) :
        _queue{xQueueCreate(size, sizeof(T))} {
        assert(_queue);
    }

    ~Queue() {
        vQueueDelete(_queue);
    }

    bool push(const T &data, int timeout = -1) const {
        T *copy = new T{data};
        BaseType_t ret;
        if (xPortInIsrContext()) {
            assert(timeout <= 0);
            ret = xQueueSendFromISR(_queue, static_cast<void*>(copy), 0);
        } else {
            ret = xQueueSend(_queue, static_cast<void*>(copy), ticksToMs(timeout));
        }
        _allocator.deallocate(copy, 1);
        return ret == pdTRUE;
    }

    bool pushToFront(const T &data, int timeout = -1) const {
        T *copy = new T{data};
        BaseType_t ret;
        if (xPortInIsrContext()) {
            assert(timeout <= 0);
            ret = xQueueSendToFrontFromISR(_queue, static_cast<void*>(copy), 0);
        } else {
            ret = xQueueSendToFront(_queue, static_cast<void*>(copy), ticksToMs(timeout));
        }
        _allocator.deallocate(copy, 1);
        return ret == pdTRUE;
    }

    T pop(int timeout = -1) {
        std::unique_ptr<T> data{_allocator.allocate(1)};
        BaseType_t ret;
        if (xPortInIsrContext()) {
            assert(timeout <= 0);
            ret = xQueueReceiveFromISR(_queue, static_cast<void*>(data.get()), 0);
        } else {
            ret = xQueueReceive(_queue, static_cast<void*>(data.get()), ticksToMs(timeout));
        }
        if (ret == pdTRUE)
            return std::move(*data.get());
        return T{};
    }

    [[nodiscard]] T peek(int timeout = -1) const {
        std::unique_ptr<T, Deallocator> data{_allocator.allocate(1)};
        BaseType_t ret;
        if (xPortInIsrContext()) {
            assert(timeout <= 0);
            ret = xQueuePeekFromISR(_queue, static_cast<void*>(data.get()));
        } else {
            ret = xQueuePeek(_queue, static_cast<void*>(data.get()), ticksToMs(timeout));
        }
        if (ret == pdTRUE)
            return T{*data.get()};
        return T{};
    }

    [[nodiscard]] unsigned int size() const {
        if (xPortInIsrContext())
            return uxQueueMessagesWaitingFromISR(_queue);
        return uxQueueMessagesWaiting(_queue);
    }

    [[nodiscard]] bool empty() const {
        return size() == 0;
    }

    [[nodiscard]] int freeSpace() const {
        return uxQueueSpacesAvailable(_queue);
    }

    void clear() {
        xQueueReset(_queue);
    }

private:
    struct Deallocator {
        void operator()(T* t) {
            _allocator.deallocate(t, 1); 
        }
    };

    QueueHandle_t _queue;

    static std::allocator<T> _allocator;
};

class VoidQueue {
public:
    VoidQueue(int size) :
        _queue(xQueueCreate(size, 0))
    { }

    ~VoidQueue() {
        vQueueDelete(_queue);
    }

    bool push(int timeoutMs = -1) const {
        bool ret;
        if (xPortInIsrContext()) {
            ret = xQueueSendFromISR(_queue, nullptr, 0) == pdPASS;
        } else {
            const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
            ret = xQueueSend(_queue, nullptr, timeout) == pdPASS;
        }
        return ret;
    }

    void pop(int timeoutMs = -1) {
        const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
        xQueueReceive(_queue, nullptr, timeout);
    }

    unsigned int messagesWaiting() const {
        return uxQueueMessagesWaiting(_queue);
    }

private:
    QueueHandle_t _queue;
};
