#pragma once

#include <functional>
#include <vector>

template <typename T>
class Observable {
public:
    using Observer = std::function<void(const T &value)>;

    Observable() = default;
    virtual ~Observable() = default;

    Observable(const Observable<T> &o) = delete;
    T &operator=(const Observable<T> &o) = delete;

    virtual operator T() const = 0;

    void addObserver(Observer observer, bool instantCall = false) const {
        _observers.emplace_back(observer);
        if (instantCall)
            observer(static_cast<T>(*this));
    }

protected:
    void fireChange(const T value) {
        for (Observer &l : _observers) {
            l(value);
        }
    }

private:
    mutable std::vector<Observer> _observers;
};

template <typename T>
class ObservableValue : public Observable<T> {
public:
    explicit ObservableValue(const T &value = T{}, bool alwaysUpdate = true) :
        _value{value},
        _alwaysUpdate{alwaysUpdate}
    { }

    ObservableValue(const ObservableValue<T> &o) = delete;
    T &operator=(const ObservableValue<T> &o) = delete;

    T &operator=(const T &o) {
        if (_alwaysUpdate || _value != o) {
            _value = o;
            this->Observable<T>::fireChange(o);  
        }
        return _value;
    }

    operator T() const override {
        return _value;
    }

private:
    T _value;
    bool _alwaysUpdate;
};

template <typename F /* from */, typename T /* to */>
class ObservableFilter : public Observable<T> {
public:
    using Filter = std::function<T(const F &from)>;
    explicit ObservableFilter(const Observable<F> &parent, const Filter &filter) :
        _parent{parent},
        _filter{filter}
    {
        parent.addObserver( [this] ( const F &from ) {
            this->Observable<T>::fireChange(_filter(from));
        });
    }

    ObservableFilter(const ObservableFilter<F, T> &o) = delete;
    T &operator=(const ObservableFilter<F, T> &o) = delete;

    operator T() const override {
        return T{};
    }

private:
    const Observable<F> &_parent;
    Filter _filter;
};
