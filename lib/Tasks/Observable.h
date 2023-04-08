#pragma once

#include <functional>
#include <vector>

template <typename T>
class Observable {
public:
    using Observer = std::function<void(const T &value)>;

    Observable() = default;
    explicit Observable(const T &value) :
        _value(value) { }

    Observable(const Observable<T> &o) = delete;
    T &operator=(const Observable<T> &o) = delete;

    T &operator=(const T &o) {
        if (_value != o) {
            _value = o;
            fireChange(o);  
        }
        return _value;
    }

    operator T() const {
        return _value;
    }

    void addObserver(Observer observer) const {
        _observers.emplace_back(observer);
    }

private:
    T _value;
    mutable std::vector<Observer> _observers;

    void fireChange(const T value) {
        for (Observer &l : _observers) {
            l(value);
        }
    }
};
