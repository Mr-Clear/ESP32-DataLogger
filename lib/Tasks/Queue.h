#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

template <typename T>
class Queue {
public:
    Queue(int size) :
        _queue(xQueueCreate(size, sizeof(T)))
    { }

    ~Queue() {
        vQueueDelete(_queue);
    }

    bool push(const T &data, int timeoutMs = -1) {
        const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
        return xQueueSend(_queue, static_cast<void*>(const_cast<T*>(new T(data))), timeout) == pdPASS;
    }

    T pop(int timeoutMs = -1) {
        const TickType_t timeout = timeoutMs >= 0 ? timeoutMs / portTICK_PERIOD_MS : portMAX_DELAY;
        T data;
        if (xQueueReceive( _queue, static_cast<void*>(&data), timeout) == pdPASS)
            return data;
        return T{};
    }

private:
    QueueHandle_t _queue;
};
